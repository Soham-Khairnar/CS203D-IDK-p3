#include "segment_tree.hpp"
#include <limits>
#include <stdexcept>

namespace sensor {
namespace {
std::vector<long long> toLongLong(const std::vector<Value>& values) {
    std::vector<long long> converted;
    converted.reserve(values.size());
    for (Value value : values) {
        if (value < static_cast<Value>(std::numeric_limits<long long>::min()) ||
            value > static_cast<Value>(std::numeric_limits<long long>::max())) {
            throw std::out_of_range("reading does not fit the segment tree value type");
        }
        converted.push_back(static_cast<long long>(value));
    }
    return converted;
}
}

SegmentTree::SegmentTree(Mode mode, const std::vector<Value>& initial)
    : mode_(mode) {
    if (initial.empty()) throw std::invalid_argument("SegmentTree: need at least one reading");
    const auto values = toLongLong(initial);
    if (mode_ == Mode::Lazy) lazy_.reset(new LazySegTree(values));
    else stretch_.reset(new StretchSegTree(values));
}

int SegmentTree::checkedIndex(Index index) {
    if (index < 1 || index > static_cast<Index>(std::numeric_limits<int>::max()))
        throw std::out_of_range("SegmentTree: index out of range");
    return static_cast<int>(index);
}

void SegmentTree::pointSet(Index i, Value value) {
    const int index = checkedIndex(i);
    if (mode_ == Mode::Lazy) lazy_->point_set(index, static_cast<long long>(value));
    else stretch_->point_set(index, static_cast<long long>(value));
}

void SegmentTree::rangeAdd(Index left, Index right, Value delta) {
    if (mode_ != Mode::Lazy) throw std::logic_error("rangeAdd is only available in LAZY mode");
    lazy_->range_add(checkedIndex(left), checkedIndex(right), static_cast<long long>(delta));
}

Value SegmentTree::rangeSum(Index left, Index right) {
    const int l = checkedIndex(left), r = checkedIndex(right);
    return mode_ == Mode::Lazy ? lazy_->range_sum(l, r) : stretch_->range_sum(l, r);
}

Value SegmentTree::rangeMin(Index left, Index right) {
    if (mode_ != Mode::Lazy) throw std::logic_error("rangeMin is only available in LAZY mode");
    return lazy_->range_min(checkedIndex(left), checkedIndex(right));
}

Value SegmentTree::bestStretch(Index left, Index right) {
    if (mode_ != Mode::Stretch) throw std::logic_error("bestStretch is only available in STRETCH mode");
    return stretch_->best_stretch(checkedIndex(left), checkedIndex(right));
}

} // namespace sensor
