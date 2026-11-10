#include "block_decomposition.hpp"
#include <stdexcept>

namespace sensor {
namespace {
[[noreturn]] void unfinished() {
    throw std::logic_error("Structure B is not implemented yet; complete src/structB/block_decomposition.cpp");
}
}

BlockDecomposition::BlockDecomposition(const std::vector<Value>&) { unfinished(); }
void BlockDecomposition::pointSet(Index, Value) { unfinished(); }
void BlockDecomposition::rangeAdd(Index, Index, Value) { unfinished(); }
Value BlockDecomposition::rangeSum(Index, Index) { unfinished(); }
Value BlockDecomposition::rangeMin(Index, Index) { unfinished(); }
Value BlockDecomposition::bestStretch(Index, Index) {
    throw std::logic_error("Structure B does not support STRETCH mode");
}
} // namespace sensor
