
#include "block_decomposition.hpp"
#include <algorithm>
#include <cstdio>
#include <random>
#include <stdexcept>

using namespace sensor;

static int fails = 0;
#define CHECK(cond, ...) do { if (!(cond)) { std::printf(__VA_ARGS__); std::printf("\n"); ++fails; return; } } while (0)

static void run(unsigned seed, Index n, Index blockSize, int ops, Value lo, Value hi) {
    std::mt19937_64 rng(seed);
    auto val = [&] { return std::uniform_int_distribution<Value>(lo, hi)(rng); };
    std::vector<Value> ref(n);
    for (auto& x : ref) x = val();
    BlockDecomposition bd(ref, blockSize);
    for (int t = 0; t < ops; ++t) {
        Index l = std::uniform_int_distribution<Index>(1, n)(rng);
        Index r = std::uniform_int_distribution<Index>(1, n)(rng);
        if (l > r) std::swap(l, r);
        switch (rng() % 4) {
        case 0: { Value v = val(); bd.pointSet(l, v); ref[l - 1] = v; break; }
        case 1: { Value d = val(); bd.rangeAdd(l, r, d); for (Index i = l; i <= r; ++i) ref[i - 1] += d; break; }
        case 2: { Value s = 0; for (Index i = l; i <= r; ++i) s += ref[i - 1];
                  CHECK(bd.rangeSum(l, r) == s, "SUM seed=%u n=%u B=%u op=%d [%u,%u]", seed,
                        static_cast<unsigned>(n), static_cast<unsigned>(blockSize), t,
                        static_cast<unsigned>(l), static_cast<unsigned>(r)); break; }
        case 3: { Value m = *std::min_element(ref.begin() + (l - 1), ref.begin() + r);
                  CHECK(bd.rangeMin(l, r) == m, "MIN seed=%u n=%u B=%u op=%d [%u,%u]", seed,
                        static_cast<unsigned>(n), static_cast<unsigned>(blockSize), t,
                        static_cast<unsigned>(l), static_cast<unsigned>(r)); break; }
        }
        CHECK(bd.checkInvariants(), "INVARIANT seed=%u n=%u op=%d", seed,
              static_cast<unsigned>(n), t);
    }
}

int main() {
    
    const Index ns[] = {1, 2, 3, 5, 7, 10, 17, 100, 1000};
    const Index bs[] = {0, 1, 2, 3, 7, 50, 5000};
    unsigned seed = 1;
    for (Index n : ns) for (Index b : bs) {
        run(seed++, n, b, 3000, -100, 100);
        run(seed++, n, b, 1000, -1000000000LL, 1000000000LL);
    }
    run(777, 100000, 0, 100000, -1000, 1000);   

    BlockDecomposition bd({1, 2, 3});
    try { bd.bestStretch(1, 3); std::printf("bestStretch did not throw\n"); ++fails; }
    catch (const std::logic_error&) {}
    std::printf(fails ? "FAILED (%d)\n" : "All block decomposition tests passed.\n", fails);
    return fails ? 1 : 0;
}
