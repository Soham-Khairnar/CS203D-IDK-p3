import argparse
import csv
import json
import math
import os
import platform
import statistics
import subprocess
from collections import defaultdict
from pathlib import Path

import matplotlib.pyplot as plt

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "bench" / "build"
RESULTS = ROOT / "results" / "measurement"

# Change ONLY these settings when doing a different experiment.
QUICK = False  # Set True only to test that the program runs.
SIZES = [10000, 30000, 100000, 300000, 1000000]
OPERATIONS = 200000
REPETITIONS = 5
SEEDS = [42, 97]
PROFILES = ["short", "long", "mixed"]
BLOCK_SIZES = [128, 256, 512, 1000, 2048, 4096]
COMPILER = os.environ.get("CXX", "g++")

SOURCES = {
    "A_LAZY": ["bench/bench_A_lazy.cpp"],
    "A_STRETCH": ["bench/bench_A_stretch.cpp"],
    "B_LAZY": ["bench/bench_B_lazy.cpp", "src/structB/block_decomposition.cpp"],
}


def compile_runners():
    BUILD.mkdir(parents=True, exist_ok=True)
    programs = {}
    for name, files in SOURCES.items():
        exe = BUILD / (name.lower() + (".exe" if platform.system() == "Windows" else ""))
        command = [COMPILER, "-std=c++17", "-O2", "-Wall", "-Wextra", "-Wpedantic"]
        command += [str(ROOT / file) for file in files] + ["-o", str(exe)]
        subprocess.run(command, check=True)
        programs[name] = exe
    return programs


def execute(programs, name, n, profile, operation, seed, length=0, block_size=0,
            ops=None, verify=False):
    count = OPERATIONS if ops is None else ops
    command = [str(programs[name]), str(n), str(count), profile,
               operation, str(seed), str(length), str(block_size)]
    if verify:
        command.append("--verify")
    context = " ".join([name, *command[1:]])
    process = subprocess.run(command, capture_output=True, text=True)
    if process.returncode:
        raise RuntimeError(f"{context} failed ({process.returncode}): {process.stderr.strip()}")
    if verify:
        return
    rows = list(csv.DictReader(process.stdout.splitlines()))
    if len(rows) != 1:
        raise RuntimeError(f"{context}: expected one CSV row")
    row = rows[0]
    expected = (name, n, profile, operation, length, seed, count, block_size)
    fields = ("structure", "n", "profile", "operation", "length", "seed", "ops", "block_size")
    if any(row.get(key) != str(value) for key, value in zip(fields, expected)):
        raise RuntimeError(f"{context}: CSV workload metadata did not match the request")
    try:
        timings = [float(row[key]) for key in ("build_ms", "ns_per_op", "operation_total_ns")]
    except (KeyError, TypeError, ValueError) as error:
        raise RuntimeError(f"{context}: missing or malformed timings") from error
    if not all(math.isfinite(value) and value > 0 for value in timings):
        raise RuntimeError(f"{context}: timings must be finite and positive")
    if not math.isclose(timings[1] * count, timings[2], rel_tol=1e-10):
        raise RuntimeError(f"{context}: operation timings disagree")
    return row


