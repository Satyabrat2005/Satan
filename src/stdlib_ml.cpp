#include "../include/stdlib_ml.h"
#include "../include/interpreter.h"
#include <iostream>
#include <algorithm>

// Helper: extract named argument value from args
static std::string getNamedArg(const std::vector<SatanValue>& args, const std::string& name, const std::string& defaultVal) {
    for (const auto& arg : args) {
        if (arg.isObject()) {
            auto nameIt = arg.object->find("__named_key__");
            if (nameIt != arg.object->end() && nameIt->second.str == name) {
                auto valIt = arg.object->find("__named_value__");
                if (valIt != arg.object->end()) return valIt->second.toString();
            }
        }
    }
    return defaultVal;
}

static int getNamedArgInt(const std::vector<SatanValue>& args, const std::string& name, int defaultVal) {
    std::string val = getNamedArg(args, name, "");
    if (val.empty()) return defaultVal;
    try { return std::stoi(val); } catch (...) { return defaultVal; }
}

static double getNamedArgDouble(const std::vector<SatanValue>& args, const std::string& name, double defaultVal) {
    std::string val = getNamedArg(args, name, "");
    if (val.empty()) return defaultVal;
    try { return std::stod(val); } catch (...) { return defaultVal; }
}

void registerMLBuiltins(Environment& env, PythonBridge& bridge) {
    // =================== Data Science ===================

    // load_csv(filepath)
    env.define("load_csv", SatanValue::makeNativeFn([&bridge](std::vector<SatanValue> args) -> SatanValue {
        if (args.empty() || !args[0].isString())
            throw std::runtime_error("load_csv() expects a string filepath argument.");

        if (!bridge.checkPython())
            throw std::runtime_error("Python not found! Run 'satan --setup-ml' to configure.");

        std::string filepath = args[0].str;
        std::string pyVar = bridge.newPyVar();
        std::string code = bridge.genLoadCSV(pyVar, filepath);
        std::string output = bridge.executeImmediate(code);
        std::cout << output;

        // Create ML object
        SatanValue obj = SatanValue::makeObject();
        obj.setProperty("__type__", SatanValue(std::string("DataFrame")));
        obj.setProperty("__pyvar__", SatanValue(pyVar));
        obj.setProperty("__source__", SatanValue(filepath));
        return obj;
    }));

    // read_csv - alias for load_csv
    env.define("read_csv", env.get("load_csv"));

    // =================== Machine Learning Models ===================

    auto makeModelCreator = [&bridge](const std::string& modelType) {
        return SatanValue::makeNativeFn([&bridge, modelType](std::vector<SatanValue> args) -> SatanValue {
            std::string pyVar = bridge.newPyVar();
            std::vector<std::pair<std::string, std::string>> kwargs;
            // Extract positional and named args
            for (const auto& arg : args) {
                if (arg.isNumber()) {
                    if (modelType == "KMeans") kwargs.push_back({"n_clusters", std::to_string((int)arg.number)});
                    else if (modelType == "PCA") kwargs.push_back({"n_components", std::to_string((int)arg.number)});
                    else if (modelType == "KNN") kwargs.push_back({"n_neighbors", std::to_string((int)arg.number)});
                }
            }

            SatanValue obj = SatanValue::makeObject();
            obj.setProperty("__type__", SatanValue(modelType));
            obj.setProperty("__pyvar__", SatanValue(pyVar));
            obj.setProperty("__kwargs__", SatanValue(std::string("")));
            obj.setProperty("__fitted__", SatanValue(false));

            std::string code = bridge.genCreateModel(pyVar, modelType, kwargs);
            obj.setProperty("__create_code__", SatanValue(code));

            std::cout << "Created " << modelType << " model" << std::endl;
            return obj;
        });
    };

    env.define("LinearRegression", makeModelCreator("LinearRegression"));
    env.define("LogisticRegression", makeModelCreator("LogisticRegression"));
    env.define("DecisionTree", makeModelCreator("DecisionTree"));
    env.define("RandomForest", makeModelCreator("RandomForest"));
    env.define("SVM", makeModelCreator("SVM"));
    env.define("KMeans", makeModelCreator("KMeans"));
    env.define("PCA", makeModelCreator("PCA"));
    env.define("KNN", makeModelCreator("KNN"));
    env.define("GradientBoosting", makeModelCreator("GradientBoosting"));
    env.define("XGBoost", makeModelCreator("XGBoost"));
    env.define("LightGBM", makeModelCreator("LightGBM"));

    // =================== Deep Learning ===================

    // NeuralNet([layer_sizes])
    env.define("NeuralNet", SatanValue::makeNativeFn([&bridge](std::vector<SatanValue> args) -> SatanValue {
        std::vector<int> layers;
        if (!args.empty() && args[0].isArray()) {
            for (const auto& elem : *args[0].array) {
                layers.push_back(static_cast<int>(elem.asNumber()));
            }
        } else {
            // Default network
            layers = {784, 128, 64, 10};
        }

        std::string pyVar = bridge.newPyVar();
        SatanValue obj = SatanValue::makeObject();
        obj.setProperty("__type__", SatanValue(std::string("NeuralNet")));
        obj.setProperty("__pyvar__", SatanValue(pyVar));
        obj.setProperty("__fitted__", SatanValue(false));

        std::string layerStr = "[";
        for (size_t i = 0; i < layers.size(); i++) {
            if (i > 0) layerStr += ", ";
            layerStr += std::to_string(layers[i]);
        }
        layerStr += "]";
        obj.setProperty("__layers__", SatanValue(layerStr));

        std::string code = bridge.genNeuralNet(pyVar, layers);
        obj.setProperty("__create_code__", SatanValue(code));

        std::cout << "Created NeuralNet " << layerStr << std::endl;
        return obj;
    }));

    // =================== NLP ===================

    // sentiment(text)
    env.define("sentiment", SatanValue::makeNativeFn([&bridge](std::vector<SatanValue> args) -> SatanValue {
        if (args.empty()) throw std::runtime_error("sentiment() expects a text argument.");
        std::string text = args[0].toString();
        std::string code = bridge.genSentiment(text);
        std::string output = bridge.executeImmediate(code);
        std::cout << output;
        return bridge.parseResult(output);
    }));

    // tokenize(text)
    env.define("tokenize", SatanValue::makeNativeFn([&bridge](std::vector<SatanValue> args) -> SatanValue {
        if (args.empty()) throw std::runtime_error("tokenize() expects a text argument.");
        std::string text = args[0].toString();
        std::string code = bridge.genTokenize(text);
        std::string output = bridge.executeImmediate(code);
        std::cout << output;
        return bridge.parseResult(output);
    }));

    // word_cloud(text)
    env.define("word_cloud", SatanValue::makeNativeFn([&bridge](std::vector<SatanValue> args) -> SatanValue {
        if (args.empty()) throw std::runtime_error("word_cloud() expects a text argument.");
        std::string text = args[0].toString();
        std::string plotPath = bridge.nextPlotPath();
        std::string code = bridge.genWordCloud(text, plotPath);
        std::string output = bridge.executeImmediate(code);
        std::cout << "\033[32m📊 Word cloud saved to: " << plotPath << "\033[0m" << std::endl;
        #ifdef _WIN32
        std::system(("start \"\" \"" + plotPath + "\"").c_str());
        #endif
        return SatanValue(plotPath);
    }));

    // =================== Plotting ===================

    // scatter(x_array, y_array)
    env.define("scatter", SatanValue::makeNativeFn([&bridge](std::vector<SatanValue> args) -> SatanValue {
        if (args.size() < 2) throw std::runtime_error("scatter() expects x and y arrays.");
        std::string plotPath = bridge.nextPlotPath();
        std::string xStr = args[0].toString(), yStr = args[1].toString();
        std::string code = bridge.genScatter(xStr, yStr, plotPath);
        bridge.executeImmediate(code);
        std::cout << "\033[32m📊 Scatter plot saved to: " << plotPath << "\033[0m" << std::endl;
        #ifdef _WIN32
        std::system(("start \"\" \"" + plotPath + "\"").c_str());
        #endif
        return SatanValue(plotPath);
    }));

    // histogram(data, bins?)
    env.define("histogram", SatanValue::makeNativeFn([&bridge](std::vector<SatanValue> args) -> SatanValue {
        if (args.empty()) throw std::runtime_error("histogram() expects data.");
        int bins = args.size() > 1 ? (int)args[1].asNumber() : 30;
        std::string plotPath = bridge.nextPlotPath();
        std::string code = bridge.genHistogram(args[0].toString(), plotPath, bins);
        bridge.executeImmediate(code);
        std::cout << "\033[32m📊 Histogram saved to: " << plotPath << "\033[0m" << std::endl;
        #ifdef _WIN32
        std::system(("start \"\" \"" + plotPath + "\"").c_str());
        #endif
        return SatanValue(plotPath);
    }));

    // =================== Utility ===================

    // len(array_or_string)
    env.define("len", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        if (args.empty()) return SatanValue(0.0);
        if (args[0].isArray()) return SatanValue(static_cast<double>(args[0].array->size()));
        if (args[0].isString()) return SatanValue(static_cast<double>(args[0].str.size()));
        return SatanValue(0.0);
    }));

    // type(value)
    env.define("type", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        if (args.empty()) return SatanValue(std::string("nil"));
        switch (args[0].type) {
            case ValueType::NIL: return SatanValue(std::string("nil"));
            case ValueType::NUMBER: return SatanValue(std::string("number"));
            case ValueType::STRING: return SatanValue(std::string("string"));
            case ValueType::BOOLEAN: return SatanValue(std::string("boolean"));
            case ValueType::ARRAY: return SatanValue(std::string("array"));
            case ValueType::OBJECT: {
                auto prop = args[0].getProperty("__type__");
                if (!prop.isNil()) return SatanValue(prop.str);
                return SatanValue(std::string("object"));
            }
            case ValueType::NATIVE_FN: return SatanValue(std::string("function"));
            case ValueType::FUNCTION: return SatanValue(std::string("function"));
        }
        return SatanValue(std::string("unknown"));
    }));

    // range(n) or range(start, end)
    env.define("range", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        int start = 0, end = 0;
        if (args.size() == 1) { end = (int)args[0].asNumber(); }
        else if (args.size() >= 2) { start = (int)args[0].asNumber(); end = (int)args[1].asNumber(); }
        std::vector<SatanValue> result;
        for (int i = start; i < end; i++) result.push_back(SatanValue((double)i));
        return SatanValue::makeArray(std::move(result));
    }));

    // abs, sqrt, pow, round, min, max
    env.define("abs", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        return SatanValue(std::abs(args[0].asNumber()));
    }));
    env.define("sqrt", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        return SatanValue(std::sqrt(args[0].asNumber()));
    }));
    env.define("pow", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        return SatanValue(std::pow(args[0].asNumber(), args[1].asNumber()));
    }));
    env.define("round", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        return SatanValue(std::round(args[0].asNumber()));
    }));
    env.define("min", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        if (args.size() >= 2) return SatanValue(std::min(args[0].asNumber(), args[1].asNumber()));
        if (args[0].isArray()) {
            double m = (*args[0].array)[0].asNumber();
            for (auto& v : *args[0].array) m = std::min(m, v.asNumber());
            return SatanValue(m);
        }
        return args[0];
    }));
    env.define("max", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        if (args.size() >= 2) return SatanValue(std::max(args[0].asNumber(), args[1].asNumber()));
        if (args[0].isArray()) {
            double m = (*args[0].array)[0].asNumber();
            for (auto& v : *args[0].array) m = std::max(m, v.asNumber());
            return SatanValue(m);
        }
        return args[0];
    }));

    // str(value), num(value)
    env.define("str", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        return SatanValue(args.empty() ? "" : args[0].toString());
    }));
    env.define("num", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        return SatanValue(args.empty() ? 0.0 : args[0].asNumber());
    }));

    // =================== Feature 5: load_model(path) ===================
    env.define("load_model", SatanValue::makeNativeFn([&bridge](std::vector<SatanValue> args) -> SatanValue {
        if (args.empty() || !args[0].isString())
            throw std::runtime_error("load_model() expects a filepath string.");
        std::string filepath = args[0].str;
        std::string newVar = bridge.newPyVar();
        std::string code = bridge.genLoadModel(filepath, newVar);
        std::string output = bridge.executeImmediate(code);
        std::cout << output;
        SatanValue obj = SatanValue::makeObject();
        obj.setProperty("__type__", SatanValue(std::string("RandomForest")));
        obj.setProperty("__pyvar__", SatanValue(newVar));
        obj.setProperty("__create_code__", SatanValue(std::string("")));
        obj.setProperty("__fitted__", SatanValue(true));
        return obj;
    }));

    // =================== Feature 6: AutoML ===================
    SatanValue automlObj = SatanValue::makeObject();
    automlObj.setProperty("__type__", SatanValue(std::string("AutoML")));
    automlObj.setProperty("__find_best__", SatanValue::makeNativeFn([&bridge](std::vector<SatanValue> args) -> SatanValue {
        if (args.empty() || !args[0].isObject())
            throw std::runtime_error("AutoML.find_best() requires a DataFrame.");
        std::string dataVar = args[0].getProperty("__pyvar__").str;
        std::string dataSrc = args[0].getProperty("__source__").str;
        std::string winnerVar = bridge.newPyVar();
        std::string loadCode = bridge.genLoadCSV(dataVar, dataSrc);
        std::string autoCode = bridge.genAutoML(dataVar, winnerVar);
        std::string output = bridge.executeImmediate(loadCode + autoCode);
        std::cout << output;
        SatanValue obj = SatanValue::makeObject();
        obj.setProperty("__type__", SatanValue(std::string("RandomForest")));
        obj.setProperty("__pyvar__", SatanValue(winnerVar));
        obj.setProperty("__create_code__", SatanValue(std::string("")));
        obj.setProperty("__fitted__", SatanValue(true));
        return obj;
    }));
    env.define("AutoML", automlObj);

    // =================== Phase 1: Native File I/O ===================
    env.define("read_file", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        if (args.empty() || !args[0].isString()) throw std::runtime_error("read_file() expects a file path.");
        std::ifstream file(args[0].str);
        if (!file.is_open()) throw std::runtime_error("Cannot open file: " + args[0].str);
        std::stringstream buf; buf << file.rdbuf();
        return SatanValue(buf.str());
    }));

    env.define("write_file", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        if (args.size() < 2) throw std::runtime_error("write_file(path, content) requires 2 arguments.");
        std::ofstream file(args[0].str);
        if (!file.is_open()) throw std::runtime_error("Cannot write to file: " + args[0].str);
        file << args[1].toString();
        file.close();
        return SatanValue(true);
    }));

    env.define("append_file", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        if (args.size() < 2) throw std::runtime_error("append_file(path, content) requires 2 arguments.");
        std::ofstream file(args[0].str, std::ios::app);
        if (!file.is_open()) throw std::runtime_error("Cannot append to file: " + args[0].str);
        file << args[1].toString();
        file.close();
        return SatanValue(true);
    }));

    env.define("file_exists", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        if (args.empty()) return SatanValue(false);
        std::ifstream file(args[0].str);
        return SatanValue(file.good());
    }));

    // =================== Phase 1: JSON Parse / Stringify ===================
    // Simple recursive JSON parser (handles objects, arrays, strings, numbers, booleans, null)
    std::function<SatanValue(const std::string&, size_t&)> parseJsonValue;
    auto skipWs = [](const std::string& s, size_t& i) { while (i < s.size() && isspace(s[i])) i++; };

    parseJsonValue = [&parseJsonValue, &skipWs](const std::string& s, size_t& i) -> SatanValue {
        skipWs(s, i);
        if (i >= s.size()) return SatanValue();

        if (s[i] == '"') {
            i++; // skip opening quote
            std::string str;
            while (i < s.size() && s[i] != '"') {
                if (s[i] == '\\' && i + 1 < s.size()) { i++; str += s[i]; }
                else str += s[i];
                i++;
            }
            if (i < s.size()) i++; // skip closing quote
            return SatanValue(str);
        }
        if (s[i] == '{') {
            i++; // skip {
            SatanValue obj = SatanValue::makeObject();
            skipWs(s, i);
            if (i < s.size() && s[i] == '}') { i++; return obj; }
            while (i < s.size()) {
                skipWs(s, i);
                SatanValue key = parseJsonValue(s, i);
                skipWs(s, i);
                if (i < s.size() && s[i] == ':') i++;
                SatanValue val = parseJsonValue(s, i);
                obj.setProperty(key.toString(), std::move(val));
                skipWs(s, i);
                if (i < s.size() && s[i] == ',') i++;
                else break;
            }
            if (i < s.size() && s[i] == '}') i++;
            return obj;
        }
        if (s[i] == '[') {
            i++;
            std::vector<SatanValue> arr;
            skipWs(s, i);
            if (i < s.size() && s[i] == ']') { i++; return SatanValue::makeArray({}); }
            while (i < s.size()) {
                arr.push_back(parseJsonValue(s, i));
                skipWs(s, i);
                if (i < s.size() && s[i] == ',') i++;
                else break;
            }
            if (i < s.size() && s[i] == ']') i++;
            return SatanValue::makeArray(std::move(arr));
        }
        if (s.substr(i, 4) == "true") { i += 4; return SatanValue(true); }
        if (s.substr(i, 5) == "false") { i += 5; return SatanValue(false); }
        if (s.substr(i, 4) == "null") { i += 4; return SatanValue(); }
        // number
        size_t start = i;
        if (s[i] == '-') i++;
        while (i < s.size() && (isdigit(s[i]) || s[i] == '.')) i++;
        if (i > start) return SatanValue(std::stod(s.substr(start, i - start)));
        throw std::runtime_error("Invalid JSON at position " + std::to_string(i));
    };

    env.define("json_parse", SatanValue::makeNativeFn([parseJsonValue](std::vector<SatanValue> args) mutable -> SatanValue {
        if (args.empty()) throw std::runtime_error("json_parse() expects a JSON string.");
        size_t i = 0;
        return parseJsonValue(args[0].toString(), i);
    }));

    // JSON stringify
    std::function<std::string(const SatanValue&)> jsonStringify;
    jsonStringify = [&jsonStringify](const SatanValue& val) -> std::string {
        if (val.isNil()) return "null";
        if (val.isBoolean()) return val.boolean ? "true" : "false";
        if (val.isNumber()) { std::ostringstream o; o << val.number; return o.str(); }
        if (val.isString()) return "\"" + val.str + "\"";
        if (val.isArray() && val.array) {
            std::string r = "[";
            for (size_t i = 0; i < val.array->size(); i++) {
                if (i > 0) r += ",";
                r += jsonStringify((*val.array)[i]);
            }
            return r + "]";
        }
        if (val.isObject() && val.object) {
            std::string r = "{";
            bool first = true;
            for (const auto& p : *val.object) {
                if (p.first.size() >= 2 && p.first[0] == '_' && p.first[1] == '_') continue;
                if (!first) r += ",";
                r += "\"" + p.first + "\":" + jsonStringify(p.second);
                first = false;
            }
            return r + "}";
        }
        return "null";
    };

    env.define("json_stringify", SatanValue::makeNativeFn([jsonStringify](std::vector<SatanValue> args) -> SatanValue {
        if (args.empty()) return SatanValue(std::string("null"));
        return SatanValue(jsonStringify(args[0]));
    }));

    // =================== Phase 1: Utility Functions ===================
    env.define("type_of", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        if (args.empty()) return SatanValue(std::string("nil"));
        switch (args[0].type) {
            case ValueType::NIL: return SatanValue(std::string("nil"));
            case ValueType::NUMBER: return SatanValue(std::string("number"));
            case ValueType::STRING: return SatanValue(std::string("string"));
            case ValueType::BOOLEAN: return SatanValue(std::string("boolean"));
            case ValueType::ARRAY: return SatanValue(std::string("array"));
            case ValueType::OBJECT: return SatanValue(std::string("object"));
            case ValueType::NATIVE_FN: return SatanValue(std::string("function"));
            case ValueType::FUNCTION: return SatanValue(std::string("function"));
        }
        return SatanValue(std::string("unknown"));
    }));

    env.define("len", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        if (args.empty()) return SatanValue(0.0);
        if (args[0].isString()) return SatanValue(static_cast<double>(args[0].str.size()));
        if (args[0].isArray()) return SatanValue(static_cast<double>(args[0].array ? args[0].array->size() : 0));
        return SatanValue(0.0);
    }));

    env.define("range", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        int start = 0, end = 0, step = 1;
        if (args.size() == 1) { end = (int)args[0].asNumber(); }
        else if (args.size() >= 2) { start = (int)args[0].asNumber(); end = (int)args[1].asNumber(); }
        if (args.size() >= 3) { step = (int)args[2].asNumber(); if (step == 0) step = 1; }
        std::vector<SatanValue> result;
        if (step > 0) { for (int i = start; i < end; i += step) result.push_back(SatanValue(static_cast<double>(i))); }
        else { for (int i = start; i > end; i += step) result.push_back(SatanValue(static_cast<double>(i))); }
        return SatanValue::makeArray(std::move(result));
    }));

    // Math functions
    env.define("sqrt", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        return SatanValue(std::sqrt(args.empty() ? 0.0 : args[0].asNumber()));
    }));
    env.define("abs", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        return SatanValue(std::abs(args.empty() ? 0.0 : args[0].asNumber()));
    }));
    env.define("floor", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        return SatanValue(std::floor(args.empty() ? 0.0 : args[0].asNumber()));
    }));
    env.define("ceil", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        return SatanValue(std::ceil(args.empty() ? 0.0 : args[0].asNumber()));
    }));
    env.define("round", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        return SatanValue(std::round(args.empty() ? 0.0 : args[0].asNumber()));
    }));
    env.define("pow", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        if (args.size() < 2) return SatanValue(0.0);
        return SatanValue(std::pow(args[0].asNumber(), args[1].asNumber()));
    }));
    env.define("max", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        if (args.size() < 2) return args.empty() ? SatanValue(0.0) : args[0];
        return SatanValue(std::max(args[0].asNumber(), args[1].asNumber()));
    }));
    env.define("min", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        if (args.size() < 2) return args.empty() ? SatanValue(0.0) : args[0];
        return SatanValue(std::min(args[0].asNumber(), args[1].asNumber()));
    }));

    // User input
    env.define("input", SatanValue::makeNativeFn([](std::vector<SatanValue> args) -> SatanValue {
        if (!args.empty()) std::cout << args[0].toString();
        std::string line;
        std::getline(std::cin, line);
        return SatanValue(line);
    }));

    // to_csv for DataFrames
    env.define("to_csv", SatanValue::makeNativeFn([&bridge](std::vector<SatanValue> args) -> SatanValue {
        if (args.size() < 2) throw std::runtime_error("to_csv(dataframe, path) requires 2 arguments.");
        std::string pyVar = args[0].getProperty("__pyvar__").str;
        std::string src = args[0].getProperty("__source__").str;
        std::string outPath = args[1].str;
        std::string code = bridge.genLoadCSV(pyVar, src);
        code += pyVar + ".to_csv('" + outPath + "', index=False)\n";
        code += "print('__SATAN_RESULT__:done')\n";
        std::string output = bridge.executeImmediate(code);
        std::cout << "  Saved to: " << outPath << std::endl;
        return SatanValue(true);
    }));
}

