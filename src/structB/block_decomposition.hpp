#pragma once
#include "../range_analytics.hpp"
#include "../inspectable.hpp"

namespace sensor {

class BlockDecomposition final : public RangeAnalytics, public Inspectable {
public:

    explicit BlockDecomposition(const std::vector<Value>& initial, Index blockSize = 0);
    ~BlockDecomposition() override;
    BlockDecomposition(const BlockDecomposition&) = delete;
    BlockDecomposition& operator=(const BlockDecomposition&) = delete;

    void pointSet(Index i, Value value) override;
    void rangeAdd(Index left, Index right, Value delta) override;
    Value rangeSum(Index left, Index right) override;
    Value rangeMin(Index left, Index right) override;
    Value bestStretch(Index left, Index right) override;  

    Index blockSize() const { return B_; }
    Index blockCount() const { return blocks_; }
    bool checkInvariants() const;
    std::string selfCheck() const override;

private:
    Index n_, B_, blocks_;
    Value* a_;     
    Value* sum_;   
    Value* min_;   
    Value* add_;   

    Index start(Index b) const { return b * B_; }                              
    Index end(Index b) const { return B_ < n_ - start(b) ? start(b) + B_ : n_; }
    void release();
    void push(Index b);
    void rebuild(Index b);
    void checkRange(Index left, Index right) const;
};

} 
