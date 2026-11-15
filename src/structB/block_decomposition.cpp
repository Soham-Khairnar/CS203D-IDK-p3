
#include <cmath>
#include <climits>
#include <stdexcept>

#ifndef VALUE_DEFINED
#define VALUE_DEFINED
using Value = long long;  
#endif

class BlockDecomposition {
private:
    Value* data;       
    Value* bSum;
    Value* bMin;    
    Value* bMax;       
    Value* bPre;       
    Value* bSuf;    
    Value* bBest;     
    Value* lazyAdd;    
    char*  dirty;      
    int n, capacity, B, numBlocks;

    static int ceilDiv(int a, int b) { return (a + b - 1) / b; }
    static Value vmax(Value a, Value b) { return a > b ? a : b; }

    int blockStart(int b) const { return b * B; }
    int blockEnd(int b) const {
        int e = (b + 1) * B - 1;
        return e < n - 1 ? e : n - 1;
    }

    void rebuildBlock(int b) {
        int l = blockStart(b), r = blockEnd(b);
        Value lz = lazyAdd[b];
        Value s = 0, mn = LLONG_MAX, mx = LLONG_MIN;
        Value pre = LLONG_MIN, cur = 0, best = LLONG_MIN;
        for (int i = l; i <= r; i++) {
            Value x = data[i] + lz;
            s += x;
            if (x < mn) mn = x;
            if (x > mx) mx = x;
            if (s > pre) pre = s;                       // running prefix sum
            cur = (i == l) ? x : vmax(x, cur + x);      // Kadane (best ending at i)
            if (cur > best) best = cur;
        }
        bSum[b] = s; bMin[b] = mn; bMax[b] = mx;
        bPre[b] = pre; bSuf[b] = cur; bBest[b] = best;
        dirty[b] = 0;
    }

    void pushDown(int b) {
        if (lazyAdd[b] == 0) return;
        int l = blockStart(b), r = blockEnd(b);
        for (int i = l; i <= r; i++) data[i] += lazyAdd[b];
        lazyAdd[b] = 0;
    }

    void allocate(int cap) {
        capacity = cap;
        B = (int)std::sqrt((double)cap);
        if (B < 1) B = 1;
        int maxBlocks = ceilDiv(cap, B) + 1;
        data    = new Value[cap]();
        bSum    = new Value[maxBlocks]();
        bMin    = new Value[maxBlocks]();
        bMax    = new Value[maxBlocks]();
        bPre    = new Value[maxBlocks]();
        bSuf    = new Value[maxBlocks]();
        bBest   = new Value[maxBlocks]();
        lazyAdd = new Value[maxBlocks]();
        dirty   = new char[maxBlocks]();
    }

    void freeAll() {
        delete[] data; delete[] bSum; delete[] bMin; delete[] bMax;
        delete[] bPre; delete[] bSuf; delete[] bBest;
        delete[] lazyAdd; delete[] dirty;
    }

    void grow() {
        for (int b = 0; b < numBlocks; b++) pushDown(b);
        Value* old = new Value[n];
        for (int i = 0; i < n; i++) old[i] = data[i];
        freeAll();
        allocate(capacity * 2);
        for (int i = 0; i < n; i++) data[i] = old[i];
        delete[] old;
        numBlocks = ceilDiv(n, B);
        for (int b = 0; b < numBlocks; b++) rebuildBlock(b);
    }


    struct Acc { bool has; Value best, suf; };

    static void absorb(Acc& a, Value sum, Value pre, Value suf, Value best) {
        if (!a.has) { a.best = best; a.suf = suf; a.has = true; return; }
        Value cross = a.suf + pre;
        if (best  > a.best) a.best = best;
        if (cross > a.best) a.best = cross;
        a.suf = vmax(suf, a.suf + sum);
    }
    static void absorbElem(Acc& a, Value x) { absorb(a, x, x, x, x); }

    void pointSet0(int idx, Value value) {
        int b = idx / B;
        pushDown(b);               // clear pending lazy so assignment is absolute
        data[idx] = value;
        rebuildBlock(b);
    }

    void rangeAdd0(int l, int r, Value delta) {
        int bl = l / B, br = r / B;
        if (bl == br) {
            pushDown(bl);
            for (int i = l; i <= r; i++) data[i] += delta;
            rebuildBlock(bl);
            return;
        }
        pushDown(bl);
        for (int i = l; i <= blockEnd(bl); i++) data[i] += delta;
        rebuildBlock(bl);
        for (int b = bl + 1; b < br; b++) {
            int len = blockEnd(b) - blockStart(b) + 1;
            lazyAdd[b] += delta;
            bSum[b] += delta * len;
            bMin[b] += delta;
            bMax[b] += delta;
            dirty[b] = 1;          
        }
        pushDown(br);
        for (int i = blockStart(br); i <= r; i++) data[i] += delta;
        rebuildBlock(br);
    }

    Value rangeSum0(int l, int r) const {
        int bl = l / B, br = r / B;
        Value res = 0;
        if (bl == br) {
            for (int i = l; i <= r; i++) res += data[i] + lazyAdd[bl];
            return res;
        }
        for (int i = l; i <= blockEnd(bl); i++) res += data[i] + lazyAdd[bl];
        for (int b = bl + 1; b < br; b++) res += bSum[b];
        for (int i = blockStart(br); i <= r; i++) res += data[i] + lazyAdd[br];
        return res;
    }

