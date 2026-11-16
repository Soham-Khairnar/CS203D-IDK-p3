#!/usr/bin/env python3
"""Test owner's one-command entry point (standard library only).

  python tests/run_differential.py                 full suite (both structures)
  python tests/run_differential.py --only A        Structure A only (while B is unfinished)
  python tests/run_differential.py --only B
  python tests/run_differential.py --quick         small smoke test
  python tests/run_differential.py --skip-cli      skip the CLI replay stage

Stages
  0. Self-check of the oracle (Kadane vs an O(len^2) brute force on 200,000 random ranges).
  1. Build tests/differential_test.cpp with the real production sources and run it:
     oracle vs Structure A vs Structure B, >= 10^5 seeded operations per main configuration,
     invariant checks every 500 operations. Output is saved to results/differential_log.txt.
  2. Replay the bundled examples and seeded generated files through the real CLI
     (build/run --structure A|B) and diff against the oracle's answers.
  3. Replay small worst-case workloads (tests/gen_worst_cases.py --with-expected) the same way.
"""
import argparse
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from build import build_app, compile_cpp, executable  # noqa: E402

SOURCES = ["tests/differential_test.cpp", "src/backend_factory.cpp",
           "src/structA/segment_tree.cpp", "src/structB/block_decomposition.cpp"]
GENERATED = [  # (mode, n, profile, seed) -- seeds are fixed so the files are reproducible
    ("LAZY", 500, "mixed", 11), ("LAZY", 997, "edges", 14), ("LAZY", 64, "lazy_then_set", 16),
    ("STRETCH", 500, "mixed", 12), ("STRETCH", 300, "allneg", 13), ("STRETCH", 64, "edges", 15),
]


def lines(text):
    return text.replace("\r\n", "\n").split()


def cli(structure, infile):
    with open(infile, "rb") as stream:
        done = subprocess.run([str(executable("run")), "--structure", structure],
                              stdin=stream, capture_output=True, text=True)
    return done.returncode, done.stdout, done.stderr


def replay(name, infile, expected_file, structures):
    ok = True
    expected = lines(Path(expected_file).read_text())
    for s in structures:
        code, out, err = cli(s, infile)
        good = code == 0 and lines(out) == expected
        print(f"  [{'PASS' if good else 'FAIL'}] {name}  structure {s}" + ("" if good else f"  (exit {code}) {err.strip()[:120]}"))
        ok &= good
    return ok


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", choices=["A", "B"])
    ap.add_argument("--quick", action="store_true")
    ap.add_argument("--skip-cli", action="store_true")
    ap.add_argument("--ops", type=int)
    ap.add_argument("--seed", type=int)
    args = ap.parse_args()

    (ROOT / "build").mkdir(exist_ok=True)
    (ROOT / "results").mkdir(exist_ok=True)
    try:
        compile_cpp(executable("differential_test"), SOURCES)
    except subprocess.CalledProcessError:
        print("\nBUILD FAILED. The production sources do not compile/link yet (see messages above).\n"
              "Run `python build.py` to see the same error; the structure owners must fix it first.")
        return 1

    print("== Stage 0: oracle self-check ==")
    compile_cpp(executable("oracle_selfcheck"), ["tests/oracle_selfcheck.cpp"])
    selfcheck = subprocess.run([str(executable("oracle_selfcheck"))], capture_output=True, text=True)
    print(selfcheck.stdout.strip() or selfcheck.stderr.strip())
    if selfcheck.returncode != 0:
        return 1

    cmd = [str(executable("differential_test")), "--outdir", str(ROOT / "build")]
    for flag, value in (("--only", args.only), ("--ops", args.ops), ("--seed", args.seed)):
        if value is not None:
            cmd += [flag, str(value)]
    if args.quick:
        cmd.append("--quick")
    print("== Stage 1: differential test (oracle vs structures) ==")
    run = subprocess.run(cmd, capture_output=True, text=True)
    log = run.stdout + run.stderr
    (ROOT / "results" / "differential_log.txt").write_text(log, encoding="utf-8")
    print(log if run.returncode else "\n".join(log.splitlines()[-4:]))
    ok = run.returncode == 0
    if not ok:
        print("Differential test FAILED. Full log: results/differential_log.txt")
    if args.skip_cli:
        return 0 if ok else 1

    print("\n== Stage 2/3: CLI replay against oracle answers ==")
    try:
        build_app()
    except subprocess.CalledProcessError:
        print("Could not build the CLI (build.py failed).")
        return 1
    lazy_structs = [args.only] if args.only else ["A", "B"]
    stretch_structs = ["A"] if args.only in (None, "A") else []
    work = ROOT / "build" / "replay"
    work.mkdir(exist_ok=True)

    for name, structs in (("lazy", lazy_structs), ("stretch", stretch_structs)):
        if structs:
            ok &= replay(f"example {name}", ROOT / "tests/examples" / f"{name}.in",
                         ROOT / "tests/examples" / f"{name}.out", structs)
    for mode, n, profile, seed in GENERATED:
        structs = lazy_structs if mode == "LAZY" else stretch_structs
        if not structs:
            continue
        prefix = work / f"gen_{mode}_{n}_{profile}"
        subprocess.run([str(executable("differential_test")), "--emit", str(prefix), "--mode", mode,
                        "--n", str(n), "--ops", "4000", "--profile", profile, "--seed", str(seed)], check=True)
        ok &= replay(f"{mode} n={n} {profile} seed={seed}", f"{prefix}.in", f"{prefix}.out", structs)

    subprocess.run([sys.executable, str(ROOT / "tests" / "gen_worst_cases.py"), "--n", "400", "1000",
                    "--ops", "1500", "--seed", "3", "--outdir", str(work / "worst"), "--with-expected"],
                   check=True, capture_output=True)
    for infile in sorted((work / "worst").glob("*.in")):
        ok &= replay(f"worst-case {infile.name}", infile, infile.with_suffix(".out"), lazy_structs)

    print("\nOVERALL:", "ALL PASSED" if ok else "FAILURES (see above)")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
