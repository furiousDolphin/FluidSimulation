
#define _USE_MATH_DEFINES

#include <stdexcept>
#include <cmath>
#include <regex>
#include <string>
#include <boost/math/interpolators/makima.hpp>


#include "MatrixBuilders.hpp"

namespace FluidSimulation
{
    App::App(std::string L_operator_str, std::string left_BC_str, std::string right_BC_str, int N) :
        N_{N}
    {
        L_operator_coeffs_ = this->ParseLOperator(L_operator_str);
        left_BC_coeffs_ = this->ParseBoundaryCondition(left_BC_str);
        right_BC_coeffs_ = this->ParseBoundaryCondition(right_BC_str);

        x1_ = left_BC_coeffs_.boundary_x;
        x2_ = right_BC_coeffs_.boundary_x;

        dtdx_ = -2.0/(x1_ - x2_);

        D_hat_ = this->GetPhysicalDifferentiationOperator();
        S_ = this->GetTransitionOperator();
        L_hat_ = this->GetLOperator();
        R_ = this->GetInverseCholeskyDecompMatrix();
        A_tilde_ = R_ * ( L_hat_ * S_ );
        c0_ = this->GetBCTargetsChebCoeffs();
        b_tilde_0_ = R_*(L_hat_ * c0_);
    }

    App::BCCoeffs App::ParseBoundaryCondition(std::string expression)
    {
        BCCoeffs coeffs;
        std::smatch match;

        //-----------------------------------------------------------------------------
        std::regex deriv1_regex(R"(([-+]?\d*\.?\d*)\s*u'\(([-+]?\d*\.?\d*)\))");
        //--------------------------------------------------------
        std::regex func_regex(R"(([-+]?\d*\.?\d*)\s*u\(([-+]?\d*\.?\d*)\))");
        //-----------------------------------------------------------------------------
        std::regex target_regex(R"(=\s*([-+]?\d*\.?\d*))");
        //-----------------------------------------------------------------------------


        if (std::regex_search(expression, match, deriv1_regex)) 
        {
            std::string val = match[1].str();
            coeffs.deriv1_coeff = (val == "" || val == "+") ? 1.0 : (val == "-") ? -1.0 : std::stod(val);
            coeffs.boundary_x = std::stod(match[2].str());
        }

        if (std::regex_search(expression, match, func_regex)) 
        {
            std::string val = match[1].str();
            coeffs.func_coeff = (val == "" || val == "+") ? 1.0 : (val == "-") ? -1.0 : std::stod(val);
            coeffs.boundary_x = std::stod(match[2].str());
        }
        
        if (std::regex_search(expression, match, target_regex)) 
        { coeffs.target = std::stod(match[1].str()); }

        return coeffs;        
    }

    App::LOperatorCoeffs App::ParseLOperator(std::string expression) {
        LOperatorCoeffs coeffs;
        std::smatch match;

        //-----------------------------------------------------------------------------
        std::regex deriv2_regex(R"(([-+]?\d*\.?\d*)\s*u'')");
        //-----------------------------------------------------------------------------
        std::regex deriv1_regex(R"(([-+]?\d*\.?\d*)\s*u'(?!'))");
        //-----------------------------------------------------------------------------
        std::regex func_regex(R"(([-+]?\d*\.?\d*)\s*u(?![u']))");
        //-----------------------------------------------------------------------------


        if (std::regex_search(expression, match, deriv2_regex)) 
        {
            std::string val = match[1].str();
            coeffs.deriv2_coeff = (val == "" || val == "+") ? 1.0 : (val == "-") ? -1.0 : std::stod(val);
        }

        if (std::regex_search(expression, match, deriv1_regex)) 
        {
            std::string val = match[1].str();
            coeffs.deriv1_coeff = (val == "" || val == "+") ? 1.0 : (val == "-") ? -1.0 : std::stod(val);
        }
        
        if (std::regex_search(expression, match, func_regex)) 
        { coeffs.func_coeff = std::stod(match[1].str()); }

        return coeffs;
    }

    Eigen::MatrixXd App::GetPhysicalDifferentiationOperator()
    {
        return dtdx_*DifferentiationOperator(N_);        
    }

