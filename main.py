import sys
import os
import time


build_path = os.path.join(os.getcwd(), 'build')
if build_path not in sys.path:
    sys.path.append(build_path)


import numpy as np
from scripts import Plots
import fluid_module

x, y = fluid_module.run_cheb_test()

subplot_manager = Plots.SubplotManager((960, 540), title="subploty")
subplot = Plots.Subplot(xlabel="x", ylabel="y", title="cheb test res")
subplot.add(x, y, label="cheb approx", color="blue")
subplot.add(x, np.atan(x), label="atan(x)", color="red")

subplot_manager.add(subplot)
subplot_manager.show()



