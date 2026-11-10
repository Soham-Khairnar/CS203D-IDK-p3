#include "range_analytics.hpp"
#include "structA/segment_tree.hpp"
#include "structB/block_decomposition.hpp"
#include <stdexcept>

namespace sensor {
std::unique_ptr<RangeAnalytics> makeBackend(
    Structure structure, Mode mode, const std::vector<Value>& initial) {
    if (structure == Structure::A) {
        return std::make_unique<SegmentTree>(mode, initial);
    }
    if (mode == Mode::Stretch) {
        throw std::invalid_argument("STRETCH mode requires Structure A");
    }
    return std::make_unique<BlockDecomposition>(initial);
}
} // namespace sensor
