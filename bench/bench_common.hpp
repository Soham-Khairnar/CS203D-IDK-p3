#pragma once

#include "../tests/oracle.hpp"
#include "../tests/workload.hpp"
#include <chrono>
#ifdef _WIN32
// Older MinGW builds may implement steady_clock using a low-resolution tick.
// QueryPerformanceCounter is the Windows high-resolution monotonic counter.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
#include <cstdint>
#include <limits>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace bench {

using Op = sensor::testing::Op;
using Value = sensor::Value;
// Same public timing interface on both Windows and other platforms.
// The result is a monotonic tick count converted to nanoseconds.
struct Timer {
#ifdef _WIN32
    static std::int64_t now() {
        LARGE_INTEGER counter;
        if (!QueryPerformanceCounter(&counter))
            throw std::runtime_error("QueryPerformanceCounter failed");
        return static_cast<std::int64_t>(counter.QuadPart);
    }

    static double elapsed_ns(std::int64_t begin, std::int64_t end) {
        static const double frequency = []() -> double {
            LARGE_INTEGER frequency_value;
            if (!QueryPerformanceFrequency(&frequency_value) || frequency_value.QuadPart <= 0)
                throw std::runtime_error("QueryPerformanceFrequency failed");
            return static_cast<double>(frequency_value.QuadPart);
        }();
        return static_cast<double>(end - begin) * 1'000'000'000.0 / frequency;
    }
#else
    using Clock = std::chrono::steady_clock;
    static Clock::time_point now() { return Clock::now(); }

    static double elapsed_ns(Clock::time_point begin, Clock::time_point end) {
        return std::chrono::duration<double, std::nano>(end - begin).count();
    }
#endif
};

struct Config {
    std::size_t n = 10000;
    std::size_t count = 2000;
    std::string profile = "short";
    std::string operation = "US";
    std::uint64_t seed = 42;
    std::size_t length = 0;      // 0 = use the range profile
    std::size_t block_size = 0;  // 0 = implementation default
    bool verify = false;
};

inline std::uint64_t parse_unsigned(const std::string& value, const char* field,
                                    std::uint64_t maximum) {
    if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos)
        throw std::runtime_error(std::string("invalid ") + field);
    const auto parsed = std::stoull(value);
    if (parsed > maximum)
        throw std::runtime_error(std::string(field) + " is out of range");
    return parsed;
}

inline Config parse(int argc, char** argv) {
    if (argc != 8 && (argc != 9 || std::string(argv[8]) != "--verify"))
        throw std::runtime_error("usage: runner n ops short|long|mixed|full US|LAZY|STRETCH|U|A|S|M|B seed length block_size [--verify]");
    const auto size_max = std::numeric_limits<std::size_t>::max();
    Config c;
    c.n = parse_unsigned(argv[1], "n", 1000000);
    c.count = parse_unsigned(argv[2], "operation count", size_max);
    c.profile = argv[3];
    c.operation = argv[4];
    c.seed = parse_unsigned(argv[5], "seed", std::numeric_limits<std::uint64_t>::max());
    c.length = parse_unsigned(argv[6], "range length", c.n);
    c.block_size = parse_unsigned(argv[7], "block size", size_max);
    c.verify = argc == 9;
    if (c.n == 0 || c.count == 0)
        throw std::runtime_error("n and operation count must be positive");
    if (c.profile != "short" && c.profile != "long" && c.profile != "mixed" && c.profile != "full")
        throw std::runtime_error("unknown range profile");
    if (c.operation != "US" && c.operation != "LAZY" && c.operation != "STRETCH" &&
        c.operation != "U" && c.operation != "A" && c.operation != "S" &&
        c.operation != "M" && c.operation != "B")
        throw std::runtime_error("unknown operation");
    if (c.operation == "U" && c.length != 0)
        throw std::runtime_error("point assignment does not accept a range length");
    if (c.verify && (c.n > 128 || c.count > 1024))
        throw std::runtime_error("verification requires n <= 128 and ops <= 1024");
    return c;
}

struct Work {
    std::vector<Value> initial;
    std::vector<Op> ops;
};

