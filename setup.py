from distutils.core import setup, Extension
import numpy

setup(
    name='psimulation',
    version='1.0',
    ext_modules=[
        Extension("psimulation", ["sim.cpp"],
                  include_dirs=[numpy.get_include()])
    ],
)
