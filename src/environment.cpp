#include "../include/environment.h"
#include <stdexcept>
#include <iostream>

// ---------------- Numbers ----------------
void Environment::define(const std::string& name, double value) {
    // Allow variable shadowing: a variable in a nested scope can have
    // the same name as a variable in an outer scope, hiding the outer one.
    // Only the current scope's map is written to, so the outer variable
    // remains untouched and becomes visible again once this scope exits.
    if (parent && parent->exists(name)) {
        std::cout << "[env] variable " << name << " shadows outer variable" << std::endl;
    }
    values[name] = value;
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
}

FunctionObject Environment::getFunction(const std::string& name) const {
    auto it = values.find(name);
    if (it != values.end() && std::holds_alternative<FunctionObject>(it->second)) {
        return std::get<FunctionObject>(it->second);
    }
    if (parent) return parent->getFunction(name);
    throw std::runtime_error("Undefined function: " + name);
}

// ---------------- Helpers ----------------
bool Environment::exists(const std::string& name) const {
    if (values.find(name) != values.end()) return true;
    if (parent) return parent->exists(name);
    return false;
}
