#include "segment_tree.hpp"
#include <stdexcept>

namespace sensor {

SegmentTree::SegmentTree(Mode mode, const std::vector<Value>& initial)
    : mode_(mode), n_(initial.size()), lazy_(nullptr), stretch_(nullptr) {
    if (n_ == 0) throw std::invalid_argument("SegmentTree: need at least one reading");
    std::vector<long long> a(initial.begin(), initial.end());
    if (mode_ == Mode::Lazy) lazy_ = new LazySegTree(a);
    else stretch_ = new StretchSegTree(a);
}

SegmentTree::~SegmentTree() {
    delete lazy_;
    delete stretch_;
}

void SegmentTree::checkRange(Index l, Index r) const {
    if (l < 1 || r > n_ || l > r) throw std::out_of_range("SegmentTree: bad range");
}

void SegmentTree::pointSet(Index i, Value value) {
    checkRange(i, i);
    if (lazy_) lazy_->point_set((int)i, value);
    else stretch_->point_set((int)i, value);
}

void SegmentTree::rangeAdd(Index l, Index r, Value delta) {
    checkRange(l, r);
    if (!lazy_) throw std::logic_error("Structure A STRETCH mode does not support range add");
    lazy_->range_add((int)l, (int)r, delta);
}

Value SegmentTree::rangeSum(Index l, Index r) {
    checkRange(l, r);
    if (lazy_) return lazy_->range_sum((int)l, (int)r);
    return stretch_->range_sum((int)l, (int)r);
}

Value SegmentTree::rangeMin(Index l, Index r) {
    checkRange(l, r);
    if (!lazy_) throw std::logic_error("Structure A STRETCH mode does not support range minimum");
    return lazy_->range_min((int)l, (int)r);
}

Value SegmentTree::bestStretch(Index l, Index r) {
    checkRange(l, r);
    if (!stretch_) throw std::logic_error("Structure A LAZY mode does not support best stretch");
    return stretch_->best_stretch((int)l, (int)r);
}

std::string SegmentTree::selfCheck() const {
    if (lazy_) return lazy_->selfCheck();
    return stretch_->selfCheck();
}

}