    Value rangeMin0(int l, int r) const {
        int bl = l / B, br = r / B;
        Value res = LLONG_MAX;
        if (bl == br) {
            for (int i = l; i <= r; i++) {
                Value v = data[i] + lazyAdd[bl]; if (v < res) res = v;
            }
            return res;
        }
        for (int i = l; i <= blockEnd(bl); i++) {
            Value v = data[i] + lazyAdd[bl]; if (v < res) res = v;
        }
        for (int b = bl + 1; b < br; b++) if (bMin[b] < res) res = bMin[b];
        for (int i = blockStart(br); i <= r; i++) {
            Value v = data[i] + lazyAdd[br]; if (v < res) res = v;
        }
        return res;
    }

    Value rangeMax0(int l, int r) const {
        int bl = l / B, br = r / B;
        Value res = LLONG_MIN;
        if (bl == br) {
            for (int i = l; i <= r; i++) {
                Value v = data[i] + lazyAdd[bl]; if (v > res) res = v;
            }
            return res;
        }
        for (int i = l; i <= blockEnd(bl); i++) {
            Value v = data[i] + lazyAdd[bl]; if (v > res) res = v;
        }
        for (int b = bl + 1; b < br; b++) if (bMax[b] > res) res = bMax[b];
        for (int i = blockStart(br); i <= r; i++) {
            Value v = data[i] + lazyAdd[br]; if (v > res) res = v;
        }
        return res;
    }

    Value bestStretch0(int l, int r) {
        int bl = l / B, br = r / B;
        Acc a; a.has = false; a.best = 0; a.suf = 0;
        if (bl == br) {
            for (int i = l; i <= r; i++) absorbElem(a, data[i] + lazyAdd[bl]);
            return a.best;
        }
        for (int i = l; i <= blockEnd(bl); i++) absorbElem(a, data[i] + lazyAdd[bl]);
        for (int b = bl + 1; b < br; b++) {
            if (dirty[b]) rebuildBlock(b);     // lazily refresh four-values
            absorb(a, bSum[b], bPre[b], bSuf[b], bBest[b]);
        }
        for (int i = blockStart(br); i <= r; i++) absorbElem(a, data[i] + lazyAdd[br]);
        return a.best;
    }

public:
    explicit BlockDecomposition(int initialCapacity = 16) : n(0), numBlocks(0) {
        allocate(initialCapacity < 4 ? 4 : initialCapacity);
    }

    // arr[0..size-1] is the initial data; it becomes positions 1..size.
    BlockDecomposition(const Value* arr, int size) : n(size), numBlocks(0) {
        allocate(size < 4 ? 4 : size);
        for (int i = 0; i < size; i++) data[i] = arr[i];
        numBlocks = ceilDiv(n, B);
        for (int b = 0; b < numBlocks; b++) rebuildBlock(b);
    }

    ~BlockDecomposition() { freeAll(); }
    BlockDecomposition(const BlockDecomposition&) = delete;
    BlockDecomposition& operator=(const BlockDecomposition&) = delete;

    int size() const { return n; }

    void append(Value value) {
        if (n == capacity) grow();
        int b = n / B;
        if (b < numBlocks) pushDown(b);   
        data[n++] = value;
        if (b >= numBlocks) numBlocks = b + 1;
        rebuildBlock(b);
    }


    void  pointSet(int idx, Value v)       { pointSet0(idx - 1, v); }
    void  rangeAdd(int l, int r, Value d)  { rangeAdd0(l - 1, r - 1, d); }
    Value rangeSum(int l, int r) const     { return rangeSum0(l - 1, r - 1); }
    Value rangeMin(int l, int r) const     { return rangeMin0(l - 1, r - 1); }
    Value rangeMax(int l, int r) const     { return rangeMax0(l - 1, r - 1); }
    Value bestStretch(int l, int r)        { return bestStretch0(l - 1, r - 1); }
    Value get(int idx) const               { return data[idx - 1] + lazyAdd[(idx - 1) / B]; }

    void rangeAssign(int, int, Value) {
        throw std::logic_error("BlockDecomposition: rangeAssign not supported");
    }
};

#include <iostream>
using namespace std;

int main() {
    Value init[] = {5, -3, 8, -10, 9, 2, -7, 4};
    BlockDecomposition bd(init, 8);

    cout << bd.rangeSum(1, 8) << "\n";     
    cout << bd.bestStretch(1, 8) << "\n";   
    cout << bd.bestStretch(4, 4) << "\n";

    bd.rangeAdd(2, 7, 10);                   
    cout << bd.bestStretch(1, 8) << "\n";   
    bd.pointSet(3, -100);                  
    cout << bd.get(3) << "\n";          
    cout << bd.rangeMax(1, 8) << "\n";      

    bd.append(50);
    cout << bd.size() << " " << bd.get(9) << "\n";   

    try { bd.rangeAssign(1, 2, 0); }
    catch (const logic_error& e) { cout << "caught: " << e.what() << "\n"; }
}