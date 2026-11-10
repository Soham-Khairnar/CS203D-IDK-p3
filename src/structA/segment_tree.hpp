#pragma once
#include "../range_analytics.hpp"

namespace sensor {

// Structure A owner: replace the explicit placeholders in segment_tree.cpp.
// LAZY: sum, minimum, pending add. STRETCH: total, prefix, suffix, best.
class SegmentTree final : public RangeAnalytics {
public:
    SegmentTree(Mode mode, const std::vector<Value>& initial);
    void pointSet(Index i, Value value) override;
    void rangeAdd(Index left, Index right, Value delta) override;
    Value rangeSum(Index left, Index right) override;
    Value rangeMin(Index left, Index right) override;
    Value bestStretch(Index left, Index right) override;
};

} // namespace sensor
