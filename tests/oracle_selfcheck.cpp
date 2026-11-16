#include "oracle.hpp"
#include "workload.hpp"
#include <iostream>
using namespace sensor; using namespace sensor::testing;
// Self-check of the answer key: the oracle (Kadane) must equal an O(len^2) brute force.
int main() {
    Rng rng(42); long checked = 0, allneg = 0;
    for (int trial = 0; trial < 200000; ++trial) {
        Index n = (Index)rng.uni(1, 12);
        Value lo = (trial % 3 == 0) ? -9 : -5, hi = (trial % 3 == 0) ? -1 : 5;   // every third array is all-negative
        std::vector<Value> a;
        for (Index i = 0; i < n; ++i) a.push_back(rng.uni(lo, hi));
        Oracle o(a);
        Index l = (Index)rng.uni(1, (Value)n), r = (Index)rng.uni((Value)l, (Value)n);
        Value brute = a[l - 1];
        for (Index i = l; i <= r; ++i) { Value s = 0; for (Index j = i; j <= r; ++j) { s += a[j - 1]; if (s > brute) brute = s; } }
        if (o.bestStretch(l, r) != brute) { std::cout << "MISMATCH trial " << trial << "\n"; return 1; }
        ++checked; if (hi < 0) ++allneg;
    }
    std::cout << "Kadane == brute force O(len^2) on " << checked << " random ranges (" << allneg << " all-negative): OK\n";
}
