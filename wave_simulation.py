


import sys
import os

build_path = os.path.join(os.getcwd(), 'build')
if build_path not in sys.path:
    sys.path.append(build_path)

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import module_dos as m  # Twoja biblioteka C++

# 1. Konfiguracja parametrów
N = 64
L = 10.0
x = np.linspace(0, L, 500)  # Gęsta siatka do rysowania

# 2. Inicjalizacja silnika C++
# (Ustawiamy c_sq i gamma według uznania)
eq = m.WaveEquation("u(0)=0", f"u({L})=0", c_sq=2000.0, gamma=10, N=N)
sim = m.WaveSimulation(eq, x)

# 3. Ustawienie warunku początkowego (np. uderzenie w środek)
u_init = np.zeros_like(x)
v_init = np.zeros_like(x)
# Robimy "wgłębienie" na środku
u_init = 0.5 * np.exp(-((x - L/2)**2) / 0.1) 
sim.set_u_v(u_init, v_init)

# 4. Konfiguracja wykresu Matplotlib
fig, ax = plt.subplots()
line, = ax.plot(x, u_init, lw=2)
ax.set_ylim(-1.5, 1.5)
ax.set_title("Symulacja Spektralna Wody")

def update(frame):
    # Wykonujemy krok symulacji w C++
    sim.step()
    
    # Pobieramy nowe dane
    u_vals, _ = sim.get_u_v()
    
    # Aktualizujemy linię na wykresie
    line.set_ydata(u_vals)
    return line,

# 5. Odpalenie animacji
ani = FuncAnimation(fig, update, interval=16, blit=True) # ~60 FPS
plt.show()
