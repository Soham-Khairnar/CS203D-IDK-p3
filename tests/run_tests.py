from pathlib import Path
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from build import build_app, compile_cpp, executable

SOURCES = ["src/backend_factory.cpp", "src/structA/segment_tree.cpp",
           "src/structB/block_decomposition.cpp"]


def check(condition, message):
    if not condition:
        raise RuntimeError(message)


def app_tests():
    app = build_app()
    target = executable("application_test")
    compile_cpp(target, ["tests/application_test.cpp", "app/runner.cpp", *SOURCES])
    subprocess.run([str(target)], check=True)
    help_result = subprocess.run([str(app), "--help"], capture_output=True, text=True)
    check(help_result.returncode == 0 and "Usage:" in help_result.stdout, "--help failed")
    for args in [[], ["--structure", "C"], ["--structure", "A", "extra"]]:
        result = subprocess.run([str(app), *args], capture_output=True, text=True)
        check(result.returncode == 2 and not result.stdout and "Usage:" in result.stderr,
              f"argument validation failed: {args}")
    result = subprocess.run([str(app), "--structure", "B"],
                            input="STRETCH 1\n0\n", capture_output=True, text=True)
    check(result.returncode == 1 and not result.stdout and "requires Structure A" in result.stderr,
          "unsupported backend/mode failed")
    print("CLI argument/error tests passed.")


def structure_b_tests():
    target = executable("block_decomposition_test")
    compile_cpp(target, ["src/structB/block_decomposition_test.cpp",
                         "src/structB/block_decomposition.cpp"])
    subprocess.run([str(target)], check=True)


def differential_test(extra_args):
    target = executable("differential_test")
    compile_cpp(target, ["tests/differential_test.cpp", *SOURCES])
    root = Path(__file__).resolve().parents[1]
    subprocess.run([str(target), *extra_args], check=True, cwd=root)


def main():
    app_tests()
    structure_b_tests()
    differential_test(sys.argv[1:])


if __name__ == "__main__":
    main()
