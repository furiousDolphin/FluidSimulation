#--------------------------------------------------

import sys
import os

build_path = os.path.join(os.getcwd(), 'build')
if build_path not in sys.path:
    sys.path.append(build_path)

import module_uno

#--------------------------------------------------

import numpy as np
from scripts import Plots

app = module_uno.App("1u'' + 0u' - 4u", "1u(0)=10", "1u(1)=0", 64)
f = np.zeros(500)
x, y = app.solve(f)

subplot_manager = Plots.SubplotManager((960, 540), title="subploty")
subplot = Plots.Subplot(xlabel="x", ylabel="y", title="simulation")
subplot.add(x, y, label="u(x)", color="blue")

subplot_manager.add(subplot)
subplot_manager.show()



