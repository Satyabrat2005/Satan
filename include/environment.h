#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <string>
#include <unordered_map>
#include <variant>
#include <memory>
#include <vector>

// Forward declarations
class BlockStmt;

struct FunctionObject {
    std::vector<std::string> params;
    std::shared_ptr<BlockStmt> body;
};

class Environment {
private:
    std::unordered_map<std::string, std::variant<double, FunctionObject>> values;
    Environment* parent;

public:
    Environment() : parent(nullptr) {}
    explicit Environment(Environment* parentEnv) : parent(parentEnv) {}
    
    void define(const std::string& name, double value);
    void defineFunction(const std::string& name, const FunctionObject& func);

    double getNumber(const std::string& name) const;
    FunctionObject getFunction(const std::string& name) const;

    bool exists(const std::string& name) const;
};

#endif
