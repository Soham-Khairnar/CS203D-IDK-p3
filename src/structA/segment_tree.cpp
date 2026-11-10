#include "segment_tree.hpp"
#include <stdexcept>

namespace sensor {
namespace {
[[noreturn]] void unfinished() {
    throw std::logic_error("Structure A is not implemented yet; complete src/structA/segment_tree.cpp");
}
}

SegmentTree::SegmentTree(Mode, const std::vector<Value>&) { unfinished(); }
void SegmentTree::pointSet(Index, Value) { unfinished(); }
void SegmentTree::rangeAdd(Index, Index, Value) { unfinished(); }
Value SegmentTree::rangeSum(Index, Index) { unfinished(); }
Value SegmentTree::rangeMin(Index, Index) { unfinished(); }
Value SegmentTree::bestStretch(Index, Index) { unfinished(); }
} // namespace sensor
