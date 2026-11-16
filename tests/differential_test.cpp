#include "oracle.hpp"
#include "invariants.hpp"
#include "workload.hpp"
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace sensor;
using namespace sensor::testing;

namespace {

struct Options {
    bool runA = true, runB = true, quick = false;
    std::size_t mainOps = 100000;      // the brief requires >= 10^5 operations
    std::uint64_t baseSeed = 203003;   // fixed; every config uses baseSeed + its index
    std::size_t checkEvery = 500;      // invariant check period (operations)
    std::string outDir = "build";
};

struct Config {
    Mode mode;
    Index n;
    Profile profile;
    std::size_t ops;
    std::uint64_t seed;
};

struct Totals {
    int configs = 0, failed = 0;
    unsigned long long opsChecked = 0, invariantChecks = 0;
    bool anyWhiteBox = false;
};

const char* modeName(Mode m) { return m == Mode::Lazy ? "LAZY" : "STRETCH"; }

std::string describe(const Config& c) {
    std::ostringstream s;
    s << "mode=" << modeName(c.mode) << " n=" << c.n << " profile=" << profileName(c.profile)
      << " seed=" << c.seed << " ops=" << c.ops;
    return s.str();
}

bool apply(RangeAnalytics& b, const Op& op, Value& answer) {
    switch (op.type) {
        case 'U': b.pointSet(op.a, op.x); return false;
        case 'A': b.rangeAdd(op.a, op.b, op.x); return false;
        case 'S': answer = b.rangeSum(op.a, op.b); return true;
        case 'M': answer = b.rangeMin(op.a, op.b); return true;
        default:  answer = b.bestStretch(op.a, op.b); return true;
    }
}

void writeRepro(const Options& opt, const Config& c, const Workload& w, std::size_t failingOp) {
    std::ostringstream name;
    name << opt.outDir << "/fail_" << modeName(c.mode) << "_n" << c.n << "_" << profileName(c.profile)
         << "_seed" << c.seed << "_op" << failingOp;
    std::ofstream in(name.str() + ".in");
    if (!in) { std::cerr << "  (could not write repro file; does '" << opt.outDir << "' exist?)\n"; return; }
    writeWorkload(in, w, failingOp);  // ops 1..failingOp
    std::cerr << "  repro file: " << name.str() << ".in  (replay: build/run --structure A < that file)\n";
}

bool fail(const Options& opt, const Config& c, const Workload& w, std::size_t opNo,
          const std::string& who, const std::string& what) {
    std::cerr << "\n[FAIL] " << describe(c) << "\n"
              << "  structure : " << who << "\n"
              << "  operation : #" << opNo << "  (" << opText(w.ops[opNo - 1]) << ")\n"
              << "  problem   : " << what << "\n";
    writeRepro(opt, c, w, opNo);
    return false;
}

bool runConfig(const Options& opt, const Config& c, Totals& tot) {
    Workload w = makeWorkload(c.mode, c.n, c.ops, c.profile, c.seed);
    Oracle oracle(w.init);

    struct Sub { std::string name; std::unique_ptr<RangeAnalytics> impl; };
    std::vector<Sub> subs;
    try {
        if (opt.runA) subs.push_back({"A", makeBackend(Structure::A, c.mode, w.init)});
        if (opt.runB && c.mode == Mode::Lazy) subs.push_back({"B", makeBackend(Structure::B, c.mode, w.init)});
    } catch (const std::exception& e) {
        std::cerr << "\n[FAIL] " << describe(c) << "\n  could not construct a backend: " << e.what() << "\n";
        return false;
    }

    if (subs.empty()) return true;  // nothing selected supports this mode (reported as SKIP)

    Rng sampler(c.seed ^ 0x9e3779b97f4a7c15ULL);
    for (std::size_t k = 0; k < w.ops.size(); ++k) {
        const Op& op = w.ops[k];
        Value expected = 0;
        bool isQuery;
        try { isQuery = apply(oracle, op, expected); }
        catch (const std::exception& e) { return fail(opt, c, w, k + 1, "ORACLE", e.what()); }

        for (std::size_t s = 0; s < subs.size(); ++s) {
            Value got = 0;
            try { apply(*subs[s].impl, op, got); }
            catch (const std::exception& e) { return fail(opt, c, w, k + 1, subs[s].name, std::string("threw: ") + e.what()); }
            if (isQuery && got != expected)
                return fail(opt, c, w, k + 1, subs[s].name,
                            "expected " + std::to_string(expected) + ", got " + std::to_string(got));
        }
        tot.opsChecked += subs.size();  // one comparison per structure per operation

        if ((k + 1) % opt.checkEvery == 0 || k + 1 == w.ops.size()) {
            for (std::size_t s = 0; s < subs.size(); ++s) {
                bool insp = false;
                std::string e = whiteBoxCheck(*subs[s].impl, &insp);
                if (insp) tot.anyWhiteBox = true;
                if (e.empty()) {
                    try { e = blackBoxCheck(*subs[s].impl, c.mode, oracle.values(), sampler); }
                    catch (const std::exception& ex) { e = std::string("threw during invariant check: ") + ex.what(); }
                }
                ++tot.invariantChecks;
                if (!e.empty()) return fail(opt, c, w, k + 1, subs[s].name, "INVARIANT VIOLATED: " + e);
            }
        }
    }
    return true;
}

std::vector<Config> buildSuite(const Options& opt) {
    std::vector<Config> suite;
    std::uint64_t seed = opt.baseSeed;
    const Mode modes[] = {Mode::Lazy, Mode::Stretch};

    const Index small[] = {1, 2, 3, 5, 8, 17, 64, 101};
    for (Mode m : modes)
        for (Index n : small) {
            suite.push_back({m, n, Profile::Mixed, 3000, seed++});
            suite.push_back({m, n, Profile::Edges, 3000, seed++});
        }
    if (opt.quick) return suite;

    const Profile profiles[] = {Profile::Mixed, Profile::Short, Profile::Long, Profile::Edges,
                                Profile::LazyThenSet, Profile::BigValues};
    for (Mode m : modes) {
        for (Profile p : profiles) suite.push_back({m, 1000, p, opt.mainOps, seed++});
        suite.push_back({m, 1024, Profile::Mixed, opt.mainOps, seed++});
        suite.push_back({m, 997, Profile::Edges, opt.mainOps, seed++});
    }
    suite.push_back({Mode::Stretch, 1000, Profile::AllNegative, opt.mainOps, seed++});
    suite.push_back({Mode::Stretch, 37, Profile::AllNegative, 20000, seed++});

    for (Mode m : modes) {
        suite.push_back({m, 20000, Profile::Mixed, 20000, seed++});
        suite.push_back({m, 20000, Profile::Edges, 20000, seed++});
    }
    return suite;
}

} // namespace

