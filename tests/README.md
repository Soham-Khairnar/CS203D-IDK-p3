# Testing handoff

`python tests/run_tests.py` builds and checks the application using a test-only
array backend. It covers routing, input validation, output, both modes, 1-based
boundaries, 64-bit values, and parsing one million readings. It does not establish
correctness or performance of either production data structure.

Test owner TODO:

- Implement a separate plain-array oracle; use Kadane for nonempty best stretch.
- Add unit tests with the structure owners and differential tests of at least
  100,000 fixed-seed random operations in LAZY (A/B/oracle) and STRETCH (A/oracle).
- Record seeds and operation counts; print the seed and failing operation on error.
- Include full/single-element ranges, negative values, point sets after range adds,
  repeated updates, all-negative stretches, and queries crossing block boundaries.
- Check lazy segment-tree aggregates and pending-tag consistency; check four-value
  merges in stretch mode; check block sums/minima including pending additions.
  Agree on debug inspection hooks with the structure owners.
- Build worst-case ranges touching many tree nodes or partial blocks (five-person team).
- Once backends are ready, replay `examples/*.in` against the production CLI and
  compare to the corresponding `.out` files. Compare A and B on LAZY inputs.

Suggested future files: `oracle.hpp`, `differential_test.cpp`,
`invariants.hpp`, `worst_cases.cpp`. These are pending work, not passing tests.
