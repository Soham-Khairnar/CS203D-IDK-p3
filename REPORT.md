# Report: Range Analytics on Sensor Streams

## 1. Team and contributions

See the roles table in [README.md](README.md). Add links to each member's main
commits after they exist.

- Abhishek Verma: shared interface, factory, CLI, input validation and dispatch,
  output/error handling, build/run scripts, application tests, examples and Q3.
  Integration with completed production backends is pending.
- Soham: TODO - Structure A implementation, unit tests and Q1.
- Smita: TODO - Structure B implementation, unit tests and Q2.
- Test owner: Shrawan Kumar Prajapati (250008033, shrawanprajapati) - plain-array oracle with Kadane, seeded workload generator, differential test, invariant checker, worst-case input generator and cost model (`tests/`), Q4.
- Measurement owner: TODO - name, benchmarks, data, figures and Q5.

## 2. Problem

Up to one million sensor readings receive point corrections and range calibration
offsets. LAZY mode supports point assignment, range addition, range sum and minimum,
using either a segment tree or block decomposition. STRETCH mode supports point
assignment, sum and a maximum nonempty contiguous subarray sum, using Structure A
alone. Range addition is excluded from STRETCH mode because adding a constant can
change which stretch wins depending on its length.

The C++ CLI reads operations through EOF and prints one answer per query. A shared
`RangeAnalytics` interface and factory separate parsing/output from the algorithms.
Public indices are 1-based and inclusive; values are signed 64-bit integers. The
application checks syntax, indices, and supported mode/structure combinations.
Errors go to stderr with a nonzero exit code. Backends return results without printing.

## 3. Structure A: Segment tree

TODO (Soham): storage, lazy propagation, four-value merge, operations/complexity,
design choice and diagram. Implement src/structA/segment_tree.cpp and unit tests.

## 4. Structure B: Block decomposition

TODO (Smita): storage, pending adds, partial/full blocks, operations/complexity,
block-size choice and diagram. Explain the LAZY-only restriction. Implement
src/structB/block_decomposition.cpp and unit tests.

## 5. Testing

**Oracle.** `tests/oracle.hpp` keeps a plain array. Range add, sum and minimum use loops, and best stretch uses Kadane's algorithm initialised with the first element so an all-negative range returns its largest value.

**Differential test.** `tests/differential_test.cpp` replays the same seeded random operations on the oracle, Structure A and Structure B (LAZY: A, B and oracle; STRETCH: A and oracle) and compares every query answer. Base seed 203003; each configuration uses base seed + its index. The main configurations run 100,000 operations each over seven profiles (mixed, short, long, edges, lazy_then_set, allneg, big), including full and single-element ranges, negative values, point sets after range adds, queries crossing block boundaries and all-negative stretches. Total operations compared: [N]. Result: [ALL PASSED / describe failures]. On a mismatch the test prints the seed, configuration, operation number, expected and actual values, and writes a replayable repro file.

**Invariant checker.** Every 500 operations: (1) black-box checks on any backend: every position reads back its true value, range sums are additive over splits, a range minimum equals the minimum of its two halves, and best stretch is at least the largest element and at most the range maximum-sum bound; (2) white-box checks through `Inspectable::selfCheck()`: stored node sums and minima equal the true values, pending tags are consistent, four-value merges are correct in STRETCH mode, and block sums, minima and pending adds are consistent. Invariant checks run: [N].

**Bugs found.** [list each bug: what failed, which structure, which seed/operation, how it was fixed; or write "none found"].

## 6. Measurements

TODO (Measurement owner): machine/compiler, sizes 10^4 to 10^6, short/long ranges,
block-size sweep, memory, at least five runs, seeds, CSV and labelled figures.
Follow bench/README.md. No timing data has been collected.

## 7. Conclusion

TODO (team): choose a structure using measured results and discuss STRETCH support.

## 8. Answers to Q1–Q5

### Q1 (author: Khairnar Soham Nilesh)

TODO: derive four-value merge equations and demonstrate associativity on an example.

### Q2 (author: Smita Singh)

TODO: block pending-add correctness and the sqrt(n) block-size bound.

### Q3 (author: Abhishek Verma)

**Why does a segment-tree range query visit O(log n) nodes, at most four per level?**

A balanced segment tree repeatedly splits an interval into floor/ceiling halves,
giving height ceil(log2(n)). Height alone is not enough to establish the query
bound: the entire tree still contains O(n) nodes. The important fact is that only
partially overlapping intervals require further descent.

