
#ifndef MATRIX_BUILDERS_HPP_
#define MATRIX_BUILDERS_HPP_

#include <boost/numeric/odeint.hpp>
#include <boost/numeric/odeint/external/eigen/eigen.hpp>
#include <unsupported/Eigen/FFT>
#include <Eigen/Dense>

#include <vector>
#include <cmath>

using Eigen::MatrixXd;

namespace FluidSimulation
{
    Eigen::MatrixXd DifferentiationOperator(int N);
    Eigen::VectorXd ChebNodes(int N, double x1, double x2);
    Eigen::VectorXd EvaluateCheb( const Eigen::VectorXd& c, const Eigen::VectorXd& Xi );
    Eigen::VectorXd ChebCoeffs(const Eigen::VectorXd& f_values);
    Eigen::MatrixXd ChebMassMatrix(int N);
    Eigen::VectorXd InterpolateToChebNodes( const Eigen::VectorXd& x_dense, const Eigen::VectorXd& y_dense, const Eigen::VectorXd& cheb_nodes);
    
    class App
    {
        public:
            App(std::string L_operator_str, std::string left_BC_str, std::string right_BC_str, int N);
            std::pair<Eigen::VectorXd, Eigen::VectorXd> solve(const Eigen::VectorXd& f_dense);


        private:
            struct App::BCCoeffs 
            {
                double deriv1_coeff = 0.0; 
                double func_coeff = 0.0;  
                double target = 0.0;     
                double boundary_x = 0.0;
            };

            struct LOperatorCoeffs 
            {
                double deriv2_coeff = 0.0;
                double deriv1_coeff = 0.0; 
                double func_coeff = 0.0;       
            };
            
            BCCoeffs ParseBoundaryCondition(std::string expression);
            LOperatorCoeffs ParseLOperator(std::string expression);

            Eigen::MatrixXd GetPhysicalDifferentiationOperator();
            Eigen::MatrixXd GetTransitionOperator();
            Eigen::MatrixXd GetLOperator();
            Eigen::MatrixXd GetInverseCholeskyDecompMatrix();
            Eigen::MatrixXd GetBCTargetsChebCoeffs();

            BCCoeffs left_BC_coeffs_;
            BCCoeffs right_BC_coeffs_;
            LOperatorCoeffs L_operator_coeffs_;

            int N_;
            double x1_, x2_;
            double dtdx_;

            Eigen::MatrixXd D_hat_;
            Eigen::MatrixXd S_;
            Eigen::MatrixXd L_hat_;
            Eigen::MatrixXd A_tilde_;
            Eigen::MatrixXd R_;
            Eigen::VectorXd c0_;
            Eigen::VectorXd b_tilde_0_;





    };
}

#endif