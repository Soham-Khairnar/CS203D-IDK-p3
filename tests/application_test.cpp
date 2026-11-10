#include "../app/runner.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
using namespace sensor;
int checks = 0;

void require(bool condition, const std::string& message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}

// Test double ONLY: verifies app dispatch and formatting before real backends exist.
// This is not a production backend or the Test owner's differential test suite.
class RecordingArray final : public RangeAnalytics {
    std::vector<Value> values;
public:
    explicit RecordingArray(const std::vector<Value>& initial) : values(initial) {}
    void pointSet(Index i, Value value) override { values.at(i - 1) = value; }
    void rangeAdd(Index l, Index r, Value delta) override {
        for (Index i = l; i <= r; ++i) values.at(i - 1) += delta;
    }
    Value rangeSum(Index l, Index r) override {
        Value sum = 0;
        for (Index i = l; i <= r; ++i) sum += values.at(i - 1);
        return sum;
    }
    Value rangeMin(Index l, Index r) override {
        return *std::min_element(values.begin() + l - 1, values.begin() + r);
    }
    Value bestStretch(Index l, Index r) override {
        Value ending = values.at(l - 1), best = ending;
        for (Index i = l + 1; i <= r; ++i) {
            ending = std::max(values.at(i - 1), ending + values.at(i - 1));
            best = std::max(best, ending);
        }
        return best;
    }
};

std::string execute(const std::string& text, Structure structure = Structure::A) {
    std::istringstream input(text);
    std::ostringstream output;
    bool created = false;
    run(input, output, structure,
        [&](Structure actual, Mode mode, const std::vector<Value>& initial) {
            require(actual == structure, "incorrect structure passed to factory");
            require(mode == (text.find("STRETCH") == 0 ? Mode::Stretch : Mode::Lazy),
                    "incorrect mode passed to factory");
            created = true;
            return std::make_unique<RecordingArray>(initial);
        });
    require(created, "factory was not called");
    return output.str();
}

void rejects(const std::string& text, const std::string& fragment,
             Structure structure = Structure::A) {
    bool rejected = false;
    try { execute(text, structure); }
    catch (const std::invalid_argument& error) {
        rejected = true;
        require(std::string(error.what()).find(fragment) != std::string::npos,
                "unexpected diagnostic: " + std::string(error.what()));
    }
    require(rejected, "invalid input was accepted: " + text);
}
}

int main() {
    try {
        const std::string lazy = "LAZY 5\n1 2 3 4 5\nS 1 5\nA 2 4 10\nM 1 5\nU 1 -5\nS 1 3\n";
        require(execute(lazy) == "15\n1\n20\n", "LAZY A dispatch");
        require(execute(lazy, Structure::B) == "15\n1\n20\n", "LAZY B dispatch");
        require(execute("LAZY 2\n1 2\nA 1 2 10\nU 1 -5\nS 1 2\nM 1 2\n") == "7\n-5\n",
                "point assignment after pending addition");
        require(execute("STRETCH 5\n-4 3 -1 5 -8\nB 1 5\nS 1 5\nU 2 -6\nB 1 3\n") == "7\n-5\n-1\n",
                "STRETCH dispatch and nonempty result");
        require(execute("STRETCH 3\n-8 -2 -5\nB 1 3\nB 1 1\n") == "-2\n-8\n",
                "all-negative range");
        require(execute("LAZY 1\r\n3000000000\r\n\r\nS 1 1\r\nA 1 1 -5\r\nM 1 1") == "3000000000\n2999999995\n",
                "64-bit values, CRLF, blanks, EOF without newline");
        require(execute("LAZY 1\n0\nU 1 4\nA 1 1 3\n").empty(), "updates printed output");
        require(execute("LAZY 1\n0\n").empty(), "no-command file");
        rejects("", "line 1");
        rejects("OTHER 1\n0\n", "mode must");
        rejects("LAZY 0\n", "array size");
        rejects("LAZY 1000001\n", "array size");
        rejects("LAZY 1 extra\n0\n", "extra token");
        rejects("LAZY 1\n", "line 2");
        rejects("LAZY 2\n1\n", "line 2");
        rejects("LAZY 1\n1 2\n", "extra token");
        rejects("LAZY 1\n9223372036854775808\n", "64-bit integer");
        rejects("STRETCH 1\n0\n", "requires Structure A", Structure::B);
        rejects("LAZY 1\n0\nB 1 1\n", "STRETCH mode");
        rejects("STRETCH 1\n0\nM 1 1\n", "LAZY mode");
        rejects("STRETCH 1\n0\nA 1 1 2\n", "LAZY mode");
        rejects("LAZY 2\n1 2\nS 2 1\n", "left <= right");
        rejects("LAZY 1\n0\nU 0 3\n", "index must");
        rejects("LAZY 1\n0\nS 1 2\n", "index must");
        rejects("LAZY 1\n0\nU -1 3\n", "index must");
        rejects("LAZY 1\n0\nU 1\n", "64-bit integer");
        rejects("LAZY 1\n0\nA 1 1\n", "64-bit integer");
        rejects("LAZY 1\n0\nS 1 1 extra\n", "extra token");
        rejects("LAZY 1\n0\nX 1 1\n", "unknown command");
        rejects("LAZY 1\n0\nS 1.5 1\n", "64-bit integer");

        // Exercise parsing at the assignment's maximum n, using the test double.
        std::string large = "LAZY 1000000\n";
        for (int i = 0; i < 1000000; ++i) large += "1 ";
        large += "\nS 1 1000000\nU 1000000 -2\nS 999999 1000000\n";
        require(execute(large) == "1000000\n-1\n", "maximum-size input");
        std::cout << "Application tests passed (" << checks << " checks).\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Application test failed: " << error.what() << '\n';
        return 1;
    }
}
