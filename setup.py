from distutils.core import setup, Extension

setup(
    name='psimulation',
    version='1.0',
    ext_modules=[
        Extension("psimulation", ["sim.cpp", "mutils.cpp"])
    ],
)