    Eigen::MatrixXd App::GetTransitionOperator()
    {
        Eigen::MatrixXd S = Eigen::MatrixXd::Zero((N_+1), (N_+1)-2);

        //-------------------------------------------------------------------

        auto [left_deriv1_coeff, left_func_coeff, left_target, _1] = left_BC_coeffs_;

        auto L1 = [this, left_deriv1_coeff, left_func_coeff](int n)
        { 
            return (std::pow(-1.0, n+1)*(left_deriv1_coeff*dtdx_*(n*n)) + 
                    std::pow(-1.0, n)*(left_func_coeff)  ); 
        };  
        
        //-------------------------------------------------------------------

        auto [right_deriv1_coeff, right_func_coeff, right_target, _2] = right_BC_coeffs_;

        auto L2 = [this, right_deriv1_coeff, right_func_coeff](int n)
        { 
            return right_deriv1_coeff*dtdx_*(n*n) + right_func_coeff;
        };       

        //-------------------------------------------------------------------

        for ( int k = 0; k < (N_+1)-2; k++ )
        {
            Eigen::Matrix2d A;
            Eigen::Vector2d B;

            A(0, 0) = L1(k+1);
            A(0, 1) = L1(k+2);
            B(0) = -L1(k);

            A(1, 0) = L2(k+1);
            A(1, 1) = L2(k+2);
            B(1) = -L2(k);

            Eigen::Vector2d sol = A.colPivHouseholderQr().solve(B);

            S(k, k)     = 1.0;
            S(k + 1, k) = sol(0); //a_k
            S(k + 2, k) = sol(1); //b_k
        }
        return S;
    }

    Eigen::MatrixXd App::GetBCTargetsChebCoeffs()
    {
        //v ~ aT1(t) + bT0(t)    v' ~ aT0(t)dtdx   ->     
        //deriv_coeff*v'+func_coeff*v=target  ->
        //deriv_coeff*[aT0(t)dtdx]+func_coeff*[aT1(t) + bT0(t)]=target
        //[deriv_coeff*(a*dtdx) + func_coeff*b]T0(t) + func_coeff*a*T1(t) = target
        //[ deriv_coeff*dtdx*T0(t) + funct_coeff*T1(t) ]*a + func_coeff*T0(t)*b = target

        Eigen::VectorXd coeffs = Eigen::VectorXd::Zero(N_+1);
        auto T0 = [](double t){ return 1.0; };
        auto T1 = [](double t){ return 1.0*t; };
        auto [left_deriv1_coeff, left_func_coeff, left_target, _1] = left_BC_coeffs_;
        auto [right_deriv1_coeff, right_func_coeff, right_target, _2] = right_BC_coeffs_;


        Eigen::Matrix2d A;
        Eigen::Vector2d B;

        A(0, 0) = left_deriv1_coeff*dtdx_*T0(-1.0) + left_func_coeff*T1(-1.0);
        A(0, 1) = left_func_coeff*T0(-1.0);
        B(0) = left_BC_coeffs_.target;

        A(1, 0) = right_deriv1_coeff*dtdx_*T0(1.0) + right_func_coeff*T1(1.0);
        A(1, 1) = right_func_coeff*T0(1.0);
        B(1) = right_BC_coeffs_.target;

        Eigen::Vector2d sol = A.colPivHouseholderQr().solve(B);

        coeffs(0) = sol(1);    //zamienilem bo wspolczynnik a jest do T1 , b do T0
        coeffs(1) = sol(0); 

        return coeffs;
    }

    Eigen::MatrixXd App::GetLOperator()
    {
        auto [ deriv2_coeff, deriv1_coeff, func_coeff] = L_operator_coeffs_;

        Eigen::MatrixXd I = Eigen::MatrixXd::Identity(N_+1, N_+1);
        Eigen::MatrixXd L_hat = deriv2_coeff*(D_hat_*D_hat_) + deriv1_coeff*D_hat_ + func_coeff*I;

        return L_hat;
    }

    Eigen::MatrixXd App::GetInverseCholeskyDecompMatrix()
    {
        Eigen::MatrixXd M = ChebMassMatrix(N_);
        Eigen::LLT<Eigen::MatrixXd> llt{M};
        Eigen::MatrixXd R = llt.matrixL().transpose();    
        return R;    
    }

