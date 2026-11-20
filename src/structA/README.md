# Structure A handoff: Soham / Q1

The standalone LAZY and STRETCH trees are implemented in `lazy_segtree.hpp` and
`stretch_segtree.hpp`. `segment_tree.hpp/.cpp` adapt them to the shared
`RangeAnalytics` interface used by the app and factory.

- LAZY: store sum, minimum, pending addition; build in O(n), operations O(log n).
  Applying delta to length k changes sum by k*delta, min by delta, and tag by delta.
  Push before descending. Point assignment replaces the actual current reading.
- STRETCH: store total, best nonempty prefix, best nonempty suffix, best nonempty
  stretch. Build leaves with all four values equal to the reading. Implement point
  assignment, range sum, and best stretch; reject range add and range minimum.
- Preserve left-to-right merge order, particularly for partial range queries.
- Use 64-bit arithmetic and O(n) storage; avoid recursion proportional to n.
- Unit tests are in `tests/test_structA.cpp` and cover both modes.
- Q1 is written in REPORT.md with merge equations and an associativity example.

The factory selects this adapter for both modes. Do not print from methods.
