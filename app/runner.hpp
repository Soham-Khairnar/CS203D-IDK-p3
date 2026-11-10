#pragma once
#include "../src/range_analytics.hpp"
#include <functional>
#include <iosfwd>

namespace sensor {
using BackendFactory = std::function<std::unique_ptr<RangeAnalytics>(
    Structure, Mode, const std::vector<Value>&)>;

// Reads one operation file through EOF. Throws on malformed input/backend error.
// Factory injection lets application tests verify dispatch before backends exist.
void run(std::istream& input, std::ostream& output, Structure structure,
         const BackendFactory& factory = makeBackend);
} // namespace sensor
