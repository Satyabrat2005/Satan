#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <stdexcept>
#include "satan_value.h"

class Environment {
private:
    std::unordered_map<std::string, SatanValue> values;
    Environment* parent;

public:
    Environment() : parent(nullptr) {}
    explicit Environment(Environment* parentEnv) : parent(parentEnv) {}

    void define(const std::string& name, SatanValue value) {
        values[name] = std::move(value);
    }

    // Legacy overload for doubles
    void define(const std::string& name, double value) {
        values[name] = SatanValue(value);
    }

    void assign(const std::string& name, SatanValue value) {
        auto it = values.find(name);
        if (it != values.end()) { it->second = std::move(value); return; }
        if (parent) { parent->assign(name, std::move(value)); return; }
        throw std::runtime_error("Undefined variable: " + name);
    }

    SatanValue get(const std::string& name) const {
        auto it = values.find(name);
        if (it != values.end()) return it->second;
        if (parent) return parent->get(name);
        throw std::runtime_error("Undefined variable: " + name);
    }

    // Legacy compatibility
    double getNumber(const std::string& name) const {
        SatanValue val = get(name);
        if (val.isNumber()) return val.number;
        if (val.isBoolean()) return val.boolean ? 1.0 : 0.0;
        throw std::runtime_error("Undefined variable or not a number: " + name);
    }

    void defineFunction(const std::string& name, const FunctionObject& func) {
        define(name, SatanValue::makeFunction(func));
    }

    FunctionObject getFunction(const std::string& name) const {
        SatanValue val = get(name);
        if (val.isFunction() && val.function) return *val.function;
        if (val.isNativeFn()) throw std::runtime_error("Cannot get native function as FunctionObject: " + name);
        throw std::runtime_error("Undefined function: " + name);
    }

    bool exists(const std::string& name) const {
        if (values.find(name) != values.end()) return true;
        if (parent) return parent->exists(name);
        return false;
    }
};

#endif
