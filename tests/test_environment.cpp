#define CATCH_CONFIG_MAIN
#include "../third_party/catch.hpp"
#include "../include/environment.h"

// Include parser.h for BlockStmt (used by FunctionObject::body).
// We provide no-op stubs for all virtual methods so we can link without
// pulling in the full parser/lexer/interpreter translation units.
#include "../include/parser.h"

// --------------- Stmt stubs ---------------
void BlockStmt::execute(Environment&) const {}
void VarDecl::execute(Environment&) const {}
void AssembleStmt::execute(Environment&) const {}
void PrintStmt::execute(Environment&) const {}
void IfStmt::execute(Environment&) const {}
void ExprStmt::execute(Environment&) const {}
void SummonStmt::execute(Environment&) const {}
void FunDecl::execute(Environment&) const {}
void ReturnStmt::execute(Environment&) const {}
void WhileStmt::execute(Environment&) const {}
void ForStmt::execute(Environment&) const {}
void BreakStmt::execute(Environment&) const {}
void ContinueStmt::execute(Environment&) const {}

// --------------- Expr stubs ---------------
void LiteralExpr::print() const {}
double LiteralExpr::evaluate(Environment&) const { return 0; }
void VariableExpr::print() const {}
double VariableExpr::evaluate(Environment&) const { return 0; }
void BinaryExpr::print() const {}
double BinaryExpr::evaluate(Environment&) const { return 0; }
void CallExpr::print() const {}
double CallExpr::evaluate(Environment&) const { return 0; }
void LogicalExpr::print() const {}
double LogicalExpr::evaluate(Environment&) const { return 0; }
void UnaryExpr::print() const {}
double UnaryExpr::evaluate(Environment&) const { return 0; }

// --------------- Token stub ---------------
Token::Token(TokenType t, std::string lex, int ln)
    : type(t), lexeme(std::move(lex)), line(ln) {}

// =============================================================================
// Variable Definition Tests
// =============================================================================

TEST_CASE("Environment defines and retrieves a variable", "[environment][variable]") {
    Environment env;
    env.define("x", 42.0);

    REQUIRE(env.exists("x"));
    REQUIRE(env.getNumber("x") == 42.0);
}

TEST_CASE("Environment defines multiple variables", "[environment][variable]") {
    Environment env;
    env.define("a", 1.0);
    env.define("b", 2.0);
    env.define("c", 3.0);

    REQUIRE(env.getNumber("a") == 1.0);
    REQUIRE(env.getNumber("b") == 2.0);
    REQUIRE(env.getNumber("c") == 3.0);
}

TEST_CASE("Environment overwrites a variable with a new value", "[environment][variable]") {
    Environment env;
    env.define("x", 10.0);
    REQUIRE(env.getNumber("x") == 10.0);

    env.define("x", 20.0);
    REQUIRE(env.getNumber("x") == 20.0);
}

TEST_CASE("Environment throws on undefined variable lookup", "[environment][variable]") {
    Environment env;
    REQUIRE_THROWS_AS(env.getNumber("nonexistent"), std::runtime_error);
    REQUIRE_THROWS_WITH(env.getNumber("nonexistent"),
                        Catch::Contains("Undefined variable or not a number"));
}

TEST_CASE("Environment exists returns false for undefined variable", "[environment][variable]") {
    Environment env;
    REQUIRE_FALSE(env.exists("ghost"));
}

// =============================================================================
// Function Definition Tests
// =============================================================================

TEST_CASE("Environment defines and retrieves a function", "[environment][function]") {
    Environment env;

    FunctionObject func;
    func.params = {"a", "b"};
    func.body = std::make_shared<BlockStmt>(std::vector<std::unique_ptr<Stmt>>{});

    env.defineFunction("add", func);

    REQUIRE(env.exists("add"));

    FunctionObject retrieved = env.getFunction("add");
    REQUIRE(retrieved.params.size() == 2);
    REQUIRE(retrieved.params[0] == "a");
    REQUIRE(retrieved.params[1] == "b");
}

TEST_CASE("Environment defines a zero-parameter function", "[environment][function]") {
    Environment env;

    FunctionObject func;
    func.params = {};
    func.body = std::make_shared<BlockStmt>(std::vector<std::unique_ptr<Stmt>>{});

    env.defineFunction("noop", func);

    FunctionObject retrieved = env.getFunction("noop");
    REQUIRE(retrieved.params.empty());
}

TEST_CASE("Environment throws on undefined function lookup", "[environment][function]") {
    Environment env;
    REQUIRE_THROWS_AS(env.getFunction("missing"), std::runtime_error);
    REQUIRE_THROWS_WITH(env.getFunction("missing"),
                        Catch::Contains("Undefined function"));
}

TEST_CASE("Environment getNumber throws when name is a function", "[environment][function]") {
    Environment env;

    FunctionObject func;
    func.params = {"x"};
    func.body = std::make_shared<BlockStmt>(std::vector<std::unique_ptr<Stmt>>{});

    env.defineFunction("myFunc", func);

    // "myFunc" exists but is not a number
    REQUIRE(env.exists("myFunc"));
    REQUIRE_THROWS_AS(env.getNumber("myFunc"), std::runtime_error);
}

TEST_CASE("Environment getFunction throws when name is a variable", "[environment][function]") {
    Environment env;
    env.define("pi", 3.14);

    REQUIRE(env.exists("pi"));
    REQUIRE_THROWS_AS(env.getFunction("pi"), std::runtime_error);
}

// =============================================================================
// Scope Creation Tests
// =============================================================================

