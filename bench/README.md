# Measurement handoff / Q5

Entry point: `python bench/benchmark.py`.

The script compiles dedicated LAZY-A, STRETCH-A and LAZY-B benchmark runners,
performs untimed oracle checks, runs the configured workload matrix and regenerates
raw CSV, summary CSV, environment metadata and labelled figures. Use
`python bench/benchmark.py --quick` for a smoke run. Use `--output DIR` to place
generated results elsewhere; for example, `--output build/benchmark-check` keeps
the checked-in measurement artifacts untouched.

The runner warns when timed batches are shorter than 5 ms because those timings
may be noisy. The current 200,000-operation runs produced some such batches on
this machine, especially for short ranges and the quick profile. For final report
measurements, increase `OPERATIONS` in `benchmark.py` until the warning is gone
or explain the short-batch limitation alongside the results.

The recorded full configuration uses both structures' benchmark runners:

1. Generate identical seeded LAZY workloads for A and B, at n from 10^4 to 10^6.
2. Separate short/long ranges and operation types; record the operation mix and seeds.
3. Average at least five timed runs per configuration. Record build time separately;
   identify whether timings include input parsing/process startup or only operations.
4. Sweep block sizes around sqrt(n). Measure memory with a documented method.
5. Save raw CSV in results/ with n, structure, block size, range length, operation,
   seed, repetition, time and units, and memory and units. Record machine/OS/compiler.
6. Generate labelled figures from raw CSV. Compare O(log n) and O(sqrt(n)); explain
   where each backend wins. Coordinate worst-case workloads with the Test owner.
7. The current runner regenerates data and figures. Update REPORT section 6 and Q5
   when adopting a new run, explaining the environment and any changed settings.
