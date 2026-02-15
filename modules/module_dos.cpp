
#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>
#include <regex>
#include <string>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <Eigen/Dense>
#include <boost/numeric/odeint.hpp>
#include <boost/numeric/odeint/external/eigen/eigen.hpp>
#include <boost/math/interpolators/makima.hpp>
#include <unsupported/Eigen/FFT>

#include "MatrixBuilders.hpp"
#include "WaveEquation.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <pybind11/stl.h>

namespace py = pybind11;


PYBIND11_MODULE(module_dos, m)
{
    m.doc() = "";

    py::class_<FluidSimulation::WaveEquation>(m, "WaveEquation")
        .def(py::init<std::string, std::string, double, double, int>(), 
             py::arg("left_BC_str"),
             py::arg("right_BC_str"),
             py::arg("c_sq"),
             py::arg("gamma"),
             py::arg("N"));
    
    py::class_<FluidSimulation::WaveSimulation>(m, "WaveSimulation")
        .def(py::init<FluidSimulation::WaveEquation&, const Eigen::VectorXd&>(),
             py::arg("wave_equation"),
             py::arg("x_dense"))
        .def("set_u_v", 
             &FluidSimulation::WaveSimulation::set_u_v, 
             "opis",
             py::arg("u_dense"),
             py::arg("v_dense"))
        .def("step", 
             &FluidSimulation::WaveSimulation::step,
             "opis")
        .def("get_u_v", 
             &FluidSimulation::WaveSimulation::get_u_v,
             "opis");
}

