# Structure B: Smita / Q2

`BlockDecomposition` implements the LAZY backend. It stores raw readings in `a_`
and, for each block, its sum, minimum and pending addition. The raw readings do
not include the pending tag.

- Keep readings plus sum, minimum, and pending addition per block.
- Choose a default block size near sqrt(n); allow overriding it for benchmarks.
- Handle full blocks through aggregates and partial blocks element by element.
- Define whether raw readings exclude the pending tag, and maintain that convention.
- Point assignment first pushes the pending value, then replaces the reading with
  the requested absolute value and rebuilds the block aggregates.
- Full-block additions update its tag, sum and minimum in O(1). Partial blocks are
  pushed and updated reading by reading. Sum and minimum queries use aggregates for
  full blocks and inspect readings at the boundaries.
- The default block size is floor(sqrt(n)); the constructor accepts an override for
  benchmark sweeps. Storage is O(n); default operations take O(sqrt(n)).
- `checkInvariants()` and `Inspectable::selfCheck()` verify each block's sum and
  minimum against its raw readings plus its pending value.
- `bestStretch` is unsupported; STRETCH mode uses Structure A only.

Unit coverage is in `block_decomposition_test.cpp` and the seeded integration test
is in `tests/differential_test.cpp`.

The factory routes Structure B LAZY files here. Backend methods do not print.