def configurations():
    """(experiment, structures, n, profile, operation, length, block_size)."""
    cases = []
    for n in SIZES:
        for profile in PROFILES:
            cases.append(("shared", ["A_LAZY", "A_STRETCH", "B_LAZY"], n, profile, "US", 0, 0))
            cases.append(("lazy", ["A_LAZY", "B_LAZY"], n, profile, "LAZY", 0, 0))
            cases.append(("stretch", ["A_STRETCH"], n, profile, "STRETCH", 0, 0))

    n = max(SIZES)
    supported = {
        "U": ["A_LAZY", "A_STRETCH", "B_LAZY"],
        "A": ["A_LAZY", "B_LAZY"],
        "S": ["A_LAZY", "A_STRETCH", "B_LAZY"],
        "M": ["A_LAZY", "B_LAZY"],
        "B": ["A_STRETCH"],
    }
    for profile in ("short", "long", "full"):
        for operation, names in supported.items():
            cases.append(("operation", names, n, profile, operation, 0, 0))

    lengths = sorted({length for length in (1, 10, 100, 1000, max(1, n // 10), n)
                      if length <= n})
    for length in lengths:
        for operation in ("A", "S", "M", "B"):
            cases.append(("length", supported[operation], n, "mixed", operation, length, 0))

    for block in BLOCK_SIZES:
        for profile in ("short", "long"):
            cases.append(("block", ["B_LAZY"], n, profile, "LAZY", 0, block))
    return cases


def save_csv(path, rows):
    if not rows:
        return
    with path.open("w", newline="", encoding="utf-8") as file:
        writer = csv.DictWriter(file, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def summarize(raw):
    groups = defaultdict(list)
    fields = ("experiment", "structure", "n", "profile", "operation", "length",
              "block_size", "effective_block_size")
    for row in raw:
        groups[tuple(row[key] for key in fields)].append(row)
    result = []
    for key, trials in groups.items():
        times = [float(row["ns_per_op"]) for row in trials]
        builds = [float(row["build_ms"]) for row in trials]
        entry = dict(zip(fields, key))
        entry.update(trials=len(trials), mean_ns_per_op=statistics.mean(times),
                     stdev_ns_per_op=statistics.stdev(times) if len(times)>1 else 0,
                     mean_build_ms=statistics.mean(builds),
                     stdev_build_ms=statistics.stdev(builds) if len(builds)>1 else 0,
                     storage_bytes=int(trials[0]["storage_bytes"]))
        result.append(entry)
    return result


def lines(rows, filename, title, value, ylabel, selector, x_field="n", logx=False):
    chosen = [r for r in rows if selector(r)]
    if not chosen:
        return
    fig, ax = plt.subplots(figsize=(8, 5))
    names = sorted(set(r["structure"] for r in chosen))
    for name in names:
        entries = sorted((r for r in chosen if r["structure"] == name),
                         key=lambda r: int(r[x_field]))
        if not entries:
            continue
        xs = [int(r[x_field]) for r in entries]
        ys = [float(r[value]) for r in entries]
        errors = [float(r["stdev_ns_per_op"] if value == "mean_ns_per_op"
                        else r["stdev_build_ms"] if value == "mean_build_ms" else 0)
                  for r in entries]
        ax.errorbar(xs, ys, yerr=errors, marker="o", capsize=3, label=name)
    ax.set(title=title, xlabel="Range length" if x_field == "length" else "Array size (n)", ylabel=ylabel)
    if logx:
        ax.set_xscale("log")
    ax.legend()
    ax.grid(True, alpha=0.25)
    fig.tight_layout()
    fig.savefig(RESULTS / "figures" / filename, dpi=160)
    plt.close(fig)


def graph_operations(rows, profile):
    chosen = [r for r in rows if r["experiment"] == "operation" and r["profile"] == profile]
    if not chosen:
        return
    labels = ["U", "A", "S", "M", "B"]
    names = ["A_LAZY", "A_STRETCH", "B_LAZY"]
    fig, ax = plt.subplots(figsize=(9, 5))
    for j, name in enumerate(names):
        xs, ys, errors = [], [], []
        for i, op in enumerate(labels):
            match = [r for r in chosen if r["structure"] == name and r["operation"] == op]
            if match:
                xs.append(i + (j - 1) * 0.23)
                ys.append(float(match[0]["mean_ns_per_op"]))
                errors.append(float(match[0]["stdev_ns_per_op"]))
        ax.bar(xs, ys, yerr=errors, capsize=3, width=0.23, label=name)
    ax.set_xticks(range(len(labels)), labels)
    ax.set(xlabel="Operation", ylabel="Mean nanoseconds per operation",
           title=f"Individual operations ({profile} ranges)")
    ax.legend()
    ax.grid(axis="y", alpha=0.25)
    fig.tight_layout()
    fig.savefig(RESULTS / "figures" / f"operations_{profile}.png", dpi=160)
    plt.close(fig)


def graph_block(rows):
    chosen = [r for r in rows if r["experiment"] == "block"]
    if not chosen:
        return
    fig, ax = plt.subplots(figsize=(8, 5))
    for profile in ("short", "long"):
        data = sorted([r for r in chosen if r["profile"] == profile], key=lambda r: int(r["block_size"]))
        ax.errorbar([int(r["block_size"]) for r in data],
                    [float(r["mean_ns_per_op"]) for r in data],
                    yerr=[float(r["stdev_ns_per_op"]) for r in data],
                    marker="o", capsize=3, label=profile)
    ax.set(xlabel="Block size", ylabel="Mean nanoseconds per operation",
           title="B LAZY: effect of block size")
    ax.legend()
    ax.grid(True, alpha=0.25)
    fig.tight_layout()
    fig.savefig(RESULTS / "figures" / "block_size_effect.png", dpi=160)
    plt.close(fig)


def graph_memory(rows):
    chosen = [r for r in rows if r["experiment"] == "shared" and r["profile"] == "mixed"]
    fig, ax = plt.subplots(figsize=(8, 5))
    for name in ("A_LAZY", "A_STRETCH", "B_LAZY"):
        data = sorted([r for r in chosen if r["structure"] == name], key=lambda r: int(r["n"]))
        ax.plot([int(r["n"]) for r in data],
                [int(r["storage_bytes"]) / (1024**2) for r in data], marker="o", label=name)
    ax.set(xlabel="Array size (n)", ylabel="Allocated primary arrays (MiB)",
           title="Primary data-structure storage (excluding overhead)")
    ax.legend()
    ax.grid(True, alpha=0.25)
    fig.tight_layout()
    fig.savefig(RESULTS / "figures" / "storage_vs_n.png", dpi=160)
    plt.close(fig)


def collect(programs):
    verification = {
        "A_LAZY": ("LAZY", "US", "U", "A", "S", "M"),
        "A_STRETCH": ("STRETCH", "US", "U", "S", "B"),
        "B_LAZY": ("LAZY", "US", "U", "A", "S", "M"),
    }
    for name, operations in verification.items():
        for operation in operations:
            for n, profile, length in ((1, "short", 0), (32, "long", 0),
                                       (33, "mixed", 7), (33, "full", 0)):
                block = 5 if name == "B_LAZY" and profile == "mixed" else 0
                execute(programs, name, n, profile, operation, 42,
                        0 if operation == "U" else length, block, ops=200, verify=True)
    print("Untimed oracle checks passed.", flush=True)
    raw = []
    checksums = {}
    cases = configurations()
    for index, (experiment, names, n, profile, operation, length, block) in enumerate(cases, start=1):
        for seed in SEEDS:
            for rep in range(REPETITIONS):
                offset = rep % len(names)
                for name in names[offset:] + names[:offset]:
                    row = execute(programs, name, n, profile, operation, seed, length, block)
                    key = (n, profile, operation, seed, length)
                    reference = checksums.setdefault(key, row["checksum"])
                    if row["checksum"] != reference:
                        raise RuntimeError(
                            f"Checksum mismatch: {name}, {experiment}, n={n}, "
                            f"profile={profile}, operation={operation}, seed={seed}, "
                            f"length={length}, block={block}, repetition={rep + 1}"
                        )
                    row["experiment"] = experiment
                    row["repetition"] = rep + 1
                    raw.append(row)
        print(f"{index}/{len(cases)}: {experiment}, n={n}, {profile}, {operation}", flush=True)
    return raw


def write_results(raw, quick):
    short = [r for r in raw if float(r["operation_total_ns"]) < 5_000_000]
    if short:
        print(f"WARNING: {len(short)} batches lasted less than 5 ms; increase OPERATIONS for stable final measurements.", flush=True)

    RESULTS.mkdir(parents=True, exist_ok=True)
    (RESULTS / "figures").mkdir(exist_ok=True)
    save_csv(RESULTS / "raw_timings.csv", raw)
    summary = summarize(raw)
    save_csv(RESULTS / "summary_timings.csv", summary)

    compiler_version = subprocess.run([COMPILER, "--version"], check=True,
                                      capture_output=True, text=True).stdout.splitlines()[0]
    environment = {"quick": quick, "sizes": SIZES, "ops": OPERATIONS,
                   "repetitions": REPETITIONS, "seeds": SEEDS, "profiles": PROFILES,
                   "block_sizes": BLOCK_SIZES, "compiler": COMPILER,
                   "compiler_version": compiler_version,
                   "platform": platform.platform(),
                   "backends": "A lazy_segtree.hpp, A stretch_segtree.hpp, B block_decomposition.cpp",
                   "memory": "Only known allocated primary arrays; excludes input, workload, allocators, process overhead"}
    (RESULTS / "environment.json").write_text(json.dumps(environment, indent=2), encoding="utf-8")

    for profile in PROFILES:
        lines(summary, f"time_vs_n_{profile}.png", f"Shared U/S workload: {profile} ranges",
              "mean_ns_per_op", "Mean nanoseconds per operation",
              lambda r, p=profile: r["experiment"] == "shared" and r["profile"] == p)
        lines(summary, f"lazy_vs_n_{profile}.png", f"LAZY operations: {profile} ranges",
              "mean_ns_per_op", "Mean nanoseconds per operation",
              lambda r, p=profile: r["experiment"] == "lazy" and r["profile"] == p)
        lines(summary, f"stretch_vs_n_{profile}.png", f"STRETCH operations: {profile} ranges",
              "mean_ns_per_op", "Mean nanoseconds per operation",
              lambda r, p=profile: r["experiment"] == "stretch" and r["profile"] == p)
    lines(summary, "build_vs_n.png", "Construction time vs array size",
          "mean_build_ms", "Mean construction time (ms)",
          lambda r: r["experiment"] == "shared" and r["profile"] == "mixed")
    graph_memory(summary)
    for profile in ("short", "long", "full"):
        graph_operations(summary, profile)
    for op in ("A", "S", "M", "B"):
        lines(summary, f"length_{op}.png", f"Operation {op}: time vs range length",
              "mean_ns_per_op", "Mean nanoseconds per operation",
              lambda r, o=op: r["experiment"] == "length" and r["operation"] == o,
              x_field="length", logx=True)
    graph_block(summary)
    print(f"Saved {len(raw)} trials and figures in {RESULTS}")
    if quick:
        print("QUICK results are for testing only.")


def main():
    global SIZES, OPERATIONS, REPETITIONS, BLOCK_SIZES, SEEDS, RESULTS
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--quick", action="store_true", help="Run the smaller verification suite")
    parser.add_argument("--output", type=Path, help="Override the results directory")
    args = parser.parse_args()
    quick = QUICK or args.quick
    if quick:
        SIZES, OPERATIONS, REPETITIONS = [1000, 10000], 5000, 2
        BLOCK_SIZES, SEEDS = [16, 64, 128], [42]
    RESULTS = args.output or (RESULTS.with_name("measurement_quick") if quick else RESULTS)
    raw = collect(compile_runners())
    write_results(raw, quick)


if __name__ == "__main__":
    main()