    std::pair<Eigen::VectorXd, Eigen::VectorXd> App::solve(const Eigen::VectorXd& f_dense)
    {
        Eigen::VectorXd X = Eigen::VectorXd::LinSpaced(500, x1_, x2_);
        Eigen::VectorXd cheb_nodes = FluidSimulation::ChebNodes(N_, x1_, x2_);
        Eigen::VectorXd f_cheb = InterpolateToChebNodes(X, f_dense, cheb_nodes);
        Eigen::VectorXd f_hat = FluidSimulation::ChebCoeffs(f_cheb);
        


        Eigen::VectorXd b_tilde = R_ * f_hat - b_tilde_0_;

        Eigen::BDCSVD<Eigen::MatrixXd> svd{A_tilde_, Eigen::ComputeFullU | Eigen::ComputeFullV};
        Eigen::VectorXd d = svd.solve(b_tilde);
        Eigen::VectorXd c = S_*d + c0_;

        Eigen::VectorXd t = ( -2.0/(x1_ - x2_) )*X.array() + ( (x1_ + x2_)/(x1_ - x2_) );
        Eigen::VectorXd Y = FluidSimulation::EvaluateCheb(c, t);

        return {X, Y};         
    }



    Eigen::VectorXd ChebNodes(int N, double x1, double x2)
    {
        Eigen::VectorXd nodes = ( Eigen::VectorXd::LinSpaced(N+1, 0, N) * (M_PI/N) ).array().cos();
        return ( (x2 - x1)/2 )*nodes.array() + ( (x2 + x1)/2 );
    }

    Eigen::VectorXd EvaluateCheb( const Eigen::VectorXd& c, const Eigen::VectorXd& t )
    {
        int N = c.size() - 1;
        int M = t.size();

        Eigen::VectorXd bk1 = Eigen::VectorXd::Zero(M);
        Eigen::VectorXd bk2 = Eigen::VectorXd::Zero(M);
        
        for (int k = N; k >= 1; k--) 
        {
            Eigen::VectorXd temp = bk1; 
            bk1 = (2.0 * t.array() * bk1.array() - bk2.array() + c(k)).matrix();
            bk2 = temp;
        }

        return (t.array() * bk1.array() - bk2.array() + c(0)).matrix();
    }

    Eigen::VectorXd ChebCoeffs(const Eigen::VectorXd& f_vals) 
    {
        int N = f_vals.size() - 1;
        
        Eigen::VectorXd g_vals(2 * N);

        for (int j = 0; j <= N; j++) 
        { g_vals[j] = f_vals[j]; }

        for (int j = 1; j < N; j++) 
        { g_vals[2 * N - j] = f_vals[j]; }

        Eigen::FFT<double> fft;
        Eigen::VectorXcd freq;
        fft.fwd(freq, g_vals); 

        Eigen::VectorXd coeffs = freq.head(N + 1).real() / static_cast<double>(N);
        
        coeffs(0) /= 2.0;
        coeffs(N) /= 2.0;

        return coeffs;
    }

    Eigen::MatrixXd DifferentiationOperator(int N)
    {
        Eigen::MatrixXd D = Eigen::MatrixXd::Zero(N+1, N+1);

        for ( int i = 0; i < N+1; i++ )
        {
            double factor = (i == 0) ? 1.0 : 2.0;

            for ( int j = i+1; j < N+1; j+=2 )
            { D(i, j) = factor*j; }
        }

        return D;
    }

    double Clenshaw(const Eigen::VectorXd& c, double t) 
    {
        int N = c.size() - 1;
        double bk1 = 0.0, bk2 = 0.0;
        
        for (int k = N; k >= 1; --k) {
            double bk = 2.0 * t * bk1 - bk2 + c(k);
            bk2 = bk1;
            bk1 = bk;
        }
        return t * bk1 - bk2 + c(0);
    }   

    Eigen::MatrixXd ChebMassMatrix(int N)
    {
        Eigen::VectorXd weights( N+1 );
        
        weights.fill(M_PI / 2.0);
        weights[0] = M_PI;

        Eigen::DiagonalMatrix<double, Eigen::Dynamic> M = weights.asDiagonal();

        return M;
    }

    Eigen::VectorXd InterpolateToChebNodes(
        const Eigen::VectorXd& x_dense, 
        const Eigen::VectorXd& y_dense, 
        const Eigen::VectorXd& cheb_nodes)
    {

        std::vector<double> xv(x_dense.data(), x_dense.data() + x_dense.size());
        std::vector<double> yv(y_dense.data(), y_dense.data() + y_dense.size());

        auto spline = boost::math::interpolators::makima<std::vector<double>>(std::move(xv), std::move(yv));

        Eigen::VectorXd y_at_cheb(cheb_nodes.size());
        for (int i = 0; i < cheb_nodes.size(); i++) { 
            double t_x = std::clamp(cheb_nodes(i), x_dense.minCoeff(), x_dense.maxCoeff());
            y_at_cheb(i) = spline(t_x);
        }
        return y_at_cheb;
    }
}

