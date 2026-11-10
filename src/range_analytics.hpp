#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace sensor {

using Value = std::int64_t;
using Index = std::size_t;
enum class Mode { Lazy, Stretch };
enum class Structure { A, B };

// Contract for both owners:
// - All indices are 1-based; ranges are inclusive and nonempty.
// - The application validates indices and supported operations before dispatch.
// - pointSet assigns an absolute value, including after pending range additions.
// - bestStretch returns a NONEMPTY maximum subarray sum.
// - Arithmetic uses Value; supported inputs must keep results within its bounds.
// - Backends must not print anything. Unsupported operations throw logic_error.
class RangeAnalytics {
public:
    virtual ~RangeAnalytics() = default;
    virtual void pointSet(Index i, Value value) = 0;
    virtual void rangeAdd(Index left, Index right, Value delta) = 0;
    virtual Value rangeSum(Index left, Index right) = 0;
    virtual Value rangeMin(Index left, Index right) = 0;
    virtual Value bestStretch(Index left, Index right) = 0;
};

std::unique_ptr<RangeAnalytics> makeBackend(
    Structure structure, Mode mode, const std::vector<Value>& initial);

} // namespace sensor