TEST_CASE("Child scope inherits parent variables", "[environment][scope]") {
    Environment parent;
    parent.define("x", 100.0);

    Environment child(&parent);

    REQUIRE(child.exists("x"));
    REQUIRE(child.getNumber("x") == 100.0);
}

TEST_CASE("Child scope can define its own variables", "[environment][scope]") {
    Environment parent;
    Environment child(&parent);

    child.define("local_var", 55.0);

    REQUIRE(child.exists("local_var"));
    REQUIRE(child.getNumber("local_var") == 55.0);
    REQUIRE_FALSE(parent.exists("local_var"));
}

TEST_CASE("Child scope shadows parent variable", "[environment][scope]") {
    Environment parent;
    parent.define("x", 1.0);

    Environment child(&parent);
    child.define("x", 2.0);

    REQUIRE(child.getNumber("x") == 2.0);
    REQUIRE(parent.getNumber("x") == 1.0);
}

TEST_CASE("Default constructed environment has no parent", "[environment][scope]") {
    Environment env;

    REQUIRE_FALSE(env.exists("anything"));
    REQUIRE_THROWS_AS(env.getNumber("anything"), std::runtime_error);
}

// =============================================================================
// Nested Scope Variable/Function Lookup Tests
// =============================================================================

TEST_CASE("Deeply nested scope lookup walks ancestor chain", "[environment][nested]") {
    Environment grandparent;
    grandparent.define("level", 0.0);

    Environment parent(&grandparent);
    parent.define("middle", 1.0);

    Environment child(&parent);

    REQUIRE(child.exists("level"));
    REQUIRE(child.getNumber("level") == 0.0);

    REQUIRE(child.exists("middle"));
    REQUIRE(child.getNumber("middle") == 1.0);
}

TEST_CASE("Nested scope function lookup walks ancestor chain", "[environment][nested]") {
    Environment grandparent;

    FunctionObject func;
    func.params = {"n"};
    func.body = std::make_shared<BlockStmt>(std::vector<std::unique_ptr<Stmt>>{});

    grandparent.defineFunction("factorial", func);

    Environment parent(&grandparent);
    Environment child(&parent);

    REQUIRE(child.exists("factorial"));

    FunctionObject retrieved = child.getFunction("factorial");
    REQUIRE(retrieved.params.size() == 1);
    REQUIRE(retrieved.params[0] == "n");
}

TEST_CASE("Nested scope shadows function from ancestor", "[environment][nested]") {
    Environment grandparent;

    FunctionObject origFunc;
    origFunc.params = {"a"};
    origFunc.body = std::make_shared<BlockStmt>(std::vector<std::unique_ptr<Stmt>>{});
    grandparent.defineFunction("compute", origFunc);

    Environment child(&grandparent);

    FunctionObject newFunc;
    newFunc.params = {"a", "b", "c"};
    newFunc.body = std::make_shared<BlockStmt>(std::vector<std::unique_ptr<Stmt>>{});
    child.defineFunction("compute", newFunc);

    REQUIRE(child.getFunction("compute").params.size() == 3);
    REQUIRE(grandparent.getFunction("compute").params.size() == 1);
}

TEST_CASE("Undefined variable throws even in nested scope", "[environment][nested]") {
    Environment grandparent;
    Environment parent(&grandparent);
    Environment child(&parent);

    REQUIRE_FALSE(child.exists("missing"));
    REQUIRE_THROWS_AS(child.getNumber("missing"), std::runtime_error);
    REQUIRE_THROWS_AS(child.getFunction("missing"), std::runtime_error);
}

TEST_CASE("Mixed variables and functions in nested scopes", "[environment][nested]") {
    Environment global;
    global.define("PI", 3.14159);

    FunctionObject areaFunc;
    areaFunc.params = {"r"};
    areaFunc.body = std::make_shared<BlockStmt>(std::vector<std::unique_ptr<Stmt>>{});
    global.defineFunction("circleArea", areaFunc);

    Environment local(&global);
    local.define("radius", 5.0);

    // local scope can see both the variable and function from parent
    REQUIRE(local.exists("PI"));
    REQUIRE(local.getNumber("PI") == Approx(3.14159));
    REQUIRE(local.exists("circleArea"));
    REQUIRE(local.getFunction("circleArea").params[0] == "r");

    // local scope has its own variable
    REQUIRE(local.getNumber("radius") == 5.0);

    // parent does not see child's variable
    REQUIRE_FALSE(global.exists("radius"));
}

TEST_CASE("Inner scope shadows outer and outer unchanged after inner exits", "[environment][nested]") {
    Environment outer;
    outer.define("x", 10.0);

    REQUIRE(outer.exists("x"));
    REQUIRE(outer.getNumber("x") == 10.0);

    // Enter an inner scope that shadows "x"
    {
        Environment inner(&outer);
        inner.define("x", 99.0);

        // Inner scope sees its own shadowed value
        REQUIRE(inner.getNumber("x") == 99.0);

        // Outer scope is still unchanged while inner is alive
        REQUIRE(outer.getNumber("x") == 10.0);
    }
    // Inner scope has exited (destroyed)

    // Outer scope variable remains unchanged after inner scope exit
    REQUIRE(outer.exists("x"));
    REQUIRE(outer.getNumber("x") == 10.0);
}
TEST_CASE("exists walks the full ancestor chain", "[environment][nested]") {
    Environment level0;
    level0.define("root_var", 0.0);

    Environment level1(&level0);
    Environment level2(&level1);
    Environment level3(&level2);
    Environment level4(&level3);

    REQUIRE(level4.exists("root_var"));
    REQUIRE(level4.getNumber("root_var") == 0.0);
}
