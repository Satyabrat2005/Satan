#include "../include/environment.h"
#include <stdexcept>
#include <iostream>

void Environment::define(const std::string& name, double value) {
    values[name] = value;
    std::cout << "[env] " << name << " defined as " << value << std::endl;
}

double Environment::get(const std::string& name) const {
    auto it = values.find(name);
    if (it != values.end()) {
        return it->second;
    }

    std::cerr << "[env error] Undefined variable: " << name << std::endl;
    throw std::runtime_error("Undefined variable: " + name);
}
