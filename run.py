"""Portable CLI launcher; build first with python build.py."""
import argparse
from pathlib import Path
import subprocess
import sys
from build import executable


def main():
    parser = argparse.ArgumentParser(description="Replay sensor operations using the C++ CLI")
    parser.add_argument("--structure", required=True, choices=["A", "B"])
    parser.add_argument("--input", type=Path, help="operation file (default: standard input)")
    args = parser.parse_args()
    target = executable("run")
    if not target.is_file():
        parser.exit(1, "Build the application first: python build.py\n")
    try:
        if args.input:
            with args.input.open("rb") as stream:
                return subprocess.run([str(target), "--structure", args.structure], stdin=stream).returncode
        return subprocess.run([str(target), "--structure", args.structure]).returncode
    except OSError as error:
        print(f"Error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
