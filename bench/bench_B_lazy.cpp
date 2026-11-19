#include "bench_common.hpp"
#include "../src/structB/block_decomposition.hpp"

int main(int argc, char** argv) {
    try {
        const auto c = bench::parse(argc, argv);
        if (c.operation == "B" || c.operation == "STRETCH")
            throw std::runtime_error("B LAZY supports U, A, S, M, US, LAZY");
        const auto factory = [&](const std::vector<sensor::Value>& initial, const std::vector<long long>&) {
            return std::make_unique<sensor::BlockDecomposition>(initial, c.block_size);
        };
        const auto invoke = [](const std::unique_ptr<sensor::BlockDecomposition>& t, const bench::Op& op) -> bench::Value {
            switch (op.type) {
                case 'U': t->pointSet(op.a, op.x); return 0;
                case 'A': t->rangeAdd(op.a, op.b, op.x); return 0;
                case 'S': return t->rangeSum(op.a, op.b);
                case 'M': return t->rangeMin(op.a, op.b);
                default: throw std::runtime_error("unsupported operation");
            }
        };
        const auto final_sum = [](const std::unique_ptr<sensor::BlockDecomposition>& t, std::size_t n) {
            return t->rangeSum(1, n);
        };
        const auto storage = [](const std::unique_ptr<sensor::BlockDecomposition>& t, std::size_t n) {
            const std::size_t blocks = t->blockCount();
            return bench::Storage{(n + 3 * blocks) * sizeof(sensor::Value), t->blockSize()};
        };
        return bench::measure(c, "B_LAZY", factory, invoke, final_sum, storage);
    } catch (const std::exception& e) { std::cerr << "B_LAZY: " << e.what() << '\n'; return 1; }
}
