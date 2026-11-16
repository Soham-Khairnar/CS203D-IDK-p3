#include "block_decomposition.hpp"
#include <cstdint>
#include <stdexcept>

namespace sensor {
namespace {
const Value kMaxValue = INT64_MAX;

Index isqrt(Index n) {
    Index s = 0;
    while (s + 1 <= n / (s + 1)) ++s;
    return s;
}
} 

BlockDecomposition::BlockDecomposition(const std::vector<Value>& initial, Index blockSize)
    : n_(initial.size()), B_(blockSize), blocks_(0),
      a_(nullptr), sum_(nullptr), min_(nullptr), add_(nullptr) {
    if (n_ == 0) throw std::invalid_argument("BlockDecomposition: need at least one reading");
    if (B_ == 0) {
        B_ = isqrt(n_);
        if (B_ < 1) B_ = 1;
    }
    blocks_ = (n_ - 1) / B_ + 1;
    try {
        a_   = new Value[n_];
        sum_ = new Value[blocks_]();
        min_ = new Value[blocks_]();
        add_ = new Value[blocks_]();       
    } catch (...) {
        release();
        throw;
    }
    for (Index i = 0; i < n_; ++i) a_[i] = initial[i];
    for (Index b = 0; b < blocks_; ++b) rebuild(b);
}

BlockDecomposition::~BlockDecomposition() { release(); }

void BlockDecomposition::release() {
    delete[] a_;   a_ = nullptr;
    delete[] sum_; sum_ = nullptr;
    delete[] min_; min_ = nullptr;
    delete[] add_; add_ = nullptr;
}

void BlockDecomposition::rebuild(Index b) {
    Value s = 0, mn = kMaxValue;
    for (Index i = start(b); i < end(b); ++i) {
        const Value x = a_[i] + add_[b];
        s += x;
        if (x < mn) mn = x;
    }
    sum_[b] = s;
    min_[b] = mn;
}


void BlockDecomposition::push(Index b) {
    if (add_[b] == 0) return;
    for (Index i = start(b); i < end(b); ++i) a_[i] += add_[b];
    add_[b] = 0;
}

void BlockDecomposition::checkRange(Index l, Index r) const {
    if (l < 1 || r > n_ || l > r) throw std::out_of_range("BlockDecomposition: bad range");
}

void BlockDecomposition::pointSet(Index i, Value value) {
    checkRange(i, i);
    const Index i0 = i - 1, b = i0 / B_;
    push(b);                 
    a_[i0] = value;
    rebuild(b);
}

void BlockDecomposition::rangeAdd(Index l, Index r, Value delta) {
    checkRange(l, r);
    const Index lo0 = l - 1, hi0 = r;      
    for (Index b = lo0 / B_; b <= (hi0 - 1) / B_; ++b) {
        const Index lo = lo0 > start(b) ? lo0 : start(b);
        const Index hi = hi0 < end(b) ? hi0 : end(b);
        if (lo == start(b) && hi == end(b)) {          
            add_[b] += delta;
            sum_[b] += delta * static_cast<Value>(hi - lo);
            min_[b] += delta;
        } else {                                      
            push(b);
            for (Index i = lo; i < hi; ++i) a_[i] += delta;
            rebuild(b);
        }
    }
}

Value BlockDecomposition::rangeSum(Index l, Index r) {
    checkRange(l, r);
    const Index lo0 = l - 1, hi0 = r;
    Value res = 0;
    for (Index b = lo0 / B_; b <= (hi0 - 1) / B_; ++b) {
        const Index lo = lo0 > start(b) ? lo0 : start(b);
        const Index hi = hi0 < end(b) ? hi0 : end(b);
        if (lo == start(b) && hi == end(b)) res += sum_[b];
        else for (Index i = lo; i < hi; ++i) res += a_[i] + add_[b];
    }
    return res;
}

Value BlockDecomposition::rangeMin(Index l, Index r) {
    checkRange(l, r);
    const Index lo0 = l - 1, hi0 = r;
    Value res = kMaxValue;
    for (Index b = lo0 / B_; b <= (hi0 - 1) / B_; ++b) {
        const Index lo = lo0 > start(b) ? lo0 : start(b);
        const Index hi = hi0 < end(b) ? hi0 : end(b);
        if (lo == start(b) && hi == end(b)) {
            if (min_[b] < res) res = min_[b];
        } else {
            for (Index i = lo; i < hi; ++i) {
                const Value v = a_[i] + add_[b];
                if (v < res) res = v;
            }
        }
    }
    return res;
}

Value BlockDecomposition::bestStretch(Index, Index) {
    throw std::logic_error("Structure B does not support bestStretch; use Structure A in STRETCH mode");
}

bool BlockDecomposition::checkInvariants() const {
    for (Index b = 0; b < blocks_; ++b) {
        Value s = 0, mn = kMaxValue;
        for (Index i = start(b); i < end(b); ++i) {
            const Value x = a_[i] + add_[b];
            s += x;
            if (x < mn) mn = x;
        }
        if (s != sum_[b] || mn != min_[b]) return false;
    }
    return true;
}

std::string BlockDecomposition::selfCheck() const {
    for (Index b = 0; b < blocks_; ++b) {
        Value s = 0, mn = kMaxValue;
        for (Index i = start(b); i < end(b); ++i) {
            const Value x = a_[i] + add_[b];
            s += x;
            if (x < mn) mn = x;
        }
        if (s != sum_[b]) return "block " + std::to_string(b) + " sum is inconsistent";
        if (mn != min_[b]) return "block " + std::to_string(b) + " minimum is inconsistent";
    }
    return {};
}

} 

