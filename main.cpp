#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif

#include <SDL.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;


int add(int i, int j) {
    return i + j;
}

PYBIND11_MODULE(fluid_module, m) 
{
    m.doc() = "Minimalny modul fluid_module";
    m.def("add", &add, "Funkcja dodajaca dwa parametry");
}