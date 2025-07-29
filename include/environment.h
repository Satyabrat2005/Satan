#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <string>
#include <unordered_map>

class Environment {
private:
    std::unordered_map<std::string, double> values;

public:
    void define(const std::string& name, double value);
    double get(const std::string& name) const;
};

#endif
