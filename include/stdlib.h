#ifndef STDLIB_ML_H
#define STDLIB_ML_H

#include "satan_value.h"
#include "environment.h"
#include "python_bridge.h"

// Register all built-in AI/ML/DL/NLP/DS functions into the environment
void registerMLBuiltins(Environment& env, PythonBridge& bridge);

// Handle method calls on ML objects (DataFrames, Models, etc.)
SatanValue handleMethodCall(const SatanValue& object, const std::string& method,
                            const std::vector<SatanValue>& args, PythonBridge& bridge);

// Handle property access on ML objects
SatanValue handlePropertyAccess(const SatanValue& object, const std::string& property,
                                PythonBridge& bridge);

#endif
