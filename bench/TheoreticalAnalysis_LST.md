# Lazy Segment Tree: Theoretical Analysis

## 1. Problem and Supported Operations

The tree stores a nonempty array of sensor readings. Indices are 1-based and both range endpoints are included. Assume valid indices, constant-time arithmetic and no overflow.

| Operation | Meaning |
|---|---|
| **U i v** | Replace the reading at index i with v |
| **A l r x** | Add x to each reading in the requested range |
| **S l r** | Return the sum over the requested range |
| **M l r** | Return the minimum over the requested range |

## 2. Lazy Propagation

**Apply.** For a fully covered node, update its sum, minimum and pending tag. Here ℓ is the length of the node's interval.

$$
\begin{aligned}
\mathrm{sum} &\gets \mathrm{sum}+x\ell \\
\mathrm{mn} &\gets \mathrm{mn}+x \\
\mathrm{lz} &\gets \mathrm{lz}+x.
\end{aligned}
$$

**Push.** Apply the parent's pending tag to both children using their respective lengths, then clear the tag. The parent's aggregates retain their values.

**Pull.** With the parent tag cleared, combine its children's aggregates.

$$
\mathrm{sum}_P=\mathrm{sum}_L+\mathrm{sum}_R,\qquad
\mathrm{mn}_P=\min(\mathrm{mn}_L,\mathrm{mn}_R).
$$

## 3. Building the Tree

Each leaf stores its reading as both sum and minimum. All tags start at zero. Internal nodes build both children before pulling. Every internal node has two children, so the number of logical nodes is

$$
N_{\mathrm{nodes}}=2n-1.
$$

For a power-of-two input size, the build recurrence is

$$
T(1)\in\Theta(1),\qquad T(n)=2T(n/2)+\Theta(1),\qquad T(n)\in\Theta(n).
$$

## 4. Range Addition

Return immediately on no overlap. Apply the addition and stop on complete overlap. On partial overlap, push the tag, recurse into both children and pull the parent. Only intervals containing a query boundary can require further splitting, so at most two nodes per depth are partially covered.

$$
h=\lceil\log_2 n\rceil,\qquad V_{\mathrm{range}}(n)\le 1+4h.
$$

A full-array update stops at the root. A singleton update descends to a leaf, giving a matching logarithmic worst-case lower bound for growing inputs.

## 5. Range Sum

Return zero on no overlap and the cached sum on complete overlap. On partial overlap, push the tag and add the results from both children. The fully covered intervals partition the requested range.

$$
S(l,r)=\sum_{i=l}^{r}a_i.
$$

## 6. Range Minimum

Return the largest representable reading on no overlap and the cached minimum on complete overlap. On partial overlap, push the tag and take the smaller child result.

$$
M(l,r)=\min_{l\le i\le r}a_i.
$$

## 7. Point Assignment

The implementation queries the current reading and then adds the difference between the target and current readings over the singleton interval. Both traversals push pending ancestor tags as needed. The difference must fit the signed integer type.

$$
\Delta=v-S(i,i),\qquad S(i,i)+\Delta=v.
$$

## 8. Correctness Argument

A node's cached aggregates incorporate its own pending tag. To recover the logical aggregates over that node's interval, also account for any tags still held by strict ancestors. If their sum is d, the logical aggregates are

$$
\mathrm{logical\ sum}=\mathrm{sum}+d\ell,\qquad
\mathrm{logical\ minimum}=\mathrm{mn}+d.
$$

The build establishes the invariant with zero tags. Applying an addition updates the covered interval. Pushing preserves the logical readings, and pulling restores the parent aggregates after a partial update. Queries push along the paths they follow, so returned aggregates include all relevant updates.

## 9. Time and Space Complexity Summary

$$
\begin{aligned}
T_{\mathrm{build}}(n)&\in\Theta(n) \\
T_{\mathrm{apply/push/pull}}&\in\Theta(1) \\
T_{\mathrm{point\ assignment}}(n)&\in\Theta(1+\log n) \\
T_{\mathrm{add/sum/min,\ worst}}(n)&\in\Theta(1+\log n) \\
T_{\mathrm{add/sum/min,\ full}}&\in\Theta(1) \\
D_{\mathrm{stack}}(n)&\le 1+\lceil\log_2 n\rceil.
\end{aligned}
$$

The benchmark counts three arrays of 64-bit values, excluding container and allocator overhead.

$$
M_{\mathrm{arrays}}(n)=3(4n+4)\cdot 8\ \mathrm{bytes}=(96n+96)\ \mathrm{bytes}.
$$

## 10. Predictions to Check in Benchmarks

- Build time should grow approximately linearly with array size.
- Point assignment makes two traversals down to a leaf, and irregular ranges can split across several levels.
- Full-array additions, sums and minima stop at the root. Caching, memory layout and compiler optimization can affect the measured differences.
