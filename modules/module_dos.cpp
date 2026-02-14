
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

#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <pybind11/stl.h>

namespace py = pybind11;

struct BCCoeffs {
    double deriv1_coeff = 0.0; 
    double func_coeff = 0.0;  
    double target = 0.0;     
    double boundary_x = 0.0;
};

BCCoeffs ParseBoundaryCondition(std::string expression) {
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




struct LOperatorCoeffs 
{
    double deriv2_coeff = 0.0;
    double deriv1_coeff = 0.0; 
    double func_coeff = 0.0;       
};

LOperatorCoeffs ParseLOperator(std::string expression) {
    LOperatorCoeffs coeffs;
    std::smatch match;

    //-----------------------------------------------------------------------------
    std::regex deriv2_regex(R"(([-+]?\d*\.?\d*)\s*u'')");
    //-----------------------------------------------------------------------------
    std::regex deriv1_regex(R"(([-+]?\d*\.?\d*)\s*u')");
    //-----------------------------------------------------------------------------
    std::regex func_regex(R"(([-+]?\d*\.?\d*)\s*u)");
    //-----------------------------------------------------------------------------


    if (std::regex_search(expression, match, deriv2_regex)) 
    {
        std::string val = match[1].str();
        coeffs.deriv2_coeff = (val == "" || val == "+") ? 1.0 : (val == "-") ? -1.0 : std::stod(val);
    }

    if (std::regex_search(expression, match, deriv1_regex)) 
    {
        std::string val = match[1].str();
        coeffs.func_coeff = (val == "" || val == "+") ? 1.0 : (val == "-") ? -1.0 : std::stod(val);
    }
    
    if (std::regex_search(expression, match, func_regex)) 
    { coeffs.func_coeff = std::stod(match[1].str()); }

    return coeffs;
}


Eigen::VectorXd InterpolateToChebNodes(
    const Eigen::VectorXd& x_dense, 
    const Eigen::VectorXd& y_dense, 
    const Eigen::VectorXd& cheb_nodes)
{
    std::vector<double> x_vec{ x_dense.data(), x_dense.data() + x_dense.size() };
    std::vector<double> y_vec{ x_dense.data(), x_dense.data() + x_dense.size() };

    auto spline = boost::math::interpolators::makima<std::vector<double>>(std::move(x_vec), std::move(y_vec));

    Eigen::VectorXd y_at_cheb{cheb_nodes.size()};

    double x_min = x_dense.minCoeff();
    double x_max = x_dense.maxCoeff();

    for ( int i = 0; i < cheb_nodes.size(); i++ )
    { 
        double target_x = std::clamp(cheb_nodes(i), x_min, x_max);
        y_at_cheb(i) = spline(target_x);
    }
    return y_at_cheb;
}






// BCCoeffs bc = parseBoundaryCondition("2u'(1) + 1u(1) = 0");

// auto L2 = [bc](int n) {
//     // 2*u'(1) + 1*u(1) staje się:
//     // bc.deriv_coeff * (2 * n^2) + bc.func_coeff * (1)
//     return bc.deriv_coeff * (2.0 * n * n) + bc.func_coeff * 1.0;
// };

// solver.set_bc(left="1u(0) = 0", right="2u'(1) + 1u(1) = 0")