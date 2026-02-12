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


std::pair<Eigen::VectorXd, Eigen::VectorXd> run_cheb_test() 
{
    std::size_t N = 50;
    double x1 = -20;
    double x2 =  20;
    
    auto f = [](double x) { return std::atan(x); };


    Eigen::VectorXd nodes = FluidSimulation::ChebNodes(N, x1, x2);
    Eigen::VectorXd f_vals = nodes.unaryExpr(f);
    auto const& coeffs = FluidSimulation::ChebCoeffs(f_vals);

    Eigen::VectorXd x = Eigen::VectorXd::LinSpaced(500, x1, x2);
    Eigen::VectorXd t = ( -2.0/(x1 - x2) )*x.array() + ( (x1 + x2)/(x1 - x2) );
    Eigen::VectorXd y = FluidSimulation::EvaluateCheb(coeffs, t);

    return {x, y}; 
}


PYBIND11_MODULE(fluid_module, m) 
{
    m.doc() = "Moduł symulacji płynów z obsługą wielomianów Czebyszewa";


    m.def("run_cheb_test", &run_cheb_test, "Uruchamia test interpolacji Czebyszewa i zwraca (x, y)");


    m.def("cheb_nodes", &FluidSimulation::ChebNodes);
    m.def("cheb_coeffs", &FluidSimulation::ChebCoeffs);
    m.def("evaluate_cheb", &FluidSimulation::EvaluateCheb);
}
