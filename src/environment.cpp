#include "../include/environment.h"
#include <stdexcept>
#include <iostream>

// ---------------- Numbers ----------------
void Environment::define(const std::string& name, double value) {
    values[name] = value;
    std::cout << "[env] variable " << name << " = " << value << std::endl;
}

double Environment::getNumber(const std::string& name) const {
    auto it = values.find(name);
    if (it != values.end() && std::holds_alternative<double>(it->second)) {
        return std::get<double>(it->second);
    }
    if (parent) return parent->getNumber(name);
    throw std::runtime_error("Undefined variable or not a number: " + name);
}

// ---------------- Functions ----------------
void Environment::defineFunction(const std::string& name, const FunctionObject& func) {
    values[name] = func;
    std::cout << "[env] function " << name << " defined with "
              << func.params.size() << " params" << std::endl;
}

FunctionObject Environment::getFunction(const std::string& name) const {
    auto it = values.find(name);
    if (it != values.end() && std::holds_alternative<FunctionObject>(it->second)) {
        return std::get<FunctionObject>(it->second);
    }
    throw std::runtime_error("Undefined function: " + name);
}

// ---------------- Helpers ----------------
bool Environment::exists(const std::string& name) const {
    return values.find(name) != values.end();
}
