
#ifndef WAVE_EUATION_HPP_
#define WAVE_EUATION_HPP_

#include <boost/numeric/odeint.hpp>
#include <boost/numeric/odeint/external/eigen/eigen.hpp>
#include <unsupported/Eigen/FFT>
#include <Eigen/Dense>

#include <vector>
#include <cmath>

namespace FluidSimulation
{

    class WaveSimulation;
    class WaveEquation
    {
        public:
            WaveEquation(std::string left_BC_str, std::string right_BC_str, double c_sq, double gamma, int N = 32);
            friend WaveSimulation;
            void operator()(const Eigen::VectorXd& X, Eigen::VectorXd& dXdt, double t);


        private:
            struct BCCoeffs 
            {
                double deriv1_coeff = 0.0; 
                double func_coeff = 0.0;  
                double target = 0.0;     
                double boundary_x = 0.0;
            };

            
            
            BCCoeffs ParseBoundaryCondition(std::string expression);

            Eigen::MatrixXd GetPhysicalDifferentiationOperator();
            Eigen::MatrixXd GetTransitionOperator();
            Eigen::MatrixXd GetInverseCholeskyDecompMatrix();
            Eigen::MatrixXd GetBCTargetsChebCoeffs();

            BCCoeffs left_BC_coeffs_;
            BCCoeffs right_BC_coeffs_;

            double c_sq_;
            double gamma_;

            int N_;
            double x1_, x2_;
            double dtdx_;

            Eigen::MatrixXd D_hat_;
            Eigen::MatrixXd D_hat_sq_;
            Eigen::MatrixXd S_;
            Eigen::MatrixXd A_tilde_;
            Eigen::MatrixXd R_;
            Eigen::VectorXd c0_;
            Eigen::VectorXd b_tilde_0_;
            Eigen::ColPivHouseholderQR<Eigen::MatrixXd> RS_QR_;


    };


    class WaveSimulation
    {
        public:
            WaveSimulation(const WaveEquation& wave_equation, const Eigen::VectorXd& x_dense);
  
            void set_u_v(const Eigen::VectorXd& u_dense, const Eigen::VectorXd& v_dense);
            void step();
            std::pair<Eigen::VectorXd, Eigen::VectorXd> get_u_v();


        private:
            const WaveEquation& eq_;
            Eigen::VectorXd cheb_t_dense_;
            Eigen::VectorXd cheb_nodes_;
            Eigen::VectorXd x_dense_;
            Eigen::VectorXd state_;
            double t_;
            double dt_;
            boost::numeric::odeint::runge_kutta4<Eigen::VectorXd> stepper_;


    };

}

#endif