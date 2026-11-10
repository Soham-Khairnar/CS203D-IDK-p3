#include "runner.hpp"
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace sensor {
namespace {
[[noreturn]] void fail(std::size_t line, const std::string& message) {
    throw std::invalid_argument("line " + std::to_string(line) + ": " + message);
}

void endOfLine(std::istringstream& row, std::size_t line) {
    std::string extra;
    if (row >> extra) fail(line, "unexpected extra token: " + extra);
}

Value number(std::istringstream& row, std::size_t line) {
    Value value;
    if (!(row >> value)) fail(line, "expected a signed 64-bit integer");
    return value;
}

Index index(std::istringstream& row, std::size_t line, Index n) {
    const Value value = number(row, line);
    if (value < 1 || value > static_cast<Value>(n)) {
        fail(line, "index must be in 1.." + std::to_string(n));
    }
    return static_cast<Index>(value);
}
}

void run(std::istream& input, std::ostream& output, Structure structure,
         const BackendFactory& factory) {
    std::string text;
    if (!std::getline(input, text)) fail(1, "missing mode and array size");
    std::istringstream header(text);
    std::string modeName;
    header >> modeName;
    if (modeName != "LAZY" && modeName != "STRETCH") {
        fail(1, "mode must be LAZY or STRETCH");
    }
    const Mode mode = modeName == "LAZY" ? Mode::Lazy : Mode::Stretch;
    const Value size = number(header, 1);
    endOfLine(header, 1);
    if (size < 1 || size > 1000000) fail(1, "array size must be in 1..1000000");
    if (structure == Structure::B && mode == Mode::Stretch) {
        fail(1, "STRETCH mode requires Structure A");
    }
    const Index n = static_cast<Index>(size);

    if (!std::getline(input, text)) fail(2, "missing initial readings");
    std::istringstream readings(text);
    std::vector<Value> initial;
    initial.reserve(n);
    for (Index i = 0; i < n; ++i) initial.push_back(number(readings, 2));
    endOfLine(readings, 2);
    auto backend = factory(structure, mode, initial);
    if (!backend) throw std::runtime_error("backend factory returned null");

    std::size_t line = 2;
    while (std::getline(input, text)) {
        ++line;
        std::istringstream row(text);
        std::string command;
        if (!(row >> command)) continue; // Allow blank lines between operations.
        if (command == "U") {
            const Index i = index(row, line, n);
            const Value value = number(row, line);
            endOfLine(row, line);
            backend->pointSet(i, value);
        } else if (command == "A" || command == "S" || command == "M" || command == "B") {
            if ((command == "A" || command == "M") && mode != Mode::Lazy) {
                fail(line, command + " is only supported in LAZY mode");
            }
            if (command == "B" && mode != Mode::Stretch) {
                fail(line, "B is only supported in STRETCH mode");
            }
            const Index left = index(row, line, n);
            const Index right = index(row, line, n);
            if (left > right) fail(line, "range must satisfy left <= right");
            if (command == "A") {
                const Value delta = number(row, line);
                endOfLine(row, line);
                backend->rangeAdd(left, right, delta);
            } else {
                endOfLine(row, line);
                Value answer;
                if (command == "S") answer = backend->rangeSum(left, right);
                else if (command == "M") answer = backend->rangeMin(left, right);
                else answer = backend->bestStretch(left, right);
                output << answer << '\n';
                if (!output) throw std::runtime_error("could not write query result");
            }
        } else {
            fail(line, "unknown command: " + command);
        }
    }
    if (input.bad() || (input.fail() && !input.eof())) {
        throw std::runtime_error("could not read operation file");
    }
}
} // namespace sensor
