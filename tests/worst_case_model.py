"""Cost models used to FIND and JUSTIFY worst-case ranges (standard library only).

They count work analytically, independent of any teammate's code:

  tree_visits(n, l, r)   node calls made by the standard recursive segment-tree query on
                         [1..n] with mid = (lo+hi)//2. A disjoint node and a fully covered
                         node each cost one call (they return immediately).

  block_cost(n, B, l, r) work of a block-decomposition query as implemented in
                         src/structB/block_decomposition.cpp: the two END blocks are always
                         scanned element by element (even when the range is block-aligned),
                         every block strictly between them costs one step. A range inside a
                         single block costs its length.

All indices are 1-based and inclusive.

Run `python tests/worst_case_model.py` to reproduce the brute-force evidence.
"""
import math


def tree_visits(n, l, r):
    def go(lo, hi):
        if r < lo or hi < l:
            return 1
        if l <= lo and hi <= r:
            return 1
        mid = (lo + hi) // 2
        return 1 + go(lo, mid) + go(mid + 1, hi)
    return go(1, n)


def block_cost(n, B, l, r):
    l0, r0 = l - 1, r - 1
    bl, br = l0 // B, r0 // B
    if bl == br:
        return r0 - l0 + 1
    left = (bl + 1) * B - l0            # elements l0 .. end of block bl
    right = r0 - br * B + 1             # elements start of block br .. r0
    return left + right + (br - bl - 1)  # + one step per whole block in between


def default_block_size(n):
    # Structure B allocates B = floor(sqrt(max(n, 4))).
    return max(1, math.isqrt(max(n, 4)))


def tree_worst_value(n):
    """Maximum tree_visits over all ranges: 4*ceil(log2 n) - 1, except 2 less when
    n = 2^k + 1. Verified by brute force for every n in 2..300 (see __main__)."""
    if n < 2:
        return 1
    k = math.ceil(math.log2(n))
    v = 4 * k - 1
    if n > 2 and (n - 1) & (n - 2) == 0:   # n - 1 is a power of two
        v -= 2
    return v


def brute_force_worst(n, cost):
    best, arg = -1, []
    for l in range(1, n + 1):
        for r in range(l, n + 1):
            c = cost(l, r)
            if c > best:
                best, arg = c, [(l, r)]
            elif c == best:
                arg.append((l, r))
    return best, arg


def tree_worst_pool(n, rng, want=64, max_tries=200000):
    """Ranges whose tree_visits equals the proven-by-brute-force maximum. Sampled from
    l in the left half, r in the right half (where every worst range lives)."""
    target = tree_worst_value(n)
    pool, best, best_rng = [], -1, (2, max(2, n - 1))
    if n < 4:
        return [(1, n)]
    for _ in range(max_tries):
        l = rng.randint(1, n // 2)
        r = rng.randint(n // 2 + 1, n)
        c = tree_visits(n, l, r)
        if c > best:
            best, best_rng = c, (l, r)
        if c == target:
            pool.append((l, r))
            if len(pool) >= want:
                break
    return pool or [best_rng]


def block_worst_pool(n, B):
    """Both end blocks fully scanned + every whole block between: l in {1,2}, r at the end
    of the last full block (or one before). All four cost within 2 of the maximum."""
    full = n // B
    if full < 2:
        return [(1, n)]
    end = full * B
    pool = [(1, end), (2, end), (1, end - 1), (2, end - 1)]
    return [p for p in pool if p[0] <= p[1]]


if __name__ == "__main__":
    print("Brute force over EVERY range 1<=l<=r<=n\n")
    ok_formula = True
    for n in range(2, 301):
        m, _ = brute_force_worst(n, lambda l, r: tree_visits(n, l, r))
        if m != tree_worst_value(n):
            ok_formula = False
            print(f"  formula mismatch at n={n}: brute force {m}, formula {tree_worst_value(n)}")
    print(f"tree: max visits == 4*ceil(log2 n)-1 (-2 when n=2^k+1) for all n in 2..300: {ok_formula}\n")

    print(f"{'n':>5} {'B':>3} | tree max  formula | block max  pool-best  argmax-of-brute-force")
    for n in [16, 31, 64, 100, 128, 256, 400]:
        B = default_block_size(n)
        tm, _ = brute_force_worst(n, lambda l, r: tree_visits(n, l, r))
        bm, barg = brute_force_worst(n, lambda l, r: block_cost(n, B, l, r))
        pb = max(block_cost(n, B, l, r) for l, r in block_worst_pool(n, B))
        print(f"{n:>5} {B:>3} | {tm:>8} {tree_worst_value(n):>7} | {bm:>9} {pb:>10}  {barg[:2]}")
    print("\nblock worst case for n = B^2 is 3B-2 = 2B element touches + (B-2) whole blocks")
