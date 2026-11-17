#ifndef STRETCH_SEGTREE_HPP
#define STRETCH_SEGTREE_HPP

#include <vector>
#include <algorithm>

class StretchSegTree {
public:
    using ll = long long;

    struct Node {
        ll total;
        ll pref;
        ll suf;
        ll best;
    };

    StretchSegTree() : n(0) {}
    explicit StretchSegTree(const std::vector<ll>& a) { build(a); }

    void build(const std::vector<ll>& a) {
        n = (int)a.size();
        t.assign(4 * n + 4, Node{0, 0, 0, 0});
        if (n > 0) build(1, 1, n, a);
    }

    void point_set(int i, ll v) { set(1, 1, n, i, v); }

    ll range_sum(int l, int r) { return query(1, 1, n, l, r).total; }

    ll best_stretch(int l, int r) { return query(1, 1, n, l, r).best; }

    int size() const { return n; }

    static Node combine(const Node& a, const Node& b) {
        Node c;
        c.total = a.total + b.total;
        c.pref = std::max(a.pref, a.total + b.pref);
        c.suf = std::max(b.suf, b.total + a.suf);
        c.best = std::max({a.best, b.best, a.suf + b.pref});
        return c;
    }

private:
    int n;
    std::vector<Node> t;

    static Node leaf(ll v) { return Node{v, v, v, v}; }

    void build(int node, int l, int r, const std::vector<ll>& a) {
        if (l == r) { t[node] = leaf(a[l - 1]); return; }
        int m = (l + r) / 2;
        build(2 * node, l, m, a);
        build(2 * node + 1, m + 1, r, a);
        t[node] = combine(t[2 * node], t[2 * node + 1]);
    }

    void set(int node, int l, int r, int i, ll v) {
        if (l == r) { t[node] = leaf(v); return; }
        int m = (l + r) / 2;
        if (i <= m) set(2 * node, l, m, i, v);
        else set(2 * node + 1, m + 1, r, i, v);
        t[node] = combine(t[2 * node], t[2 * node + 1]);
    }

    Node query(int node, int l, int r, int ql, int qr) {
        if (ql <= l && r <= qr) return t[node];
        int m = (l + r) / 2;
        if (qr <= m) return query(2 * node, l, m, ql, qr);
        if (ql > m) return query(2 * node + 1, m + 1, r, ql, qr);
        return combine(query(2 * node, l, m, ql, qr),
                       query(2 * node + 1, m + 1, r, ql, qr));
    }
};

#endif
