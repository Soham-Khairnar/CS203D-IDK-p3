#pragma once
#include <string>

namespace sensor {

// Optional white-box hook for the Test owner's invariant checker.
// A backend that wants its internals checked inherits from this (in addition to
// RangeAnalytics) and returns "" when every internal invariant holds, or a short
// human-readable description of the FIRST violation found.
// Backends that do not inherit from it are still checked through the public API.
class Inspectable {
public:
    virtual ~Inspectable() = default;
    virtual std::string selfCheck() const = 0;
};

} // namespace sensor