inline Work make_work(const Config& c) {
    if (c.operation == "LAZY" || c.operation == "STRETCH") {
        const auto mode = c.operation == "LAZY" ? sensor::Mode::Lazy : sensor::Mode::Stretch;
        auto profile = sensor::testing::Profile::Mixed;
        if (c.profile == "short") profile = sensor::testing::Profile::Short;
        if (c.profile == "long") profile = sensor::testing::Profile::Long;
        auto w = sensor::testing::makeWorkload(mode, c.n, c.count, profile, c.seed);
        sensor::testing::Rng ranges(c.seed ^ 0x8b51a64c315b765dULL);
        const auto length = c.length != 0 ? c.length : (c.profile == "full" ? c.n : 0);
        if (length != 0) {
            for (auto& op : w.ops) {
                if (op.type == 'U') continue;
                op.a = ranges.uni(1, c.n - length + 1);
                op.b = op.a + length - 1;
            }
        }
        return {std::move(w.init), std::move(w.ops)};
    }

    sensor::testing::Rng rng(c.seed);
    Work w;
    w.initial.reserve(c.n);
    w.ops.reserve(c.count);
    for (std::size_t i = 0; i < c.n; ++i)
        w.initial.push_back(rng.uni(-50, 50));

    for (std::size_t k = 0; k < c.count; ++k) {
        const char type = c.operation == "US" ? (k % 2 == 0 ? 'U' : 'S') : c.operation[0];
        if (type == 'U') {
            w.ops.push_back({type, static_cast<sensor::Index>(rng.uni(1, c.n)), 0, rng.uni(-50, 50)});
            continue;
        }
        auto length = c.length;
        if (length == 0 && c.profile == "short") length = rng.uni(1, std::min<std::size_t>(3, c.n));
        if (length == 0 && c.profile == "long") length = rng.uni(c.n - c.n / 10, c.n);
        if (length == 0 && c.profile == "full") length = c.n;
        sensor::Index l, r;
        if (length != 0) {
            l = rng.uni(1, c.n - length + 1);
            r = l + length - 1;
        } else {
            const auto kind = rng.uni(0, 15);
            l = kind == 0 ? 1 : rng.uni(1, c.n);
            r = kind == 0 ? c.n : (kind <= 2 ? l : rng.uni(1, c.n));
            if (l > r) std::swap(l, r);
        }
        w.ops.push_back({type, l, r, type == 'A' ? rng.uni(-20, 20) : 0});
    }
    return w;
}

inline Value invoke_oracle(sensor::testing::Oracle& oracle, const Op& op) {
    switch (op.type) {
        case 'U': oracle.pointSet(op.a, op.x); return 0;
        case 'A': oracle.rangeAdd(op.a, op.b, op.x); return 0;
        case 'S': return oracle.rangeSum(op.a, op.b);
        case 'M': return oracle.rangeMin(op.a, op.b);
        case 'B': return oracle.bestStretch(op.a, op.b);
        default: throw std::runtime_error("unknown oracle operation");
    }
}

inline std::uint64_t fold(std::uint64_t hash, Value v) {
    const std::uint64_t u = static_cast<std::uint64_t>(v);
    return (hash ^ (u + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2)));
}

struct Storage {
    std::size_t bytes;
    std::size_t block_size = 0;
};

// factory constructs the real data structure. invoke executes a single operation.
// final_sum prevents update-only workloads from being eliminated by optimization.
template <typename Factory, typename Invoke, typename FinalSum, typename Memory>
int measure(const Config& c, const char* structure, Factory factory,
            Invoke invoke, FinalSum final_sum, Memory storage) {
    const Work w = make_work(c); // Not timed
    const std::vector<long long> initial_ll(w.initial.begin(), w.initial.end()); // Not timed
    if (c.verify) {
        auto tree = factory(w.initial, initial_ll);
        sensor::testing::Oracle oracle(w.initial);
        for (const auto& op : w.ops) {
            if (invoke(tree, op) != invoke_oracle(oracle, op))
                throw std::runtime_error("oracle mismatch on " + sensor::testing::opText(op));
        }
        for (sensor::Index i = 1; i <= c.n; ++i) {
            if (invoke(tree, {'S', i, i, 0}) != oracle.rangeSum(i, i))
                throw std::runtime_error("oracle mismatch at final index " + std::to_string(i));
        }
        return 0;
    }
    const auto build_start = Timer::now();
    auto tree = factory(w.initial, initial_ll);
    const auto build_end = Timer::now();
    std::uint64_t checksum = 0xCBF29CE484222325ULL;
    std::size_t queries = 0;
    const auto op_start = Timer::now();
    for (const auto& op : w.ops) {
        const Value ans = invoke(tree, op);
        if (op.type == 'S' || op.type == 'M' || op.type == 'B') {
            checksum = fold(checksum, ans);
            ++queries;
        }
    }
    const auto op_end = Timer::now();
    checksum = fold(checksum, final_sum(tree, c.n)); // Not timed
    const double build_ns = Timer::elapsed_ns(build_start, build_end);
    const double total_ns = Timer::elapsed_ns(op_start, op_end);
    const double build_ms = build_ns / 1'000'000.0;
    // Keep existing column names for compatibility with benchmark.py.
    const auto metrics = storage(tree, c.n);
    std::cout << "structure,n,profile,operation,length,seed,ops,queries,block_size,build_ms,ns_per_op,storage_bytes,checksum,operation_total_ns,effective_block_size\n";
    std::cout << structure << ',' << c.n << ',' << c.profile << ',' << c.operation << ','
              << c.length << ',' << c.seed << ',' << c.count << ',' << queries << ','
              << c.block_size << ',' << std::setprecision(14) << build_ms << ','
              << total_ns / c.count << ',' << metrics.bytes << ',' << checksum
              << ',' << total_ns << ',' << metrics.block_size << '\n';
    return 0;
}

} // namespace bench
