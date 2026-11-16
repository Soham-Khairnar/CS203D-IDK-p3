#pragma once
#include "../src/range_analytics.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <ostream>
#include <random>
#include <string>

namespace sensor {
namespace testing {

struct Op {
    char type;    // U, A, S, M, B
    Index a, b;   // U: a=index; A/S/M/B: a=l, b=r
    Value x;      // U: new value; A: delta
};

enum class Profile { Mixed, Short, Long, Edges, LazyThenSet, AllNegative, BigValues };

inline const char* profileName(Profile p) {
    switch (p) {
        case Profile::Mixed: return "mixed";
        case Profile::Short: return "short";
        case Profile::Long: return "long";
        case Profile::Edges: return "edges";
        case Profile::LazyThenSet: return "lazy_then_set";
        case Profile::AllNegative: return "allneg";
        case Profile::BigValues: return "big";
    }
    return "?";
}

struct Workload {
    Mode mode;
    std::vector<Value> init;
    std::vector<Op> ops;
};

class Rng {
    std::mt19937_64 g;
public:
    explicit Rng(std::uint64_t seed) : g(seed) {}
    Value uni(Value lo, Value hi) {
        return lo + static_cast<Value>(g() % static_cast<std::uint64_t>(hi - lo + 1));
    }
};

inline Workload makeWorkload(Mode mode, Index n, std::size_t numOps, Profile profile,
                             std::uint64_t seed) {
    Rng rng(seed);
    Workload w;
    w.mode = mode;

    // Value bounds. In BigValues the add size is capped so that every element and every
    // range sum stays far below 2^63 (n * (initial + numOps * addMax) <= ~1e18).
    Value valMax = 50, addMax = 20;
    if (profile == Profile::BigValues) {
        valMax = 1000000000LL;
        double cap = 1e18 / (static_cast<double>(n) * (static_cast<double>(numOps) + 1.0));
        addMax = static_cast<Value>(std::min(1e9, std::max(1.0, cap)));
    }
    const bool neg = (profile == Profile::AllNegative);

    for (Index i = 0; i < n; ++i)
        w.init.push_back(neg ? rng.uni(-valMax, -1) : rng.uni(-valMax, valMax));

    std::vector<Index> cand;
    Index B = std::max<Index>(1, static_cast<Index>(std::sqrt(static_cast<double>(n))));
    const Index fixed[] = {1, 2, 3, n >= 2 ? n - 1 : 1, n >= 3 ? n - 2 : 1, n};
    for (Index c : fixed) cand.push_back(c);
    for (Index m = B; m <= n; m += B) { cand.push_back(m); cand.push_back(m + 1); cand.push_back(m - 1); }
    cand.erase(std::remove_if(cand.begin(), cand.end(),
                              [n](Index c) { return c < 1 || c > n; }), cand.end());

    auto pickRange = [&](Index& l, Index& r) {
        switch (profile) {
            case Profile::Short: {
                Index len = static_cast<Index>(rng.uni(1, static_cast<Value>(std::min<Index>(n, 3))));
                l = static_cast<Index>(rng.uni(1, static_cast<Value>(n - len + 1)));
                r = l + len - 1;
                return;
            }
            case Profile::Long: {
                Index lo = std::max<Index>(1, n - n / 10);
                Index len = static_cast<Index>(rng.uni(static_cast<Value>(lo), static_cast<Value>(n)));
                l = static_cast<Index>(rng.uni(1, static_cast<Value>(n - len + 1)));
                r = l + len - 1;
                return;
            }
            case Profile::Edges: {
                l = cand[static_cast<std::size_t>(rng.uni(0, static_cast<Value>(cand.size()) - 1))];
                r = cand[static_cast<std::size_t>(rng.uni(0, static_cast<Value>(cand.size()) - 1))];
                if (l > r) std::swap(l, r);
                return;
            }
            default: {
                Value kind = rng.uni(0, 15);
                if (kind == 0) { l = 1; r = n; return; }                       // full array
                if (kind == 1 || kind == 2) {                                   // single element
                    l = r = static_cast<Index>(rng.uni(1, static_cast<Value>(n))); return;
                }
                l = static_cast<Index>(rng.uni(1, static_cast<Value>(n)));
                r = static_cast<Index>(rng.uni(1, static_cast<Value>(n)));
                if (l > r) std::swap(l, r);
            }
        }
    };

    auto newValue = [&]() { return neg ? rng.uni(-valMax, -1) : rng.uni(-valMax, valMax); };
    auto newDelta = [&]() { return neg ? rng.uni(-addMax, -1) : rng.uni(-addMax, addMax); };

    Index cl = 1, cr = n;  // current range for LazyThenSet cycles
    for (std::size_t k = 0; k < numOps; ++k) {
        Op op{'S', 1, 1, 0};
        if (profile == Profile::LazyThenSet) {
            std::size_t phase = k % 4;
            if (phase == 0) pickRange(cl, cr);
            if (mode == Mode::Lazy) {
                if (phase == 0) op = {'A', cl, cr, newDelta()};
                else if (phase == 1) op = {'U', static_cast<Index>(rng.uni(static_cast<Value>(cl), static_cast<Value>(cr))), 0, newValue()};
                else if (phase == 2) op = {'S', cl, cr, 0};
                else op = {'M', cl, cr, 0};
            } else {
                if (phase == 0 || phase == 1) op = {'U', static_cast<Index>(rng.uni(static_cast<Value>(cl), static_cast<Value>(cr))), 0, newValue()};
                else if (phase == 2) op = {'S', cl, cr, 0};
                else op = {'B', cl, cr, 0};
            }
            w.ops.push_back(op);
            continue;
        }
        Index l, r;
        pickRange(l, r);
        Value t = rng.uni(0, 99);
        if (mode == Mode::Lazy) {
            if (t < 20) op = {'U', l, 0, newValue()};
            else if (t < 50) op = {'A', l, r, newDelta()};
            else if (t < 75) op = {'S', l, r, 0};
            else op = {'M', l, r, 0};
        } else {
            if (t < 35) op = {'U', l, 0, newValue()};
            else if (t < 60) op = {'S', l, r, 0};
            else op = {'B', l, r, 0};
        }
        w.ops.push_back(op);
    }
    return w;
}

inline std::string opText(const Op& o) {
    if (o.type == 'U') return std::string("U ") + std::to_string(o.a) + " " + std::to_string(o.x);
    if (o.type == 'A') return std::string("A ") + std::to_string(o.a) + " " + std::to_string(o.b) + " " + std::to_string(o.x);
    return std::string(1, o.type) + " " + std::to_string(o.a) + " " + std::to_string(o.b);
}

inline void writeWorkload(std::ostream& out, const Workload& w, std::size_t count) {
    out << (w.mode == Mode::Lazy ? "LAZY " : "STRETCH ") << w.init.size() << "\n";
    for (std::size_t i = 0; i < w.init.size(); ++i) out << (i ? " " : "") << w.init[i];
    out << "\n";
    for (std::size_t k = 0; k < count && k < w.ops.size(); ++k) out << opText(w.ops[k]) << "\n";
}

} // namespace testing
} // namespace sensor