// =================== ML Method Call Handler ===================

SatanValue handleMethodCall(const SatanValue& object, const std::string& method,
                            const std::vector<SatanValue>& args, PythonBridge& bridge) {
    std::string objType = object.getProperty("__type__").str;
    std::string pyVar = object.getProperty("__pyvar__").str;

    // DataFrame methods
    if (objType == "DataFrame") {
        std::string src = object.getProperty("__source__").str;

        if (method == "head") {
            int n = args.empty() ? 5 : (int)args[0].asNumber();
            std::string code = bridge.genLoadCSV(pyVar, src) + bridge.genHead(pyVar, n);
            std::string out = bridge.executeImmediate(code);
            std::cout << out;
            return SatanValue(out);
        }
        if (method == "describe") {
            std::string code = bridge.genLoadCSV(pyVar, src) + bridge.genDescribe(pyVar);
            std::string out = bridge.executeImmediate(code);
            std::cout << out;
            return SatanValue(out);
        }
        if (method == "shape") {
            std::string code = bridge.genLoadCSV(pyVar, src) + bridge.genShape(pyVar);
            std::string out = bridge.executeImmediate(code);
            std::cout << out;
            return bridge.parseResult(out);
        }
        if (method == "corr") {
            std::string plotPath = bridge.nextPlotPath();
            std::string code = bridge.genLoadCSV(pyVar, src) + bridge.genCorrelation(pyVar, plotPath);
            bridge.executeImmediate(code);
            std::cout << "\033[32m📊 Correlation heatmap saved to: " << plotPath << "\033[0m" << std::endl;
            #ifdef _WIN32
            std::system(("start \"\" \"" + plotPath + "\"").c_str());
            #endif
            return SatanValue(plotPath);
        }
        if (method == "plot") {
            std::string plotPath = bridge.nextPlotPath();
            std::string code = bridge.genLoadCSV(pyVar, src) + bridge.genPlotData(pyVar, plotPath);
            bridge.executeImmediate(code);
            std::cout << "\033[32m📊 Data plot saved to: " << plotPath << "\033[0m" << std::endl;
            #ifdef _WIN32
            std::system(("start \"\" \"" + plotPath + "\"").c_str());
            #endif
            return SatanValue(plotPath);
        }
        // Feature 4: Data Preprocessing
        if (method == "fill_missing") {
            std::string code = bridge.genFillMissing(pyVar, src);
            std::string out = bridge.executeImmediate(code);
            std::cout << out;
            SatanValue newDf = SatanValue::makeObject();
            newDf.setProperty("__type__", SatanValue(std::string("DataFrame")));
            newDf.setProperty("__pyvar__", SatanValue(pyVar));
            newDf.setProperty("__source__", SatanValue(src));
            return newDf;
        }
        if (method == "drop_nulls") {
            std::string code = bridge.genDropNulls(pyVar, src);
            std::string out = bridge.executeImmediate(code);
            std::cout << out;
            return bridge.parseResult(out);
        }
        if (method == "encode") {
            std::string code = bridge.genEncodeText(pyVar, src);
            std::string out = bridge.executeImmediate(code);
            std::cout << out;
            SatanValue newDf = SatanValue::makeObject();
            newDf.setProperty("__type__", SatanValue(std::string("DataFrame")));
            newDf.setProperty("__pyvar__", SatanValue(pyVar));
            newDf.setProperty("__source__", SatanValue(src));
            return newDf;
        }
        if (method == "normalize") {
            std::string code = bridge.genNormalize(pyVar, src);
            std::string out = bridge.executeImmediate(code);
            std::cout << out;
            SatanValue newDf = SatanValue::makeObject();
            newDf.setProperty("__type__", SatanValue(std::string("DataFrame")));
            newDf.setProperty("__pyvar__", SatanValue(pyVar));
            newDf.setProperty("__source__", SatanValue(src));
            return newDf;
        }
    }

    // ML Model methods (sklearn + boosting)
    if (objType == "LinearRegression" || objType == "LogisticRegression" ||
        objType == "DecisionTree" || objType == "RandomForest" ||
        objType == "SVM" || objType == "KMeans" || objType == "PCA" ||
        objType == "KNN" || objType == "GradientBoosting" ||
        objType == "XGBoost" || objType == "LightGBM") {

        if (method == "fit") {
            if (args.empty()) throw std::runtime_error(objType + ".fit() requires data argument.");
            // First arg should be a DataFrame
            std::string dataVar = "";
            std::string dataSrc = "";
            if (args[0].isObject()) {
                dataVar = args[0].getProperty("__pyvar__").str;
                dataSrc = args[0].getProperty("__source__").str;
            }
            if (dataSrc.empty()) throw std::runtime_error(objType + ".fit() requires a DataFrame.");

            std::string createCode = object.getProperty("__create_code__").str;
            std::string loadCode = bridge.genLoadCSV(dataVar, dataSrc);
            std::string fitCode = bridge.genFitModel(pyVar, dataVar);
            std::string fullCode = loadCode + createCode + fitCode;
            std::string output = bridge.executeImmediate(fullCode);
            std::cout << output;
            return bridge.parseResult(output);
        }

        if (method == "score") {
            std::string code = bridge.genScore(pyVar, "");
            std::string output = bridge.executeImmediate(code);
            std::cout << output;
            return bridge.parseResult(output);
        }

        if (method == "predict") {
            std::string inputStr = args.empty() ? "_data['X_test']" : args[0].toString();
            std::string code = bridge.genPredict(pyVar, inputStr);
            std::string output = bridge.executeImmediate(code);
            std::cout << output;
            return bridge.parseResult(output);
        }

        if (method == "plot") {
            std::string plotPath = bridge.nextPlotPath();
            std::string code = bridge.genPlotModel(pyVar, "", plotPath);
            bridge.executeImmediate(code);
            std::cout << "\033[32m📊 Model plot saved to: " << plotPath << "\033[0m" << std::endl;
            #ifdef _WIN32
            std::system(("start \"\" \"" + plotPath + "\"").c_str());
            #endif
            return SatanValue(plotPath);
        }
        // Feature 1: Hyperparameter Tuning
        if (method == "tune") {
            if (args.empty()) throw std::runtime_error(objType + ".tune() requires data argument.");
            std::string dataVar = args[0].getProperty("__pyvar__").str;
            std::string dataSrc = args[0].getProperty("__source__").str;
            std::string createCode = object.getProperty("__create_code__").str;
            std::string loadCode = bridge.genLoadCSV(dataVar, dataSrc);
            std::string tuneCode = bridge.genTuneModel(pyVar, dataVar);
            std::string output = bridge.executeImmediate(loadCode + createCode + tuneCode);
            std::cout << output;
            return bridge.parseResult(output);
        }
        // Feature 2: Advanced Visualizations
        if (method == "feature_importance") {
            std::string plotPath = bridge.nextPlotPath();
            std::string code = bridge.genFeatureImportance(pyVar, plotPath);
            bridge.executeImmediate(code);
            std::cout << "\033[32m📊 Feature importance saved to: " << plotPath << "\033[0m" << std::endl;
            #ifdef _WIN32
            std::system(("start \"\" \"" + plotPath + "\"").c_str());
            #endif
            return SatanValue(plotPath);
        }
        if (method == "roc_curve") {
            std::string plotPath = bridge.nextPlotPath();
            std::string code = bridge.genROCCurve(pyVar, plotPath);
            bridge.executeImmediate(code);
            std::cout << "\033[32m📊 ROC curve saved to: " << plotPath << "\033[0m" << std::endl;
            #ifdef _WIN32
            std::system(("start \"\" \"" + plotPath + "\"").c_str());
            #endif
            return SatanValue(plotPath);
        }
        if (method == "learning_curve") {
            std::string plotPath = bridge.nextPlotPath();
            std::string code = bridge.genLearningCurve(pyVar, plotPath);
            bridge.executeImmediate(code);
            std::cout << "\033[32m📊 Learning curve saved to: " << plotPath << "\033[0m" << std::endl;
            #ifdef _WIN32
            std::system(("start \"\" \"" + plotPath + "\"").c_str());
            #endif
            return SatanValue(plotPath);
        }
        // Feature 5: Save
        if (method == "save") {
            std::string filepath = args.empty() ? "model.smodel" : args[0].str;
            // normalize path separators
            std::replace(filepath.begin(), filepath.end(), '\\', '/');
            std::string code = bridge.genSaveModel(pyVar, filepath);
            std::string output = bridge.executeImmediate(code);
            std::cout << output;
            return SatanValue(filepath);
        }
    }

    // NeuralNet methods
    if (objType == "NeuralNet") {
        if (method == "train") {
            if (args.empty() || !args[0].isObject())
                throw std::runtime_error("NeuralNet.train() requires a DataFrame.");

            std::string dataVar = args[0].getProperty("__pyvar__").str;
            std::string dataSrc = args[0].getProperty("__source__").str;
            int epochs = 100;
            double lr = 0.01;

            // Check for named args in remaining args
            for (size_t i = 1; i < args.size(); i++) {
                if (args[i].isNumber()) {
                    if (i == 1) epochs = (int)args[i].number;
                    if (i == 2) lr = args[i].number;
                }
            }
            epochs = getNamedArgInt(args, "epochs", epochs);
            lr = getNamedArgDouble(args, "lr", lr);

            std::string createCode = object.getProperty("__create_code__").str;
            std::string loadCode = bridge.genLoadCSV(dataVar, dataSrc);
            std::string trainCode = bridge.genTrainNN(pyVar, dataVar, epochs, lr);
            std::string fullCode = loadCode + createCode + trainCode;
            std::string output = bridge.executeImmediate(fullCode);
            std::cout << output;
            return bridge.parseResult(output);
        }

        if (method == "plot_loss") {
            std::string plotPath = bridge.nextPlotPath();
            std::string code = bridge.genPlotLoss(pyVar, plotPath);
            bridge.executeImmediate(code);
            std::cout << "\033[32m📊 Loss plot saved to: " << plotPath << "\033[0m" << std::endl;
            #ifdef _WIN32
            std::system(("start \"\" \"" + plotPath + "\"").c_str());
            #endif
            return SatanValue(plotPath);
        }
    }

    // AutoML method dispatch
    if (objType == "AutoML") {
        if (method == "find_best") {
            if (args.empty() || !args[0].isObject())
                throw std::runtime_error("AutoML.find_best() requires a DataFrame.");
            std::string dataVar = args[0].getProperty("__pyvar__").str;
            std::string dataSrc = args[0].getProperty("__source__").str;
            std::string winnerVar = bridge.newPyVar();
            std::string loadCode = bridge.genLoadCSV(dataVar, dataSrc);
            std::string autoCode = bridge.genAutoML(dataVar, winnerVar);
            std::string output = bridge.executeImmediate(loadCode + autoCode);
            std::cout << output;
            SatanValue obj = SatanValue::makeObject();
            obj.setProperty("__type__", SatanValue(std::string("RandomForest")));
            obj.setProperty("__pyvar__", SatanValue(winnerVar));
            obj.setProperty("__create_code__", SatanValue(std::string("")));
            obj.setProperty("__fitted__", SatanValue(true));
            return obj;
        }
    }

    throw std::runtime_error("Unknown method '" + method + "' on " + objType);
}

