# Measurement handoff / Q5

Entry point: `python bench/benchmark.py` (currently an explicit placeholder).

After both structures pass differential tests:

1. Generate identical seeded LAZY workloads for A and B, at n from 10^4 to 10^6.
2. Separate short/long ranges and operation types; record the operation mix and seeds.
3. Average at least five timed runs per configuration. Record build time separately;
   identify whether timings include input parsing/process startup or only operations.
4. Sweep block sizes around sqrt(n). Measure memory with a documented method.
5. Save raw CSV in results/ with n, structure, block size, range length, operation,
   seed, repetition, time and units, and memory and units. Record machine/OS/compiler.
6. Generate labelled figures from raw CSV. Compare O(log n) and O(sqrt(n)); explain
   where each backend wins. Coordinate worst-case workloads with the Test owner.
7. Make this entry point regenerate the data and figures; complete REPORT sections
   6 and Q5 with actual measurements.
