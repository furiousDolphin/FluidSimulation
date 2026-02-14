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


PYBIND11_MODULE(module_uno, m) 
{
    m.doc() = "";
    
    py::class_<FluidSimulation::App>(m, "App")
        .def(py::init<std::string, std::string, std::string, int>(),
            py::arg("L_operator_str"),
            py::arg("left_BC_str"),
            py::arg("left_BC_str"),
            py::arg("N"))
        .def("solve", 
             &FluidSimulation::App::solve, 
             "rozwiazuje uklad i zwraca (x, y)",
             py::arg("f_dense"));
}