

#define _USE_MATH_DEFINES

#include <stdexcept>
#include <cmath>
#include <regex>
#include <string>
#include <boost/math/interpolators/makima.hpp>



#include "WaveEquation.hpp"
#include "MatrixBuilders.hpp"

namespace FluidSimulation
{
    WaveEquation::WaveEquation(std::string left_BC_str, std::string right_BC_str, double c_sq, double gamma, int N) :
        c_sq_{c_sq},
        gamma_{gamma},
        N_{N}
    {
        left_BC_coeffs_ = this->ParseBoundaryCondition(left_BC_str);
        right_BC_coeffs_ = this->ParseBoundaryCondition(right_BC_str);

        x1_ = left_BC_coeffs_.boundary_x;
        x2_ = right_BC_coeffs_.boundary_x;

        dtdx_ = -2.0/(x1_ - x2_);

        D_hat_ = this->GetPhysicalDifferentiationOperator();
        D_hat_sq_ = D_hat_*D_hat_;
        S_ = this->GetTransitionOperator();
        R_ = this->GetInverseCholeskyDecompMatrix();
        //A_tilde_ = R_ * ( L_hat_ * S_ );
        c0_ = this->GetBCTargetsChebCoeffs();
        //b_tilde_0_ = R_*(L_hat_ * c0_);
        RS_QR_ = (R_*S_).colPivHouseholderQr();
    }

    void WaveEquation::operator()(const Eigen::VectorXd& X, Eigen::VectorXd& dXdt, double t)
    {
        Eigen::VectorXd d = X.head( (N_+1)-2 );
        Eigen::VectorXd d_prim_t = X.tail( (N_+1)-2 );

        Eigen::VectorXd c_bis_x = D_hat_sq_*(S_*d + c0_);
        Eigen::VectorXd d_bis_x = RS_QR_.solve(R_*c_bis_x);
        Eigen::VectorXd d_bis_t = c_sq_ * d_bis_x - gamma_*d_prim_t;

        dXdt.head( (N_+1)-2 ) = d_prim_t;
        dXdt.tail( (N_+1)-2 ) = d_bis_t;
    }

    WaveEquation::BCCoeffs WaveEquation::ParseBoundaryCondition(std::string expression)
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

    Eigen::MatrixXd WaveEquation::GetPhysicalDifferentiationOperator()
    {
        return dtdx_*DifferentiationOperator(N_);        
    }

    Eigen::MatrixXd WaveEquation::GetTransitionOperator()
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

    Eigen::MatrixXd WaveEquation::GetBCTargetsChebCoeffs()
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

    Eigen::MatrixXd WaveEquation::GetInverseCholeskyDecompMatrix()
    {
        Eigen::MatrixXd M = ChebMassMatrix(N_);
        Eigen::LLT<Eigen::MatrixXd> llt{M};
        Eigen::MatrixXd R = llt.matrixL().transpose();    
        return R;    
    }


    WaveSimulation::WaveSimulation(const WaveEquation& wave_equation, const Eigen::VectorXd& x_dense) :
        eq_{wave_equation},
        x_dense_{x_dense},
        t_{0.0},
        dt_{0.0001},
        state_{2*( (eq_.N_+1)-2 )}
    {
        cheb_t_dense_ = ( -2.0/(eq_.x1_ - eq_.x2_) )*x_dense_.array() + ( (eq_.x1_ + eq_.x2_)/(eq_.x1_ - eq_.x2_) );
        cheb_nodes_ = ChebNodes(eq_.N_, eq_.x1_, eq_.x2_);
    }

    void WaveSimulation::set_u_v(const Eigen::VectorXd& u_dense, const Eigen::VectorXd& v_dense)
    {
        Eigen::VectorXd u_cheb = InterpolateToChebNodes(x_dense_, u_dense, cheb_nodes_);
        Eigen::VectorXd v_cheb = InterpolateToChebNodes(x_dense_, v_dense, cheb_nodes_);

        Eigen::VectorXd c = ChebCoeffs(u_cheb);
        Eigen::VectorXd c_prim = ChebCoeffs(v_cheb);

        Eigen::VectorXd d = eq_.RS_QR_.solve( eq_.R_*(c - eq_.c0_) );
        Eigen::VectorXd d_prim = eq_.RS_QR_.solve(eq_.R_*c_prim);

        state_.head( (eq_.N_+1)-2 ) = d;
        state_.tail( (eq_.N_+1)-2 ) = d_prim;
    }

    void WaveSimulation::step()
    {
        stepper_.do_step(eq_, state_, t_, dt_);
        t_+=dt_;
    }

    std::pair<Eigen::VectorXd, Eigen::VectorXd> WaveSimulation::get_u_v()
    {
        const Eigen::VectorXd& d = state_.head( (eq_.N_+1)-2 );
        const Eigen::VectorXd& d_prim = state_.tail( (eq_.N_+1)-2 );

        Eigen::VectorXd c = eq_.S_*d+eq_.c0_;
        Eigen::VectorXd c_prim = eq_.S_*d_prim;

        Eigen::VectorXd u_dense = EvaluateCheb(c, cheb_t_dense_);
        Eigen::VectorXd v_dense = EvaluateCheb(c_prim, cheb_t_dense_);

        return {u_dense, v_dense};
    }
}