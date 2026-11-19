# Segment Tree in STRETCH Mode: Theoretical Analysis

## 1. Problem and Supported Operations

The tree stores a nonempty array of sensor readings. Indices are 1-based and inclusive. Assume valid indices, constant-time arithmetic and no overflow.

| Operation | Meaning |
|---|---|
| **U i v** | Replace the reading at index i with v |
| **S l r** | Return the sum over the requested range |
| **B l r** | Return the largest sum of a nonempty contiguous subarray within the requested range |

## 2. Node Summary and Merge

Each node stores the total of its interval, the largest nonempty prefix and suffix sums, and the largest nonempty subarray sum. For adjacent children L and R, the parent P is computed in left-to-right order.

$$
\begin{aligned}
\mathrm{total}_P&=\mathrm{total}_L+\mathrm{total}_R \\
\mathrm{pref}_P&=\max(\mathrm{pref}_L,\mathrm{total}_L+\mathrm{pref}_R) \\
\mathrm{suf}_P&=\max(\mathrm{suf}_R,\mathrm{suf}_L+\mathrm{total}_R) \\
\mathrm{best}_P&=\max(\mathrm{best}_L,\mathrm{best}_R,\mathrm{suf}_L+\mathrm{pref}_R).
\end{aligned}
$$

## 3. Building the Tree

Set all four leaf fields to the reading at that leaf. Recursively build both children and merge them. The tree has exactly two children at every internal node. For a power-of-two input size, with constant merge work c, the build work is

$$
N_{\mathrm{nodes}}=2n-1,\qquad
T(n)=2T(n/2)+c=nT(1)+c(n-1)\quad(n>1).
$$

## 4. Point Assignment

Follow the unique path to the leaf, replace its four fields, then recompute the ancestors in order. The maximum path length, counted in visited nodes, is

$$
V_{\mathrm{assign}}(n)\le 1+\lceil\log_2 n\rceil.
$$

## 5. Range Sum

A fully covered node returns its stored summary. If the request intersects only one child, visit that child alone. When it intersects both, merge the returned summaries in array order. Read the total field of the resulting summary.

$$
S(l,r)=\sum_{i=l}^{r} a_i.
$$

## 6. Best-Stretch Query

Use the same traversal and read the best field. For example, for the three readings below, the best nonempty stretch is the middle reading.

$$
(a_1,a_2,a_3)=(-8,-3,-5),\qquad
B(1,3)=-3,\qquad
B(l,r)=\max_{l\le p\le q\le r}\sum_{i=p}^{q}a_i.
$$

## 7. Why Range Addition Needs More Information

If an entire interval receives an addition of x, a candidate stretch of length k has the following new sum.

$$
s_{\mathrm{new}}=s_{\mathrm{old}}+kx.
$$

Candidates of different lengths gain different amounts. The four cached fields do not retain the lengths and sums of alternative candidates needed to determine the new best stretch after an arbitrary addition.

## 8. Correctness Argument

The prefix stays in the left child or includes all of it and extends into the right child. The suffix has the corresponding two cases. The best stretch stays inside one child or crosses the boundary as a left suffix followed by a right prefix. These cases establish the merge formulas. Merging is associative for adjacent summaries because either parenthesization summarizes the same concatenation in the same order. Building, point assignment and querying each maintain that order.

## 9. Time and Space Complexity Summary

Only partially covered nodes require splitting. At most two nodes per depth are partial, and each visits at most two children. With h denoting the maximum depth measured in edges,

$$
\begin{aligned}
h&=\lceil\log_2 n\rceil \\
V_{\mathrm{query}}(n)&\le 1+4h \\
T_{\mathrm{build}}(n)&\in\Theta(n) \\
T_{\mathrm{merge}}&\in\Theta(1) \\
T_{\mathrm{assignment/query,\ worst}}(n)&\in\Theta(1+\log n) \\
T_{\mathrm{query,\ full}}&\in\Theta(1) \\
D_{\mathrm{stack}}(n)&\le 1+h.
\end{aligned}
$$

The benchmark counts the slots allocated for the node array. When a node occupies 32 bytes, as on the benchmark platform, the estimate is

$$
M_{\mathrm{array}}(n)=(4n+4)\cdot32\ \mathrm{bytes}=(128n+128)\ \mathrm{bytes}.
$$

## 10. Predictions to Check in Benchmarks

- Build time should grow approximately linearly with array size.
- Point assignments follow one root-to-leaf path. Irregular queries may visit several paths and perform merges.
- Full-array sum and best-stretch queries return the root summary immediately.
