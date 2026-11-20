# Structure A: segment tree (Soham / Q1)

Structure A is a segment tree in two modes. LAZY mode supports point assignment, range add, range sum and range minimum with lazy propagation. STRETCH mode supports point assignment, range sum and best-stretch queries using four values per node. All indices are 1-based and inclusive. Every operation takes O(log n) time and the build takes O(n).

## Files

| File | Purpose |
|---|---|
| `lazy_segtree.hpp` | `LazySegTree`: the LAZY-mode tree |
| `stretch_segtree.hpp` | `StretchSegTree`: the STRETCH-mode tree |
| `segment_tree.hpp`, `segment_tree.cpp` | `sensor::SegmentTree`: adapter that implements the shared `RangeAnalytics` interface and `Inspectable` |
| `tests/test_structA.cpp` | Unit test |

The factory in `src/backend_factory.cpp` creates `SegmentTree(mode, initial)` for Structure A in both modes.

## LAZY mode

Each node stores three values, in arrays of size 4n + 4:

- `sum`: sum of the node's segment, including the node's own pending add
- `mn`: minimum of the node's segment, including the node's own pending add
- `lz`: pending add that has not yet been passed to the node's children

Applying an add of `x` to a node of length `len` does `sum += x * len`, `mn += x` and `lz += x`. `push` passes a node's pending add to its two children and resets it to zero. It is called before descending into a partially covered node, so the children are never stale. After updating the children, `pull` recomputes `sum` and `mn` from them.

Point assignment is implemented as a range add of `(v - current value)` on position `i`, so a previous pending add can never leak into the new value.

## STRETCH mode

Each node stores four values:

- `total`: sum of the segment
- `pref`: best sum of a non-empty prefix
- `suf`: best sum of a non-empty suffix
- `best`: best sum of any non-empty contiguous stretch

A leaf stores its reading in all four. A parent is built from its left child `a` and right child `b` with `combine(a, b)`:

```text
total = a.total + b.total
pref  = max(a.pref, a.total + b.pref)
suf   = max(b.suf,  b.total + a.suf)
best  = max(a.best, b.best, a.suf + b.pref)
```

A range query combines the covered nodes from left to right, because `combine` is associative but not commutative. An all-negative range returns its largest value, since every stretch must be non-empty.

## Complexity and storage

| Operation | Time |
|---|---|
| build | O(n) |
| point assignment, range add, range sum, range min, best stretch | O(log n) |

Storage is O(n): about 96 bytes per reading in LAZY mode (three arrays of `long long`) and about 128 bytes per reading in STRETCH mode (four `long long` per node). All arithmetic uses 64-bit integers. Recursion depth is O(log n).

## Adapter and invariant check

`SegmentTree` holds either a `LazySegTree` or a `StretchSegTree`.

- Ranges are validated and `std::out_of_range` is thrown for a bad range.
- Unsupported operations throw `std::logic_error`: range add and range minimum in STRETCH mode, and best stretch in LAZY mode.
- `selfCheck()` returns an empty string when the tree is consistent, or a description of the first violation. In LAZY mode it checks that each internal node's `sum` and `mn` equal its children's values plus its own pending add. In STRETCH mode it checks that each internal node equals `combine` of its children and that each leaf has equal values.

## Tests

Unit test, run from the repository root:

```text
g++ -std=c++17 -O2 -I src/structA src/structA/tests/test_structA.cpp -o build/test_structA
build/test_structA
```

It compares both trees with plain loops and Kadane's algorithm on random operations (fixed seed 12345), and checks the hand-worked examples. The expected final line is:

```text
PASS: 233460 random queries + hand examples (seed 12345)
```

Structure A is also covered by the differential and invariant tests in `tests/` (`python tests/run_tests.py`).

## Limitations

- No overflow handling. Inputs must keep values, sums and results within signed 64-bit range.
- Positions are cast to `int` internally, which is safe for the project limit of n ≤ 1,000,000.
- STRETCH mode does not support range add or range minimum, by design.