int main(int argc, char** argv) {
    Options opt;
    try {
        for (int i = 1; i < argc; ++i) {
            std::string a = argv[i];
            auto next = [&]() -> std::string {
                if (i + 1 >= argc) throw std::invalid_argument("missing value for " + a);
                return argv[++i];
            };
            if (a == "--only") {
                std::string v = next();
                if (v != "A" && v != "B") throw std::invalid_argument("--only must be A or B");
                opt.runA = (v == "A");
                opt.runB = (v == "B");
            }
            else if (a == "--quick") opt.quick = true;
            else if (a == "--ops") opt.mainOps = std::stoul(next());
            else if (a == "--seed") opt.baseSeed = std::stoull(next());
            else throw std::invalid_argument("unknown argument " + a);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 2;
    }

    std::vector<Config> suite = buildSuite(opt);
    Totals tot;
    std::cout << "Differential test: base seed " << opt.baseSeed << ", structures:"
              << (opt.runA ? " A" : "") << (opt.runB ? " B" : "") << ", invariant check every "
              << opt.checkEvery << " ops\n";
    for (const Config& c : suite) {
        if (c.mode == Mode::Stretch && !opt.runA) {
            std::cout << "[SKIP] " << describe(c) << "  (only Structure A supports STRETCH)\n";
            continue;
        }
        ++tot.configs;
        bool ok = runConfig(opt, c, tot);
        if (ok) std::cout << "[PASS] " << describe(c) << "\n";
        else { ++tot.failed; break; }  // stop at the first failure
    }
    std::cout << "\nconfigs run: " << tot.configs << ", failed: " << tot.failed
              << ", structure-operations compared with the oracle: " << tot.opsChecked
              << ", invariant checks: " << tot.invariantChecks
              << ", white-box selfCheck available: " << (tot.anyWhiteBox ? "yes" : "no") << "\n";
    std::cout << (tot.failed == 0 ? "RESULT: ALL PASSED\n" : "RESULT: FAILED\n");
    return tot.failed == 0 ? 0 : 1;
}
