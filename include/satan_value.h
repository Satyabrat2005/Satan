#ifndef SATAN_VALUE_H
#define SATAN_VALUE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <sstream>
#include <cmath>
#include <iostream>
#include <stdexcept>

// Forward declarations
class BlockStmt;
class Environment;

enum class ValueType {
    NIL, NUMBER, STRING, BOOLEAN, ARRAY, OBJECT, NATIVE_FN, FUNCTION
};

struct FunctionObject {
    std::vector<std::string> params;
    std::shared_ptr<BlockStmt> body;
};

class SatanValue;
using NativeFn = std::function<SatanValue(std::vector<SatanValue>)>;

class SatanValue {
public:
    ValueType type;
    double number;
    std::string str;
    bool boolean;
    std::shared_ptr<std::vector<SatanValue>> array;
    std::shared_ptr<std::unordered_map<std::string, SatanValue>> object;
    std::shared_ptr<NativeFn> nativeFn;
    std::shared_ptr<FunctionObject> function;

    // Default: nil
    SatanValue() : type(ValueType::NIL), number(0), boolean(false) {}
    // Number
    explicit SatanValue(double n) : type(ValueType::NUMBER), number(n), boolean(false) {}
    // String
    explicit SatanValue(const std::string& s) : type(ValueType::STRING), number(0), str(s), boolean(false) {}
    // Bool
    explicit SatanValue(bool b) : type(ValueType::BOOLEAN), number(0), boolean(b) {}

    // Factory methods
    static SatanValue makeArray(std::vector<SatanValue> elements) {
        SatanValue v;
        v.type = ValueType::ARRAY;
        v.array = std::make_shared<std::vector<SatanValue>>(std::move(elements));
        return v;
    }

    static SatanValue makeObject() {
        SatanValue v;
        v.type = ValueType::OBJECT;
        v.object = std::make_shared<std::unordered_map<std::string, SatanValue>>();
        return v;
    }

    static SatanValue makeNativeFn(NativeFn fn) {
        SatanValue v;
        v.type = ValueType::NATIVE_FN;
        v.nativeFn = std::make_shared<NativeFn>(std::move(fn));
        return v;
    }

    static SatanValue makeFunction(FunctionObject func) {
        SatanValue v;
        v.type = ValueType::FUNCTION;
        v.function = std::make_shared<FunctionObject>(std::move(func));
        return v;
    }

    // Type checks
    bool isNil() const { return type == ValueType::NIL; }
    bool isNumber() const { return type == ValueType::NUMBER; }
    bool isString() const { return type == ValueType::STRING; }
    bool isBoolean() const { return type == ValueType::BOOLEAN; }
    bool isArray() const { return type == ValueType::ARRAY; }
    bool isObject() const { return type == ValueType::OBJECT; }
    bool isNativeFn() const { return type == ValueType::NATIVE_FN; }
    bool isFunction() const { return type == ValueType::FUNCTION; }
    bool isCallable() const { return isNativeFn() || isFunction(); }

    double asNumber() const {
        if (type == ValueType::NUMBER) return number;
        if (type == ValueType::BOOLEAN) return boolean ? 1.0 : 0.0;
        if (type == ValueType::STRING) {
            try { return std::stod(str); } catch (...) { return 0.0; }
        }
        return 0.0;
    }

    bool isTruthy() const {
        switch (type) {
            case ValueType::NIL: return false;
            case ValueType::BOOLEAN: return boolean;
            case ValueType::NUMBER: return number != 0.0;
            case ValueType::STRING: return !str.empty();
            case ValueType::ARRAY: return array && !array->empty();
            case ValueType::OBJECT: return true;
            case ValueType::NATIVE_FN: return true;
            case ValueType::FUNCTION: return true;
        }
        return false;
    }

    std::string toString() const {
        switch (type) {
            case ValueType::NIL: return "nil";
            case ValueType::NUMBER: {
                if (number == std::floor(number) && std::abs(number) < 1e15 && std::abs(number) >= 1.0) {
                    long long i = static_cast<long long>(number);
                    return std::to_string(i);
                }
                std::ostringstream oss;
                oss << number;
                return oss.str();
            }
            case ValueType::STRING: return str;
            case ValueType::BOOLEAN: return boolean ? "true" : "false";
            case ValueType::ARRAY: {
                std::string result = "[";
                if (array) {
                    for (size_t i = 0; i < array->size(); i++) {
                        if (i > 0) result += ", ";
                        if ((*array)[i].isString()) result += "\"" + (*array)[i].str + "\"";
                        else result += (*array)[i].toString();
                    }
                }
                return result + "]";
            }
            case ValueType::OBJECT: {
                if (object) {
                    auto it = object->find("__type__");
                    if (it != object->end()) return "<" + it->second.str + ">";
                }
                return "<object>";
            }
            case ValueType::NATIVE_FN: return "<native fn>";
            case ValueType::FUNCTION: return "<function>";
        }
        return "nil";
    }

    // Object property access
    SatanValue getProperty(const std::string& name) const {
        if (type == ValueType::OBJECT && object) {
            auto it = object->find(name);
            if (it != object->end()) return it->second;
        }
        return SatanValue();
    }

    void setProperty(const std::string& name, SatanValue val) {
        if (type != ValueType::OBJECT || !object) {
            type = ValueType::OBJECT;
            object = std::make_shared<std::unordered_map<std::string, SatanValue>>();
        }
        (*object)[name] = std::move(val);
    }
};

#endif
