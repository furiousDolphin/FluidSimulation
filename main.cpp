#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <Eigen/Dense>
#include <boost/numeric/odeint.hpp>
#include <boost/numeric/odeint/external/eigen/eigen.hpp>
#include <unsupported/Eigen/FFT>

#include "MatrixBuilders.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <pybind11/stl.h>

namespace py = pybind11;


std::pair<Eigen::VectorXd, Eigen::VectorXd> run_simulation(int N) 
{
    double x1 = 0;
    double x2 = 1;
    
    auto f = [](double x) { return std::sin(M_PI*x); };

    Eigen::VectorXd nodes = FluidSimulation::ChebNodes(N, x1, x2);
    Eigen::VectorXd f_vals = nodes.unaryExpr(f);

    Eigen::VectorXd f_hat = FluidSimulation::ChebCoeffs(f_vals);
    Eigen::MatrixXd L_hat = FluidSimulation::HelmholtzMatrix(N);

    Eigen::VectorXd d = FluidSimulation::SolveHelmholtzDiffEq(L_hat, f_hat);
    Eigen::MatrixXd S = FluidSimulation::TransitionMatrix(N);
    Eigen::VectorXd c = S*d;

    Eigen::VectorXd x = Eigen::VectorXd::LinSpaced(500, x1, x2);
    Eigen::VectorXd t = ( -2.0/(x1 - x2) )*x.array() + ( (x1 + x2)/(x1 - x2) );
    Eigen::VectorXd y = FluidSimulation::EvaluateCheb(c, t);

    return {x, y}; 
}


PYBIND11_MODULE(fluid_module, m) 
{
    m.doc() = "Moduł symulacji płynów z obsługą wielomianów Czebyszewa";
    m.def("run_simulation", &run_simulation, "przyjmuje N będące najwyższym stopniem wieomianu bazy cheb, zwraca (x, y) (odpowiedź na wymuszenie) ");
}




