#include <iostream>
#include <cmath>
#include <climits>
using namespace std;

typedef long long ll;

class BlockDecomposition {
private:
    ll*  data;        
    ll*  blockSum;    
    ll*  blockMin;    
    ll*  blockMax;    
    ll*  lazyAdd;     
    int  n;         
    int  capacity;    
    int  B;           
    int  numBlocks;

    static int ceilDiv(int a, int b) { return (a + b - 1) / b; }

    int blockStart(int b) const { return b * B; }
    int blockEnd(int b)   const { int e = (b + 1) * B - 1; return e < n - 1 ? e : n - 1; }

    void rebuildBlock(int b) {
        int l = blockStart(b), r = blockEnd(b);
        ll s = 0, mn = LLONG_MAX, mx = LLONG_MIN;
        for (int i = l; i <= r; i++) {
            s += data[i];
            if (data[i] < mn) mn = data[i];
            if (data[i] > mx) mx = data[i];
        }
        int len = r - l + 1;
        blockSum[b] = s + lazyAdd[b] * len;
        blockMin[b] = mn + lazyAdd[b];
        blockMax[b] = mx + lazyAdd[b];
    }


    void pushDown(int b) {
        if (lazyAdd[b] == 0) return;
        int l = blockStart(b), r = blockEnd(b);
        for (int i = l; i <= r; i++) data[i] += lazyAdd[b];
        lazyAdd[b] = 0;
        
    }

    void allocate(int cap) {
        capacity = cap;
        B = (int)sqrt((double)cap);
        if (B < 1) B = 1;
        int maxBlocks = ceilDiv(cap, B) + 1;
        data     = new ll[cap]();
        blockSum = new ll[maxBlocks]();
        blockMin = new ll[maxBlocks]();
        blockMax = new ll[maxBlocks]();
        lazyAdd  = new ll[maxBlocks]();
    }

    void freeAll() {
        delete[] data; delete[] blockSum; delete[] blockMin;
        delete[] blockMax; delete[] lazyAdd;
    }

    void grow() {
        int newCap = capacity * 2;
        ll* old = new ll[n];
        for (int b = 0; b < numBlocks; b++) pushDown(b);
        for (int i = 0; i < n; i++) old[i] = data[i];
        freeAll();
        allocate(newCap);
        for (int i = 0; i < n; i++) data[i] = old[i];
        delete[] old;
        numBlocks = ceilDiv(n, B);
        for (int b = 0; b < numBlocks; b++) rebuildBlock(b);
    }

public:
    BlockDecomposition(int initialCapacity = 16) : n(0), numBlocks(0) {
        allocate(initialCapacity < 4 ? 4 : initialCapacity);
    }

    BlockDecomposition(const ll* arr, int size) : n(size), numBlocks(0) {
        allocate(size < 4 ? 4 : size);
        for (int i = 0; i < size; i++) data[i] = arr[i];
        numBlocks = ceilDiv(n, B);
        for (int b = 0; b < numBlocks; b++) rebuildBlock(b);
    }

    ~BlockDecomposition() { freeAll(); }

    BlockDecomposition(const BlockDecomposition&) = delete;
    BlockDecomposition& operator=(const BlockDecomposition&) = delete;

    int size() const { return n; }

    void append(ll value) {
        if (n == capacity) grow();
        data[n++] = value;
        int b = (n - 1) / B;
        if (b >= numBlocks) numBlocks = b + 1;
        rebuildBlock(b);
    }


    void pointUpdate(int idx, ll value) {
        int b = idx / B;
        pushDown(b);
        data[idx] = value;
        rebuildBlock(b);
    }

    void rangeAdd(int l, int r, ll delta) {
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
            lazyAdd[b]  += delta;
            blockSum[b] += delta * len;
            blockMin[b] += delta;
            blockMax[b] += delta;
        }
    
        pushDown(br);
        for (int i = blockStart(br); i <= r; i++) data[i] += delta;
        rebuildBlock(br);
    }

    ll rangeSum(int l, int r) const {
        int bl = l / B, br = r / B;
        ll res = 0;
        if (bl == br) {
            for (int i = l; i <= r; i++) res += data[i] + lazyAdd[bl];
            return res;
        }
        for (int i = l; i <= blockEnd(bl); i++) res += data[i] + lazyAdd[bl];
        for (int b = bl + 1; b < br; b++) res += blockSum[b];
        for (int i = blockStart(br); i <= r; i++) res += data[i] + lazyAdd[br];
        return res;
    }

    ll rangeMin(int l, int r) const {
        int bl = l / B, br = r / B;
        ll res = LLONG_MAX;
        if (bl == br) {
            for (int i = l; i <= r; i++) {
                ll v = data[i] + lazyAdd[bl];
                if (v < res) res = v;
            }
            return res;
        }
        for (int i = l; i <= blockEnd(bl); i++) {
            ll v = data[i] + lazyAdd[bl]; if (v < res) res = v;
        }
        for (int b = bl + 1; b < br; b++) if (blockMin[b] < res) res = blockMin[b];
        for (int i = blockStart(br); i <= r; i++) {
            ll v = data[i] + lazyAdd[br]; if (v < res) res = v;
        }
        return res;
    }


    ll rangeMax(int l, int r) const {
        int bl = l / B, br = r / B;
        ll res = LLONG_MIN;
        if (bl == br) {
            for (int i = l; i <= r; i++) {
                ll v = data[i] + lazyAdd[bl];
                if (v > res) res = v;
            }
            return res;
        }
        for (int i = l; i <= blockEnd(bl); i++) {
            ll v = data[i] + lazyAdd[bl]; if (v > res) res = v;
        }
        for (int b = bl + 1; b < br; b++) if (blockMax[b] > res) res = blockMax[b];
        for (int i = blockStart(br); i <= r; i++) {
            ll v = data[i] + lazyAdd[br]; if (v > res) res = v;
        }
        return res;
    }

    double rangeAvg(int l, int r) const {
        return (double)rangeSum(l, r) / (r - l + 1);
    }

    ll get(int idx) const { return data[idx] + lazyAdd[idx / B]; }
};

