#--------------------------------------------------

import sys
import os

build_path = os.path.join(os.getcwd(), 'build')
if build_path not in sys.path:
    sys.path.append(build_path)

import fluid_module

#--------------------------------------------------

import numpy as np
from scripts import Plots

x, y = fluid_module.run_simulation(50)

subplot_manager = Plots.SubplotManager((960, 540), title="subploty")
subplot = Plots.Subplot(xlabel="x", ylabel="y", title="simulation")
subplot.add(x, y, label="u(x)", color="blue")

subplot_manager.add(subplot)
subplot_manager.show()



