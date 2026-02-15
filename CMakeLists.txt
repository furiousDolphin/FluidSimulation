

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

add_library(base_module_library INTERFACE)

target_include_directories(base_module_library INTERFACE 
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Python_INCLUDE_DIRS}
    "E:/boost_math_cpp/boost_1_88_0"
    "${SDL2_PATH}/include"
    "${SDL2_PATH}/include/SDL2"
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(base_module_library INTERFACE 
    eigen 
    Python::Module
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    imm32.lib winmm.lib version.lib setupapi.lib
)

target_compile_definitions(base_module_library INTERFACE WITHOUT_NUMPY SDL_MAIN_HANDLED)

#---------------------------------------------------------

pybind11_add_module(module_uno modules/module_uno.cpp src/MatrixBuilders.cpp)
target_link_libraries(module_uno PRIVATE base_module_library)

#---------------------------------------------------------

pybind11_add_module(module_dos modules/module_dos.cpp src/WaveEquation.cpp src/MatrixBuilders.cpp) 
target_link_libraries(module_dos PRIVATE base_module_library)

#---------------------------------------------------------

