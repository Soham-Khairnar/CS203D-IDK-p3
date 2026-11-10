# Structure B handoff: Smita / Q2

Implement `BlockDecomposition` in the provided header/source. The constructor
currently throws intentionally. This backend supports LAZY only.

- Keep readings plus sum, minimum, and pending addition per block.
- Choose a default block size near sqrt(n); allow overriding it for benchmarks.
- Handle full blocks through aggregates and partial blocks element by element.
- Define whether raw readings exclude the pending tag, and maintain that convention.
- Point assignment must account for the pending addition; rebuild the affected
  minimum if needed. A point assignment is an absolute value, not an increment.
- Implement range add, range sum, and range minimum. Keep bestStretch unsupported.
- Use 64-bit arithmetic, O(n) storage, and O(sqrt(n)) operations for the default size.
- Add unit tests, including a short final block and partial-boundary updates.
- Complete Q2 in REPORT.md and agree on invariant hooks with the Test owner.

The factory already routes Structure B LAZY files here. Do not print from methods.
