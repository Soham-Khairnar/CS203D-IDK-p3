#include "bench_common.hpp"
#include "../src/structA/stretch_segtree.hpp"

int main(int argc, char** argv) {
    try {
        const auto c = bench::parse(argc, argv);
        if (c.operation == "A" || c.operation == "M" || c.operation == "LAZY")
            throw std::runtime_error("A STRETCH supports U, S, B, US, STRETCH");
        if (c.block_size != 0)
            throw std::runtime_error("block size applies only to B LAZY");
        const auto factory = [](const std::vector<sensor::Value>&, const std::vector<long long>& initial) {
            return StretchSegTree(initial);
        };
        const auto invoke = [](StretchSegTree& t, const bench::Op& op) -> bench::Value {
            const int l = static_cast<int>(op.a), r = static_cast<int>(op.b);
            switch (op.type) {
                case 'U': t.point_set(l, op.x); return 0;
                case 'S': return t.range_sum(l, r);
                case 'B': return t.best_stretch(l, r);
                default: throw std::runtime_error("unsupported operation");
            }
        };
        const auto final_sum = [](StretchSegTree& t, std::size_t n) { return t.range_sum(1, static_cast<int>(n)); };
        const auto storage = [](const StretchSegTree&, std::size_t n) {
            return bench::Storage{static_cast<std::size_t>((4ULL * n + 4ULL) * sizeof(StretchSegTree::Node))};
        };
        return bench::measure(c, "A_STRETCH", factory, invoke, final_sum, storage);
    } catch (const std::exception& e) { std::cerr << "A_STRETCH: " << e.what() << '\n'; return 1; }
}
