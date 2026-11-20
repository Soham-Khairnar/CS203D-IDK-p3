#ifndef LAZY_SEGTREE_HPP
#define LAZY_SEGTREE_HPP

#include <vector>
#include <algorithm>
#include <climits>
#include <string>

class LazySegTree {
public:
    using ll = long long;

    LazySegTree() : n(0) {}
    explicit LazySegTree(const std::vector<ll>& a) { build(a); }

    void build(const std::vector<ll>& a) {
        n = (int)a.size();
        sum.assign(4 * n + 4, 0);
        mn.assign(4 * n + 4, 0);
        lz.assign(4 * n + 4, 0);
        if (n > 0) build(1, 1, n, a);
    }

    void point_set(int i, ll v) {
        ll cur = range_sum(i, i);
        range_add(i, i, v - cur);
    }

    void range_add(int l, int r, ll x) { add(1, 1, n, l, r, x); }

    ll range_sum(int l, int r) { return qsum(1, 1, n, l, r); }

    ll range_min(int l, int r) { return qmin(1, 1, n, l, r); }

    int size() const { return n; }

    std::string selfCheck() const {
        if (n > 0) return check(1, 1, n);
        return std::string();
    }

private:
    int n;
    std::vector<ll> sum;
    std::vector<ll> mn;
    std::vector<ll> lz;

    std::string check(int node, int l, int r) const {
        if (l == r) return std::string();
        int m = (l + r) / 2;
        if (sum[node] != sum[2 * node] + sum[2 * node + 1] + lz[node] * (r - l + 1))
            return "lazy tree: sum mismatch at node " + std::to_string(node);
        if (mn[node] != std::min(mn[2 * node], mn[2 * node + 1]) + lz[node])
            return "lazy tree: min mismatch at node " + std::to_string(node);
        std::string e = check(2 * node, l, m);
        if (!e.empty()) return e;
        return check(2 * node + 1, m + 1, r);
    }

    void pull(int node) {
        sum[node] = sum[2 * node] + sum[2 * node + 1];
        mn[node] = std::min(mn[2 * node], mn[2 * node + 1]);
    }

    void apply(int node, int len, ll x) {
        sum[node] += x * len;
        mn[node] += x;
        lz[node] += x;
    }

    void push(int node, int l, int r) {
        if (lz[node] != 0) {
            int m = (l + r) / 2;
            apply(2 * node, m - l + 1, lz[node]);
            apply(2 * node + 1, r - m, lz[node]);
            lz[node] = 0;
        }
    }

    void build(int node, int l, int r, const std::vector<ll>& a) {
        if (l == r) {
            sum[node] = mn[node] = a[l - 1];
            return;
        }
        int m = (l + r) / 2;
        build(2 * node, l, m, a);
        build(2 * node + 1, m + 1, r, a);
        pull(node);
    }

    void add(int node, int l, int r, int ql, int qr, ll x) {
        if (qr < l || r < ql) return;
        if (ql <= l && r <= qr) {
            apply(node, r - l + 1, x);
            return;
        }
        push(node, l, r);
        int m = (l + r) / 2;
        add(2 * node, l, m, ql, qr, x);
        add(2 * node + 1, m + 1, r, ql, qr, x);
        pull(node);
    }

    ll qsum(int node, int l, int r, int ql, int qr) {
        if (qr < l || r < ql) return 0;
        if (ql <= l && r <= qr) return sum[node];
        push(node, l, r);
        int m = (l + r) / 2;
        return qsum(2 * node, l, m, ql, qr) + qsum(2 * node + 1, m + 1, r, ql, qr);
    }

    ll qmin(int node, int l, int r, int ql, int qr) {
        if (qr < l || r < ql) return LLONG_MAX;
        if (ql <= l && r <= qr) return mn[node];
        push(node, l, r);
        int m = (l + r) / 2;
        return std::min(qmin(2 * node, l, m, ql, qr), qmin(2 * node + 1, m + 1, r, ql, qr));
    }
};

#endif
