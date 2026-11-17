#include <iostream>
#include <random>
#include <vector>
#include <climits>
#include <algorithm>
#include "lazy_segtree.hpp"
#include "stretch_segtree.hpp"

using ll = long long;

static ll kadane(const std::vector<ll>& a, int l, int r) {
    ll best = LLONG_MIN, cur = 0;
    for (int i = l; i <= r; i++) {
        cur = std::max(a[i - 1], cur + a[i - 1]);
        best = std::max(best, cur);
    }
    return best;
}

int main() {
    const unsigned SEED = 12345;
    const int OPS = 200000;
    std::mt19937_64 rng(SEED);
    auto rnd = [&](ll lo, ll hi) { return (ll)(lo + rng() % (hi - lo + 1)); };
    long long checks = 0;

    for (int n : {1, 2, 3, 7, 50, 1000}) {
        std::vector<ll> a(n);
        for (auto& x : a) x = rnd(-100, 100);
        LazySegTree st(a);
        for (int op = 0; op < OPS / 6; op++) {
            int l = (int)rnd(1, n), r = (int)rnd(1, n);
            if (l > r) std::swap(l, r);
            int type = (int)rnd(0, 3);
            if (type == 0) {
                ll v = rnd(-100, 100);
                st.point_set(l, v);
                a[l - 1] = v;
            } else if (type == 1) {
                ll x = rnd(-50, 50);
                st.range_add(l, r, x);
                for (int i = l; i <= r; i++) a[i - 1] += x;
            } else if (type == 2) {
                ll s = 0;
                for (int i = l; i <= r; i++) s += a[i - 1];
                if (st.range_sum(l, r) != s) { std::cout << "FAIL lazy sum n=" << n << "\n"; return 1; }
                checks++;
            } else {
                ll m = *std::min_element(a.begin() + (l - 1), a.begin() + r);
                if (st.range_min(l, r) != m) { std::cout << "FAIL lazy min n=" << n << "\n"; return 1; }
                checks++;
            }
        }
    }

    for (int n : {1, 2, 3, 7, 50, 1000}) {
        std::vector<ll> a(n);
        for (auto& x : a) x = rnd(-100, 100);
        StretchSegTree st(a);
        for (int op = 0; op < OPS / 6; op++) {
            int l = (int)rnd(1, n), r = (int)rnd(1, n);
            if (l > r) std::swap(l, r);
            int type = (int)rnd(0, 2);
            if (type == 0) {
                ll v = rnd(-100, 100);
                st.point_set(l, v);
                a[l - 1] = v;
            } else if (type == 1) {
                ll s = 0;
                for (int i = l; i <= r; i++) s += a[i - 1];
                if (st.range_sum(l, r) != s) { std::cout << "FAIL stretch sum n=" << n << "\n"; return 1; }
                checks++;
            } else {
                if (st.best_stretch(l, r) != kadane(a, l, r)) { std::cout << "FAIL stretch best n=" << n << "\n"; return 1; }
                checks++;
            }
        }
    }

    {
        LazySegTree st({3, -1, 4, 1, 5});
        bool ok = st.range_sum(1, 5) == 12 && st.range_min(1, 5) == -1;
        st.range_add(2, 4, 10);
        ok = ok && st.range_sum(1, 5) == 42 && st.range_min(1, 5) == 3;
        st.point_set(3, -2);
        ok = ok && st.range_sum(2, 4) == 18;
        StretchSegTree sk({-2, 1, -3, 4, -1});
        ok = ok && sk.best_stretch(1, 5) == 4;
        sk.point_set(5, 3);
        ok = ok && sk.best_stretch(1, 5) == 7;
        StretchSegTree neg({-5, -2, -9});
        ok = ok && neg.best_stretch(1, 3) == -2;
        if (!ok) { std::cout << "FAIL hand examples\n"; return 1; }
    }

    std::cout << "PASS: " << checks << " random queries + hand examples (seed " << SEED << ")\n";
    return 0;
}
