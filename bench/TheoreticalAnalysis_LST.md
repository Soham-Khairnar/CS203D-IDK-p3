# Lazy Segment Tree: Theoretical Analysis

## 1. Problem and Supported Operations

Given an array `a[1..n]` of sensor readings, support these operations using **1-based, inclusive indices**:

| Operation | Meaning |
|---|---|
| `U i v` | Set `a[i] = v` |
| `A l r x` | Add `x` to every element in `[l,r]` |
| `S l r` | Return the sum of elements in `[l,r]` |
| `M l r` | Return the minimum element in `[l,r]` |

Assume valid indices, constant-time arithmetic and no integer overflow. This chapter covers **LAZY mode only**.

## 2. Lazy Propagation: Apply, Push and Pull

**Apply an addition `x` to a fully covered node of length `len`:**

```text
sum  += x * len
mn   += x
lazy += x
```

This works because adding `x` to `len` elements increases their sum by `x*len` and their minimum by `x`. Pending additions combine by addition.

**Push:** Before descending into children, apply the parent's pending `lazy` value to both children, then clear the parent's tag. Its own aggregates are already correct and do not change.

**Pull:** After changing a child, recompute the parent's `sum` and `mn` from its children. Push a pending parent tag before pulling.

Each primitive takes **$\Theta$(1)** time.

## 3. Building the Tree — $\Theta$(n)

At a leaf, set `sum = mn = a[i]` and `lazy = 0`. At an internal node, build both children and then pull their aggregates.

For `n` a power of two:

$$t(n) = 2t(n/2) + \Theta(1) = \Theta(n)$$


For arbitrary `n`, the tree still contains exactly `2n-1` logical nodes, each requiring constant work. Thus **build time is Theta(n)**.

## 4. Range Addition `A l r x` — O(log n)

For a node covering `[L,R]`:

1. **No overlap:** Return without changes.
2. **Complete overlap:** Apply `x` to the node and stop; do not visit its children.
3. **Partial overlap:** Push the pending tag, update intersecting children, then pull the parent.

**Complexity:** At any depth, only nodes crossing the left or right boundary of `[l,r]` need further splitting. There are at most two such partially covered nodes, producing only a constant number of visited nodes per level. The tree has `O(log n)` levels.

Therefore the **worst-case time is $\Theta$(log n)**. Updating the full array `[1,n]` stops at the root and takes **Theta(1)**.

## 5. Range Sum `S l r` — O(log n)

For each visited node:

1. **No overlap:** Return `0`.
2. **Complete overlap:** Return the stored `sum`.
3. **Partial overlap:** Push, query the relevant children and add their results.

The fully covered intervals are disjoint and together form the requested range, so their sums give the correct answer.

The same boundary argument gives **$\Theta$(log n) worst-case time**. A full-array query returns the root sum in **$\Theta$(1)**.

## 6. Range Minimum `M l r` — O(log n)

The traversal is the same as for range sum, except:

1. **No overlap:** Return `+infinity` (a suitable neutral value).
2. **Complete overlap:** Return the stored `mn`.
3. **Partial overlap:** Push and return the minimum of the child results.

The minimum over a union of disjoint intervals is the minimum of their individual minima.

Time is **$\Theta$(log n) in the worst case** and **$\Theta$(1)** for a full-array query.

## 7. Point Assignment `U i v` — $\Theta$(log n)

Descend from the root to the leaf for index `i`, pushing pending additions at every internal node. Set the leaf's `sum = mn = v` and clear its leaf tag. Pull the aggregates on the way back.

**Why push first?** Point assignment *replaces* the value. If an older pending addition were applied after the assignment, the new value would be incorrect.

Exactly one child is visited at each level:


$$T(n)=T(\lceil n/2\rceil)+\Theta(1)=\Theta(\log n)\quad(n>1).$$


## 8. Correctness Argument

The invariant is: **every node's `sum` and `mn` describe the current logical array over its interval; `lazy` records only updates not yet forwarded to descendants.**

- **After build:** Leaves are correct, and each parent combines correct child aggregates.
- **After an update:** `apply` modifies a fully covered node exactly; `push` transfers pending changes; `pull` restores parent aggregates after partial updates.
- **After point assignment:** Previous additions are pushed before the leaf is overwritten.
- **During queries:** The requested interval is combined from disjoint, correct node aggregates.

By induction over the operations, all query results are correct.

## 9. Time and Space Complexity Summary

| Operation | Worst-case time | Important special case |
|---|---|---|
| Build | $\Theta$(n) | — |
| Apply / push / pull | $\Theta$(1) | — |
| Point assignment | $\Theta$(log n) | — |
| Range addition | $\Theta$(log n) | Full-array update: $\Theta$(1) |
| Range sum | $\Theta$(log n) | Full-array query: $\Theta$(1) |
| Range minimum | $\Theta$(log n) | Full-array query: $\Theta$(1) |
| Tree storage | $\Theta$(n) | `2n-1` logical nodes; often `4n` slots allocated |
| Recursive call stack | O(log n) | At most tree height |

## 11. Predictions to Check in Benchmarks

- Build time should grow approximately linearly with `n`.
- Point assignments should exhibit logarithmic scaling.
- Whole-array additions, sums and minima should be cheaper than many irregular interval operations because they stop at the root.
- Actual timings may differ from simple complexity curves due to memory access, recursion, caching and compiler optimization.

These are **theoretical predictions, not measured results**. Benchmarks will test them separately.
