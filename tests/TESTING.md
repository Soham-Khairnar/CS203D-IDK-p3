# Testing (Test owner: Shrawan Kumar Prajapati)

One command from the repository root:

```text
python tests/run_differential.py            # everything (needs both structures to build)
python tests/run_differential.py --only A   # Structure A only
python tests/run_differential.py --only B   # Structure B only
python tests/run_differential.py --quick    # smoke test, a few seconds
```

Output is saved to `results/differential_log.txt`. On the first failure the program prints
the **seed, configuration, operation number, the operation, expected and actual answer**, and
writes a replayable file `build/fail_*.in` (replay: `build/run --structure A < build/fail_*.in`).

## Files

| File | Purpose |
|---|---|
| `tests/oracle.hpp` | Answer key: plain array + loops, Kadane for best stretch. Shares no code with either structure. Detects int64 overflow. |
| `tests/oracle_selfcheck.cpp` | Checks the oracle itself: Kadane == O(len^2) brute force on 200,000 random ranges (a third all-negative). |
| `tests/workload.hpp` | Seeded generator for operation files (section-4 format). Own RNG helper, so a seed gives the same file on every compiler. |
| `tests/invariants.hpp` | Invariant checker: black-box properties + optional white-box hook. |
| `src/inspectable.hpp` | Tiny interface a structure may implement to expose `selfCheck()`. |
| `tests/differential_test.cpp` | Oracle vs Structure A vs Structure B, with invariant checks. |
| `tests/run_differential.py` | Builds and runs everything, then replays files through the real CLI. |
| `tests/worst_case_model.py` | Analytic cost models and brute-force evidence for the worst-case ranges. |
| `tests/gen_worst_cases.py` | Writes worst-case / best-case / random workloads (default `build/workloads/`). |

## What the differential test covers

Base seed `203003`; configuration *i* uses seed `203003 + i` (all printed in the log).

* **Sizes:** n = 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65, 100, 101 (3,000 ops each,
  mixed and edge profiles); n = 1000, 1024, 997 with 100,000 ops; n = 20,000 with 20,000 ops.
  Covers powers of two, primes, and n not divisible by the block size.
* **Profiles:** `mixed` (incl. single element and full range), `short` (length 1-3), `long` (>= 90% of n),
  `edges` (endpoints at 1, 2, 3, n-2, n-1, n and at block boundaries kB-1, kB, kB+1), `lazy_then_set`
  (repeating cycle: add on a range, point-set inside it, then sum and min on it), `big` (values near 10^9,
  add size capped so no int64 overflow is possible), `allneg` (all-negative stretches, STRETCH mode).
* **Modes:** LAZY runs oracle vs A vs B; STRETCH runs oracle vs A.

## Invariants checked every 500 operations

Black-box (any structure, public API only):

1. *Point sweep:* every position reads back its true value through `S`, and through `M` (LAZY) or `B` (STRETCH).
   Guards against stale lazy tags and blocks that were not rebuilt.
2. *Additivity:* `sum(l,r) = sum(l,m) + sum(m+1,r)` for random l <= m < r.
3. *Min of halves:* `min(l,r) = min(min(l,m), min(m+1,r))` (LAZY).
4. *Stretch bounds* (STRETCH): `best(l,r) >= best` of each half and `>= sum(l,r)` (the whole range is itself a stretch).
   Catches a dropped cross term in the four-value merge.

White-box (only if the structure implements `sensor::Inspectable`): stored sum/min equal the values recomputed
from the children plus the pending add; in STRETCH mode each node's four values equal `combine(left, right)`;
each block's sum/min/max equal the values recomputed from its elements plus the pending add.
It finds a corrupted update at the update itself, not at the first query that happens to touch the damage.

## Mutation testing (how "the tests really catch bugs" is shown)

Deliberately break a copy of the code, run `python tests/run_differential.py --only A` (or `B`),
expect `FAIL`, then undo with `git checkout -- <file>`. Never commit a mutant.
Suggested mutants: forget to push the pending add down; drop the cross term `L.suf + R.pref`;
off-by-one on the right end of a range add; point-set without clearing the pending add;
whole-block add that forgets to update the block minimum; loop one block too far.

## Worst-case workloads

```text
python tests/worst_case_model.py                                   # brute-force evidence
python tests/gen_worst_cases.py --n 10000 100000 1000000 --seed 1  # files for the Measurement owner
python tests/gen_worst_cases.py --n 1000 --ops 2000 --with-expected  # small, with expected answers
```
`manifest.csv` next to the files records seed, block size, and the model cost per range.
