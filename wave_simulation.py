

import time
import sys
import os

build_path = os.path.join(os.getcwd(), 'build')
if build_path not in sys.path:
    sys.path.append(build_path)

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import module_dos as m  


N = 64
L = 10.0
x = np.linspace(0, L, 500)  

eq = m.WaveEquation("u(0)=0", f"u({L})=0", c_sq=3000.0, gamma=20, N=N)
sim = m.WaveSimulation(eq, x)

u_init = np.zeros_like(x)
v_init = np.zeros_like(x)

u_init = 0.5 * np.exp(-((x - L/2)**2) / 0.1) 
sim.set_u_v(u_init, v_init)

fig, ax = plt.subplots()
line, = ax.plot(x, u_init, lw=2)

ax.set_ylim(-1.5, 1.5)
ax.set_title("Symulacja Spektralna Wody")

start_time = time.time()
PAUSE_DURATION = 10.0 

def update(frame):

    if time.time() - start_time < PAUSE_DURATION:
        
        return line,

    sim.step()
    u_vals, _ = sim.get_u_v()
    line.set_ydata(u_vals)
    return line,



ani = FuncAnimation(fig, update, interval=16, blit=True) 
plt.show()
