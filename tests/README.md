# Tests (Test owner: Shrawan Kumar Prajapati)

| File | Purpose |
|---|---|
| `oracle.hpp` | Plain-array oracle: loops for sum/min/add, Kadane for non-empty best stretch |
| `invariants.hpp` | Invariant checker: black-box (public API) and white-box (`Inspectable::selfCheck`) |
| `workload.hpp` | Seeded workload generator, 7 profiles: mixed, short, long, edges, lazy_then_set, allneg, big |
| `differential_test.cpp` | Oracle vs Structure A vs Structure B; prints seed, config and failing operation |
| `gen_worst_cases.py` | Worst-case, best-case and random LAZY inputs in the section 4 format |
| `worst_case_model.py` | Cost model used to find and justify worst-case ranges |
| `application_test.cpp` | CLI/application tests (Application owner) |
| `examples/` | Sample `.in` / `.out` files in the examiner format |

## Run (from the repository root)

    python tests/run_tests.py                 # application tests + full differential suite
    python tests/run_tests.py --only A        # one structure while the other is unfinished
    python tests/run_tests.py --quick         # smoke test
    python tests/run_tests.py --ops 100000 --seed 203003

The main configurations use 100,000 operations each with fixed base seed 203003
(each configuration uses base seed + its index). On a mismatch the test prints the
seed, config, operation number, expected and actual values, and writes a replayable
`build/fail_*.in` file.

## Worst-case inputs

    python tests/gen_worst_cases.py --n 10000 100000 1000000 --ops 100000 --seed 1
    python tests/worst_case_model.py          # brute-force evidence for the model

Files go to `build/workloads/` with a `manifest.csv` listing file, kind, n, ops, seed and model cost.

## Limitation

Differential and invariant tests only pass once Structure A and B are implemented.
White-box checks need the backend to inherit `sensor::Inspectable`; otherwise only
black-box checks run.