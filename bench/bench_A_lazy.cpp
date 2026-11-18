#include "../src/structA/lazy_segtree.hpp"

int main(int argc, char** argv) {
    try {
        const auto c = bench::parse(argc, argv);
        if (c.operation == "B" || c.operation == "STRETCH")
            throw std::runtime_error("A LAZY supports U, A, S, M, US, LAZY");
        if (c.block_size != 0)
            throw std::runtime_error("block size applies only to B LAZY");
        const auto factory = [](const std::vector<sensor::Value>&, const std::vector<long long>& initial) {
            return LazySegTree(initial);
        };
        const auto invoke = [](LazySegTree& t, const bench::Op& op) -> bench::Value {
            const int l = static_cast<int>(op.a), r = static_cast<int>(op.b);
            switch (op.type) {
                case 'U': t.point_set(l, op.x); return 0;
                case 'A': t.range_add(l, r, op.x); return 0;
                case 'S': return t.range_sum(l, r);
                case 'M': return t.range_min(l, r);
                default: throw std::runtime_error("unsupported operation");
            }
        };
        const auto final_sum = [](LazySegTree& t, std::size_t n) { return t.range_sum(1, static_cast<int>(n)); };
        const auto storage = [](const LazySegTree&, std::size_t n) {
            return bench::Storage{static_cast<std::size_t>(3ULL * (4ULL * n + 4ULL) * sizeof(long long))};
        };
        return bench::measure(c, "A_LAZY", factory, invoke, final_sum, storage);
    } catch (const std::exception& e) { std::cerr << "A_LAZY: " << e.what() << '\n'; return 1; }
}
