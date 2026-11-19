# Block Decomposition in LAZY Mode: Theoretical Analysis

## 1. Problem and Supported Operations

The structure stores a nonempty array of sensor readings. Public indices are 1-based and inclusive. Assume valid indices, constant-time arithmetic and no overflow.

| Operation | Meaning |
|---|---|
| **U i v** | Replace the reading at index i with v |
| **A l r x** | Add x to each reading in the requested range |
| **S l r** | Return the sum over the requested range |
| **M l r** | Return the minimum over the requested range |

## 2. Block Layout and Cached Aggregates

The array of length n is split into blocks of nominal size B. The final block may be shorter. Let K be the block count, m the maximum actual block length, and I denote the indices in block b. The raw reading is a and the logical reading is x.

$$
\begin{aligned}
K&=\left\lceil\frac{n}{B}\right\rceil,\qquad m=\min(B,n) \\
x_i&=a_i+\mathrm{add}_b\qquad (i\in I_b) \\
\mathrm{sum}_b&=\sum_{i\in I_b}x_i,\qquad
\mathrm{min}_b=\min_{i\in I_b}x_i.
\end{aligned}
$$

## 3. Building the Structure

Copy the readings, initialize the three block arrays, then rebuild the aggregates by scanning each block. With the default size, an integer square-root loop runs once. Its work is dominated by copying and scanning the input.

$$
B_{\mathrm{default}}=\lfloor\sqrt n\rfloor,\qquad
T_{\mathrm{build}}(n,B)\in\Theta(n+K)=\Theta(n).
$$

## 4. Range Addition

For a fully covered block of actual length k, an addition of d changes its pending tag and cached aggregates.

$$
\mathrm{sum}_b\gets \mathrm{sum}_b+kd,\qquad
\mathrm{min}_b\gets \mathrm{min}_b+d,\qquad
\mathrm{add}_b\gets \mathrm{add}_b+d.
$$

For a partially covered block, push the pending tag into the raw readings, update the selected readings and rebuild the aggregates. At most two blocks are partial. A full-array update uses every block's cached aggregates, including those of the last block.

## 5. Range Sum

Add the cached sum of each fully covered block. On partial blocks, scan the selected readings and include each block's pending tag.

$$
S(l,r)=\sum_{i=l}^{r}x_i.
$$

## 6. Range Minimum

Use the cached minimum of each fully covered block and scan only the selected readings of partial blocks. The minimum over these contributions gives the answer.

$$
M(l,r)=\min_{l\le i\le r}x_i.
$$

## 7. Point Assignment

Find the block containing the index, push its tag, overwrite the raw reading and rebuild the block. Rebuilding always scans the actual length of that block.

## 8. Correctness Argument

Construction sets each tag to zero and computes aggregates from the readings. Pushing preserves each logical reading by transferring its tag into the raw array. Rebuilding recomputes the aggregates from the resulting readings. A full-block addition changes every reading by the same amount, preserving the formulas in Section 2. Queries combine disjoint block contributions covering the requested interval.

## 9. Time and Space Complexity Summary

With up to two partial blocks, the range work is bounded by a constant multiple of the maximum partial-block work plus the block count. A full-array range attains the block-count term. A range confined to a largest block and omitting one of its readings attains the block-length term for growing blocks.

$$
\begin{aligned}
T_{\mathrm{build}}(n,B)&\in\Theta(n) \\
T_{\mathrm{assignment,\ worst}}(n,B)&\in\Theta(m) \\
T_{\mathrm{add/sum/min,\ worst}}(n,B)&\in\Theta(m+K) \\
T_{\mathrm{add/sum/min,\ full}}(n,B)&\in\Theta(K) \\
M_{\mathrm{arrays}}(n,B)&=(8n+24K)\ \mathrm{bytes}.
\end{aligned}
$$

The storage estimate covers one 64-bit reading per element and three 64-bit values per block. The iterative operations use constant auxiliary stack space.

## 10. Predictions to Check in Benchmarks

For nominal sizes between one and the array length, the continuous cost model balances scanning partial blocks against visiting complete blocks.

$$
f(B)=B+\frac{n}{B},\qquad
f'(B)=1-\frac{n}{B^2},\qquad
f''(B)=\frac{2n}{B^3}>0,\qquad
B_{\mathrm{min}}=\sqrt n.
$$

- Larger blocks can increase point assignment costs because rebuilding scans more readings.
- Long queries can benefit from visiting fewer blocks. Short queries may inspect only a few selected readings even inside a large block.
- The measured optimum can vary with the operation mix, range lengths and the relative costs of scans and aggregate accesses.
