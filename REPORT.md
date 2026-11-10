# Report: Range Analytics on Sensor Streams

## 1. Team and contributions

See the roles table in [README.md](README.md). Add links to each member's main
commits after they exist.

- Abhishek Verma: shared interface, factory, CLI, input validation and dispatch,
  output/error handling, build/run scripts, application tests, examples and Q3.
  Integration with completed production backends is pending.
- Soham: TODO - Structure A implementation, unit tests and Q1.
- Smita: TODO - Structure B implementation, unit tests and Q2.
- Test owner: TODO - name, oracle, differential tests, invariants, worst-case inputs, Q4.
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

`python tests/run_tests.py` builds the CLI and application tests. Tests inject a
test-only array backend to verify both LAZY routes, STRETCH, assignment after range
addition, negative stretches, 64-bit values, input errors, CRLF/EOF handling and
parsing n = 1,000,000. CLI tests verify help, arguments and unsupported B/STRETCH.
These passed with MinGW GCC 6.3.0 on Windows. They do not verify either production
data structure, which remains an explicit scaffold.

TODO (Test owner): at least 100,000 fixed-seed random operations as specified,
oracle, invariant checks, seeds/counts/results and bugs found. Follow tests/README.md.
Add sample-file integration tests when production structures are implemented.

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

### Q4 (author: [name])

TODO: lazy-tag composition, sum/minimum correctness and Kadane oracle correctness.

### Q5 (author: [name])

TODO: actual times versus O(log n)/O(sqrt(n)); explain size and range-length effects.

## 9. References

- [Course project specification](CS203-D_Mini_Projects_4-1.pdf), Project 3 and common requirements.
- OpenAI Codex: assistance declared in [TOOLS.md](TOOLS.md). Review all assisted code
  and explanations before submission and viva.
- TODO: each owner should add the algorithm sources actually consulted.
