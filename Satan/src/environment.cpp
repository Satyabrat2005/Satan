#include "../include/environment.h"
#include <stdexcept>
#include <iostream>

// Define or overwrite a variable in the environment
void Environment::define(const std::string& name, double value) {
    values[name] = value;
    std::cout << "[env] " << name << " defined as " << value << std::endl;
}

// Retrieve a variable's value, or throw a runtime error if undefined
double Environment::get(const std::string& name) const {
    auto it = values.find(name);
    if (it != values.end()) {
        return it->second;
    }

    std::cerr << "[env error] Undefined variable: " << name << std::endl;
    throw std::runtime_error("Undefined variable: " + name);
}
