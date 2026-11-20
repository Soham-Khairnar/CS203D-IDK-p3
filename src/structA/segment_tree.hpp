#pragma once
#include "../range_analytics.hpp"
#include "../inspectable.hpp"
#include "lazy_segtree.hpp"
#include "stretch_segtree.hpp"
#include <string>

namespace sensor {

class SegmentTree final : public RangeAnalytics, public Inspectable {
public:
    SegmentTree(Mode mode, const std::vector<Value>& initial);
    ~SegmentTree() override;
    SegmentTree(const SegmentTree&) = delete;
    SegmentTree& operator=(const SegmentTree&) = delete;

    void pointSet(Index i, Value value) override;
    void rangeAdd(Index left, Index right, Value delta) override;
    Value rangeSum(Index left, Index right) override;
    Value rangeMin(Index left, Index right) override;
    Value bestStretch(Index left, Index right) override;

    std::string selfCheck() const override;

private:
    Mode mode_;
    Index n_;
    LazySegTree* lazy_;
    StretchSegTree* stretch_;

    void checkRange(Index left, Index right) const;
};

}
