# Structure A handoff: Soham / Q1

Implement `SegmentTree` in the provided header/source without changing the shared
1-based inclusive interface. The constructor currently throws intentionally.

- LAZY: store sum, minimum, pending addition; build in O(n), operations O(log n).
  Applying delta to length k changes sum by k*delta, min by delta, and tag by delta.
  Push before descending. Point assignment replaces the actual current reading.
- STRETCH: store total, best nonempty prefix, best nonempty suffix, best nonempty
  stretch. Build leaves with all four values equal to the reading. Implement point
  assignment, range sum, and best stretch; reject range add and range minimum.
- Preserve left-to-right merge order, particularly for partial range queries.
- Use 64-bit arithmetic and O(n) storage; avoid recursion proportional to n.
- Add unit tests and coordinate invariant inspection with the Test owner.
- Complete Q1 in REPORT.md, including the merge equations and associativity example.

The factory already selects this class for both modes. Do not print from methods.
