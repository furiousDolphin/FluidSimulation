
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
    Eigen::MatrixXd TransitionMatrix(int N);
    Eigen::MatrixXd HelmholtzMatrix(int N);
    Eigen::VectorXd ChebNodes(int N, double x1, double x2);
    Eigen::VectorXd EvaluateCheb( const Eigen::VectorXd& c, const Eigen::VectorXd& Xi );
    Eigen::VectorXd ChebCoeffs(const Eigen::VectorXd& f_values);
    Eigen::MatrixXd ChebMassMatrix(int N);
    Eigen::VectorXd SolveHelmholtzDiffEq(const Eigen::MatrixXd& L_hat, const Eigen::VectorXd& f_hat);
}

#endif