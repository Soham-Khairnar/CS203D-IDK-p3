#pragma once
#include "../range_analytics.hpp"

namespace sensor {

// Structure B owner: LAZY only. Store raw readings and per-block sum/min/add.
class BlockDecomposition final : public RangeAnalytics {
public:
    explicit BlockDecomposition(const std::vector<Value>& initial);
    void pointSet(Index i, Value value) override;
    void rangeAdd(Index left, Index right, Value delta) override;
    Value rangeSum(Index left, Index right) override;
    Value rangeMin(Index left, Index right) override;
    Value bestStretch(Index left, Index right) override;
};

} // namespace sensor
