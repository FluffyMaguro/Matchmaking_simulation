from distutils.core import setup, Extension
import numpy

setup(
    name='psimulation',
    version='1.0',
    ext_modules=[
        Extension(
            "psimulation",
            [
                "cpp/sim.cpp", "cpp/strategies.cpp", "cpp/simulation.cpp",
                "cpp/main.cpp", "cpp/trueskill.cpp"
            ],
            include_dirs=[numpy.get_include()],
        )
    ],
)
