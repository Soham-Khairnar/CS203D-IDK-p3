#pragma once
// Oracle (answer key): a plain array with plain loops. It shares NO code with the
// segment tree or the block decomposition, so a bug in either cannot hide here.
#include "../src/range_analytics.hpp"
#include <algorithm>
#include <stdexcept>
#include <string>

namespace sensor {
namespace testing {

class Oracle final : public RangeAnalytics {
    std::vector<Value> a;  // zero-based storage; public API is 1-based

    static Value add(Value x, Value y) {
        Value r;
        if (__builtin_add_overflow(x, y, &r))
            throw std::overflow_error("oracle: int64 overflow (workload outside supported range)");
        return r;
    }
    void check(Index l, Index r) const {
        if (l < 1 || r < l || r > a.size())
            throw std::out_of_range("oracle: bad range " + std::to_string(l) + ".." + std::to_string(r));
    }

public:
    explicit Oracle(const std::vector<Value>& initial) : a(initial) {}

    void pointSet(Index i, Value v) override { check(i, i); a[i - 1] = v; }

    void rangeAdd(Index l, Index r, Value d) override {
        check(l, r);
        for (Index i = l; i <= r; ++i) a[i - 1] = add(a[i - 1], d);
    }

    Value rangeSum(Index l, Index r) override {
        check(l, r);
        Value s = 0;
        for (Index i = l; i <= r; ++i) s = add(s, a[i - 1]);
        return s;
    }

    Value rangeMin(Index l, Index r) override {
        check(l, r);
        Value m = a[l - 1];
        for (Index i = l + 1; i <= r; ++i) m = std::min(m, a[i - 1]);
        return m;
    }

    // Kadane. ending = best stretch that ENDS at i: either a[i] alone, or the best
    // stretch ending at i-1 extended by a[i]. Start from a[l], not 0, so that an
    // all-negative range returns its largest element (stretch must be nonempty).
    Value bestStretch(Index l, Index r) override {
        check(l, r);
        Value ending = a[l - 1], best = ending;
        for (Index i = l + 1; i <= r; ++i) {
            ending = std::max(a[i - 1], add(ending, a[i - 1]));
            best = std::max(best, ending);
        }
        return best;
    }

    const std::vector<Value>& values() const { return a; }
};

} // namespace testing
} // namespace sensor
