"""Build the application with GCC. Usage: python build.py"""
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parent


def compile_cpp(output, sources):
    output.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run([
        os.environ.get("CXX", "g++"), "-std=c++17", "-O2", "-Wall", "-Wextra",
        "-Wpedantic", *[str(ROOT / source) for source in sources], "-o", str(output),
    ], check=True, cwd=ROOT)


def executable(name):
    return ROOT / "build" / (name + (".exe" if os.name == "nt" else ""))


def build_app():
    target = executable("run")
    compile_cpp(target, ["app/main.cpp", "app/runner.cpp", "src/backend_factory.cpp",
                         "src/structA/segment_tree.cpp", "src/structB/block_decomposition.cpp"])
    return target


if __name__ == "__main__":
    print(f"Built {build_app()}")