For query [l,r], each visited node has one of three outcomes:

1. Disjoint interval: stop and return a neutral result.
2. Fully covered interval: return its stored summary immediately.
3. Partial overlap: descend into the children and combine their results.

At any depth, node intervals are disjoint and ordered. A contiguous query can
partially overlap at most two: the intervals containing its left and right
boundaries. All intervals strictly between these boundaries are fully covered;
outside intervals are disjoint. Only the at-most-two partial intervals recurse.
Each has at most two children, giving at most four visited nodes on the next level,
including disjoint child calls that return immediately. The root has one visit.

For height h this gives at most 1 + 4h visits, hence O(log n), or O(1 + log n)
including n = 1. Skipping disjoint child calls can reduce the actual visit count.

Example: n = 16, querying [3,14].

| Depth | Visited intervals | Continue descending from |
|---|---|---|
| 0 | [1,16] | [1,16] |
| 1 | [1,8], [9,16] | Both |
| 2 | [1,4], [5,8], [9,12], [13,16] | [1,4], [13,16] |
| 3 | [1,2], [3,4], [13,14], [15,16] | None |

[5,8] and [9,12] return stored summaries at depth 2. [3,4] and [13,14] return
summaries at depth 3; [1,2] and [15,16] are disjoint. This is 11 visits rather
than traversing all 31 nodes.

Each visit must also cost O(1). Sum/minimum merge in constant time, as does merging
the four STRETCH values in left-to-right order. In LAZY mode, pushing a pending
addition changes a fixed number of values in two children, also O(1). Thus these
range queries take O(log n) time. The argument requires returning a stored summary
on full coverage instead of traversing every reading in that interval.

### Q4 (author: Shrawan Kumar Prajapati)

**What a pending value means.** In lazy mode a node `v` covering `len` readings stores `sum`, `min` and a pending value `t`. The stored `sum` and `min` of `v` already include every add applied to `v`'s whole segment. The pending `t` is the amount that still has to be added to each reading in the two children, which have not been updated yet.

**Applying an add to a fully covered node.** Adding `x` to every reading of a segment of length `len` increases its sum by `x * len` and its minimum by `x`. So `sum += x * len`, `min += x`, `t += x`, in O(1) without visiting the children.

**Why two pending adds combine into one.** Adding `t1` and then `t2` to a reading `a` gives `(a + t1) + t2 = a + (t1 + t2)`. Integer addition is associative, so the order and grouping do not matter and one stored value `t1 + t2` is equivalent to the two adds. Pushing down moves `t` to the children: `child.sum += t * child.len`, `child.min += t`, `child.t += t`, then `t = 0`. Each child's stored values stay correct for its own segment, and the parent's values were already correct, so nothing is double counted.

**Why stored sum and minimum stay correct.** Invariant: for every node, `sum` and `min` are correct for its segment with all updates applied at the node or below, and only the tags of its ancestors are missing. A query or update pushes the tags down along its path before using the children, so every value it reads satisfies the invariant. After the recursive calls the node is recomputed as `sum = left.sum + right.sum` and `min = min(left.min, right.min)`. This restores the invariant, and with it the correctness of the sum and minimum answers.

**Why Kadane's algorithm is a correct oracle for best stretch.** Let `e[i]` be the largest sum of a non-empty stretch ending at `i`. Such a stretch is either `a[i]` alone or a stretch ending at `i-1` extended by `a[i]`; replacing the part ending at `i-1` by the best one can only help, so `e[i] = max(a[i], e[i-1] + a[i])` with `e[l] = a[l]`. Every non-empty stretch in `[l..r]` ends at some `i`, so the answer is `max e[i]`. Starting from `a[l]` instead of 0 guarantees at least one reading is chosen, so an all-negative range returns its largest value, as the specification requires. The oracle is also independent of the segment tree: it uses only a plain array and loops.

### Q5 (author: [name])

TODO: actual times versus O(log n)/O(sqrt(n)); explain size and range-length effects.

## 9. References

- [Course project specification](CS203-D_Mini_Projects_4-1.pdf), Project 3 and common requirements.
- OpenAI Codex: assistance declared in [TOOLS.md](TOOLS.md). Review all assisted code
  and explanations before submission and viva.
- TODO: each owner should add the algorithm sources actually consulted.
