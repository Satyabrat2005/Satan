cmake_minimum_required(VERSION 3.16)
project(Satan VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Main executable
add_executable(satan
    main.cpp
    src/lexer.cpp
    src/environment.cpp
    src/parser.cpp
    src/interpreter.cpp
    src/repl.cpp
    src/python_bridge.cpp
    src/stdlib_ml.cpp
    src/setup.cpp
)

target_include_directories(satan PRIVATE include)

# Copy examples directory
file(COPY examples DESTINATION ${CMAKE_BINARY_DIR})

# Test executable (environment tests)
add_executable(test_environment
    tests/test_enviroment.cpp
    src/environment.cpp
)
target_include_directories(test_environment PRIVATE include third_party)

message(STATUS "Satan v${PROJECT_VERSION} — AI/ML/DL/NLP Ready 🔱")
