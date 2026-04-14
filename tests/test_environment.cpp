#include "../include/environment.h"
#include <iostream>
#include <cassert>


int main() {
    // ---- Test: Nested scope variable shadowing ----
    // Define a variable in the outer scope
    Environment outer;
    outer.define("x", 10.0);

    // Verify the outer scope has the variable
    assert(outer.exists("x"));
    assert(outer.getNumber("x") == 10.0);

    // Create an inner (child) scope that shadows the variable
    Environment inner(&outer);
    inner.define("x", 99.0);

    // The inner scope should see its own shadowed value
    assert(inner.exists("x"));
    assert(inner.getNumber("x") == 99.0);

    // The outer scope should still hold the original value (unchanged)
    assert(outer.exists("x"));
    assert(outer.getNumber("x") == 10.0);

    // ---- Test: Inner scope can access outer variable when not shadowed ----
    outer.define("y", 42.0);
    assert(inner.getNumber("y") == 42.0);

    // ---- Test: Variable defined only in inner scope is not visible in outer ----
    inner.define("z", 7.0);
    assert(inner.exists("z"));
    assert(!outer.exists("z"));

    std::cout << "All nested scope shadowing tests passed!\n";
    return 0;
}
