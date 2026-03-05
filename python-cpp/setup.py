from setuptools import setup, Extension
import pybind11
from pybind11.setup_helpers import Pybind11Extension, build_ext
from pybind11.setup_helpers import ParallelCompile, naive_recompile

ParallelCompile("NPY_NUM_BUILD_JOBS").install()

ext_modules = [
    Pybind11Extension(
        "fast_processor",
        ["src/fast_processor.cpp"],
        cxx_std=17,
        include_dirs=[pybind11.get_include()],
        extra_compile_args=["-O3", "-march=native", "-pthread"],
    ),
]

setup(
    name="fast_processor",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.8",
)