// =================== ML Property Access Handler ===================

SatanValue handlePropertyAccess(const SatanValue& object, const std::string& property,
                                PythonBridge& bridge) {
    std::string objType = object.getProperty("__type__").str;

    // DataFrame property shortcuts
    if (objType == "DataFrame") {
        if (property == "X" || property == "features") {
            // Return a reference marker that fit() can use
            SatanValue ref = SatanValue::makeObject();
            ref.setProperty("__type__", SatanValue(std::string("DataRef")));
            ref.setProperty("__parent__", SatanValue(object.getProperty("__source__").str));
            ref.setProperty("__column__", SatanValue(std::string("X")));
            return ref;
        }
        if (property == "y" || property == "target") {
            SatanValue ref = SatanValue::makeObject();
            ref.setProperty("__type__", SatanValue(std::string("DataRef")));
            ref.setProperty("__parent__", SatanValue(object.getProperty("__source__").str));
            ref.setProperty("__column__", SatanValue(std::string("y")));
            return ref;
        }
        if (property == "columns" || property == "shape") {
            std::string src = object.getProperty("__source__").str;
            std::string pyVar = object.getProperty("__pyvar__").str;
            std::string code = bridge.genLoadCSV(pyVar, src);
            if (property == "columns") code += "print(list(" + pyVar + ".columns))\n";
            else code += bridge.genShape(pyVar);
            std::string out = bridge.executeImmediate(code);
            std::cout << out;
            return bridge.parseResult(out);
        }
    }

    // Pass through to object properties
    return object.getProperty(property);
}
