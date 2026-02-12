import sys
import os
import time


build_path = os.path.join(os.getcwd(), 'build')
if build_path not in sys.path:
    sys.path.append(build_path)


import fluid_module


res = fluid_module.add(2, 6)
print(res)