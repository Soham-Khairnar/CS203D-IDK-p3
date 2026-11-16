#!/usr/bin/env python3
"""Generate worst-case / best-case / random LAZY workloads in the section-4 input format.

Examples (run from the repository root; standard library only):
  python tests/gen_worst_cases.py --n 10000 100000 1000000 --ops 100000 --seed 1
  python tests/gen_worst_cases.py --n 1000 --ops 2000 --with-expected      # small, verifiable
  python tests/gen_worst_cases.py --n 1000000 --block-size 500             # for block-size sweeps

Files written to --outdir (default build/workloads, which .gitignore already excludes):
  tree_worst_n<N>.in    ranges that make the segment-tree query visit the most nodes,
                        in a cycle  A l r x / S l r / M l r  so lazy tags are always pending
  tree_best_n<N>.in     whole-array ranges: a segment tree only ever touches the root
  block_worst_n<N>.in   ranges that make block decomposition scan both end blocks fully and
                        step over every whole block between them
  short_n<N>.in         ranges of length 1..10 at random positions (cheap for blocks)
  random_n<N>.in        uniformly random ranges (baseline)
  manifest.csv          file, kind, n, ops, seed, block size, and the MODEL cost per range
                        (tree nodes visited / block element touches + block steps)

--with-expected also writes <file>.out from a plain-Python oracle, so the CLI can be diffed
against it. Only sensible for small n (the oracle is O(n) per operation).
"""
import argparse
import csv
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from worst_case_model import (block_cost, block_worst_pool, default_block_size,  # noqa: E402
                              tree_visits, tree_worst_pool)

KINDS = ["tree_worst", "tree_best", "block_worst", "short", "random"]


def range_stream(kind, n, B, rng, pool):
    """Yield one (l, r) per cycle of three operations."""
    while True:
        if kind == "tree_worst" or kind == "block_worst":
            yield rng.choice(pool)
        elif kind == "tree_best":
            yield (1, n)
        elif kind == "short":
            length = rng.randint(1, min(n, 10))
            l = rng.randint(1, n - length + 1)
            yield (l, l + length - 1)
        else:  # random
            a, b = rng.randint(1, n), rng.randint(1, n)
            yield (min(a, b), max(a, b))


def make_file(kind, n, ops, seed, B):
    rng = random.Random(f"{kind}:{n}:{seed}")  # string seeds are deterministic in Python 3
    init = [rng.randint(-1000, 1000) for _ in range(n)]
    pool = None
    if kind == "tree_worst":
        pool = tree_worst_pool(n, rng)
    elif kind == "block_worst":
        pool = block_worst_pool(n, B)
    lines, ranges = [], []
    stream = range_stream(kind, n, B, rng, pool)
    while len(lines) < ops:
        l, r = next(stream)
        ranges.append((l, r))
        for text in (f"A {l} {r} {rng.randint(-100, 100)}", f"S {l} {r}", f"M {l} {r}"):
            if len(lines) < ops:
                lines.append(text)
    return init, lines, ranges


def oracle_answers(init, lines):
    a = list(init)
    out = []
    for text in lines:
        t = text.split()
        if t[0] == "A":
            l, r, x = int(t[1]), int(t[2]), int(t[3])
            for i in range(l - 1, r):
                a[i] += x
        elif t[0] == "S":
            out.append(sum(a[int(t[1]) - 1:int(t[2])]))
        elif t[0] == "M":
            out.append(min(a[int(t[1]) - 1:int(t[2])]))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--n", type=int, nargs="+", default=[10000, 100000, 1000000])
    ap.add_argument("--ops", type=int, default=100000, help="operations per file")
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--block-size", type=int, default=0, help="override B (default floor(sqrt(n)))")
    ap.add_argument("--outdir", default="build/workloads")
    ap.add_argument("--only", nargs="+", choices=KINDS, default=KINDS)
    ap.add_argument("--with-expected", action="store_true")
    args = ap.parse_args()

    os.makedirs(args.outdir, exist_ok=True)
    rows = []
    for n in args.n:
        B = args.block_size or default_block_size(n)
        for kind in args.only:
            init, lines, ranges = make_file(kind, n, args.ops, args.seed, B)
            name = f"{kind}_n{n}.in"
            path = os.path.join(args.outdir, name)
            with open(path, "w", newline="\n") as f:
                f.write(f"LAZY {n}\n" + " ".join(map(str, init)) + "\n" + "\n".join(lines) + "\n")
            if args.with_expected:
                with open(path[:-3] + ".out", "w", newline="\n") as f:
                    f.write("\n".join(map(str, oracle_answers(init, lines))) + "\n")
            tv = sum(tree_visits(n, l, r) for l, r in ranges) / len(ranges)
            bc = sum(block_cost(n, B, l, r) for l, r in ranges) / len(ranges)
            rows.append([name, kind, n, len(lines), args.seed, B, f"{tv:.1f}", f"{bc:.1f}"])
            print(f"wrote {path}   mean tree nodes/range {tv:7.1f}   mean block work/range {bc:9.1f}")

    with open(os.path.join(args.outdir, "manifest.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["file", "kind", "n", "ops", "seed", "block_size",
                    "model_tree_nodes_per_range", "model_block_work_per_range"])
        w.writerows(rows)


if __name__ == "__main__":
    main()
