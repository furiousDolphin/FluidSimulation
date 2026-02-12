

cmake_minimum_required(VERSION 3.15)
project(FluidSimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules)

set(SDL2_PATH "E:/SDL2_VC/SDL2-2.32.4")
set(CMAKE_PREFIX_PATH ${SDL2_PATH})

include(FetchContent)
FetchContent_Declare(
  eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 5.0.0  
)
FetchContent_MakeAvailable(eigen)

find_package(Python COMPONENTS Interpreter Development REQUIRED)
add_subdirectory(external/pybind11)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)

#---------------------------------------------------------

pybind11_add_module(fluid_module main.cpp)

target_include_directories(fluid_module PRIVATE 
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Python_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIR}
    ${SDL2_IMAGE_INCLUDE_DIR}
    ${SDL2_TTF_INCLUDE_DIR}
    ${SDL2_MIXER_INCLUDE_DIR}
    "E:/boost_math_cpp/boost_1_88_0"
)

target_link_libraries(fluid_module PRIVATE 
    eigen 
    Python::Module
    ${SDL2_LIBRARY}
    ${SDL2_IMAGE_LIBRARY}
    ${SDL2_TTF_LIBRARY}
    ${SDL2_MIXER_LIBRARY}
    imm32.lib
    winmm.lib
    version.lib
    setupapi.lib
)

target_compile_definitions(fluid_module PRIVATE WITHOUT_NUMPY SDL_MAIN_HANDLED)

#---------------------------------------------------------

