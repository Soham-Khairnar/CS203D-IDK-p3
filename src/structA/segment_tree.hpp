#pragma once

#include "../inspectable.hpp"
#include "../range_analytics.hpp"
#include "lazy_segtree.hpp"
#include "stretch_segtree.hpp"
#include <memory>
#include <vector>

namespace sensor {

// Adapter from Soham's standalone tree implementations to the application API.
// Public indices stay 1-based and ranges inclusive.
class SegmentTree final : public RangeAnalytics {
public:
    SegmentTree(Mode mode, const std::vector<Value>& initial);

    void pointSet(Index i, Value value) override;
    void rangeAdd(Index left, Index right, Value delta) override;
    Value rangeSum(Index left, Index right) override;
    Value rangeMin(Index left, Index right) override;
    Value bestStretch(Index left, Index right) override;

private:
    Mode mode_;
    std::unique_ptr<LazySegTree> lazy_;
    std::unique_ptr<StretchSegTree> stretch_;
    static int checkedIndex(Index index);
};

} // namespace sensor
