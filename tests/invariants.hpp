#pragma once
#include "../src/inspectable.hpp"
#include "../src/range_analytics.hpp"
#include "workload.hpp"
#include <algorithm>
#include <string>

namespace sensor {
namespace testing {

inline std::string whiteBoxCheck(const RangeAnalytics& backend, bool* wasInspectable = nullptr) {
    const Inspectable* insp = dynamic_cast<const Inspectable*>(&backend);
    if (wasInspectable) *wasInspectable = (insp != nullptr);
    return insp ? insp->selfCheck() : std::string();
}

inline std::string rangeText(Index l, Index r) {
    return "[" + std::to_string(l) + ".." + std::to_string(r) + "]";
}

inline std::string blackBoxCheck(RangeAnalytics& b, Mode mode, const std::vector<Value>& truth,
                                 Rng& rng, int samples = 40) {
    const Index n = truth.size();

    auto checkPoint = [&](Index i) -> std::string {
        Value v = truth[i - 1];
        if (b.rangeSum(i, i) != v) return "point " + std::to_string(i) + ": sum " + std::to_string(b.rangeSum(i, i)) + " != " + std::to_string(v);
        if (mode == Mode::Lazy && b.rangeMin(i, i) != v) return "point " + std::to_string(i) + ": min != value";
        if (mode == Mode::Stretch && b.bestStretch(i, i) != v) return "point " + std::to_string(i) + ": best stretch of one element != value";
        return "";
    };
    if (n <= 300) {
        for (Index i = 1; i <= n; ++i) { std::string e = checkPoint(i); if (!e.empty()) return e; }
    } else {
        std::string e = checkPoint(1); if (!e.empty()) return e;
        e = checkPoint(n); if (!e.empty()) return e;
        for (int s = 0; s < 150; ++s) {
            e = checkPoint(static_cast<Index>(rng.uni(1, static_cast<Value>(n))));
            if (!e.empty()) return e;
        }
    }

    for (int s = 0; n >= 2 && s < samples; ++s) {
        Index l = static_cast<Index>(rng.uni(1, static_cast<Value>(n - 1)));
        Index r = static_cast<Index>(rng.uni(static_cast<Value>(l + 1), static_cast<Value>(n)));
        Index m = static_cast<Index>(rng.uni(static_cast<Value>(l), static_cast<Value>(r - 1)));
        Value whole = b.rangeSum(l, r), left = b.rangeSum(l, m), right = b.rangeSum(m + 1, r);
        if (whole != left + right)
            return "sum" + rangeText(l, r) + " != sum" + rangeText(l, m) + " + sum" + rangeText(m + 1, r);
        if (mode == Mode::Lazy) {
            Value mw = b.rangeMin(l, r), ml = b.rangeMin(l, m), mr = b.rangeMin(m + 1, r);
            if (mw != std::min(ml, mr))
                return "min" + rangeText(l, r) + " != min(min" + rangeText(l, m) + ", min" + rangeText(m + 1, r) + ")";
        } else {
            Value bw = b.bestStretch(l, r), bl = b.bestStretch(l, m), br = b.bestStretch(m + 1, r);
            if (bw < std::max(bl, br))
                return "best" + rangeText(l, r) + " < best of one of its halves";
            if (bw < whole)
                return "best" + rangeText(l, r) + " < sum of the whole range (the whole range is a stretch)";
        }
    }
    return "";
}

} // namespace testing
} // namespace sensor
