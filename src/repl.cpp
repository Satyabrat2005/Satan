#include "../include/repl.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include <iostream>
#include <string>
#include <sstream>

Repl::Repl() : interpreter() {}

void Repl::run() {
    std::cout << "\033[1;31m";
    std::cout << R"(
  ____    _  _____  _    _   _
 / ___|  / \|_   _|/ \  | \ | |
 \___ \ / _ \ | | / _ \ |  \| |
  ___) / ___ \| |/ ___ \| |\  |
 |____/_/   \_\_/_/   \_\_| \_|
    )" << std::endl;
    std::cout << "\033[0m";
    std::cout << "\033[1;36m Satan REPL v2.0.0 — AI/ML/DL/NLP Ready \033[0m" << std::endl;
    std::cout << "\033[90m Type 'exit' or 'quit' to leave. \033[0m" << std::endl;
    std::cout << std::endl;

    std::string line;

    while (true) {
        std::cout << "\033[1;31msatan>\033[0m ";
        std::cout.flush();

        if (!std::getline(std::cin, line)) {
            std::cout << std::endl;
            break;
        }

        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        std::string trimmed = line.substr(start);
        size_t end = trimmed.find_last_not_of(" \t\r\n");
        if (end != std::string::npos) trimmed = trimmed.substr(0, end + 1);

        if (trimmed == "exit" || trimmed == "quit") {
            std::cout << "\033[31mGoodbye! 🔱\033[0m" << std::endl;
            break;
        }

        if (trimmed == "help") {
            std::cout << "\033[1;36m=== Satan Built-in Functions ===\033[0m" << std::endl;
            std::cout << "\033[33m Data Science:\033[0m  load_csv(), .head(), .describe(), .shape(), .corr(), .plot()" << std::endl;
            std::cout << "\033[33m ML Models:\033[0m     LinearRegression(), LogisticRegression(), DecisionTree()," << std::endl;
            std::cout << "               RandomForest(), SVM(), KMeans(), PCA(), KNN()" << std::endl;
            std::cout << "\033[33m Deep Learning:\033[0m NeuralNet([layers]), .train(), .plot_loss()" << std::endl;
            std::cout << "\033[33m NLP:\033[0m           sentiment(), tokenize(), word_cloud()" << std::endl;
            std::cout << "\033[33m Plotting:\033[0m      scatter(), histogram()" << std::endl;
            std::cout << "\033[33m Math:\033[0m          abs(), sqrt(), pow(), round(), min(), max()" << std::endl;
            std::cout << "\033[33m Utility:\033[0m       len(), type(), range(), str(), num()" << std::endl;
            continue;
        }

        if (trimmed.empty()) continue;

        try {
            Lexer lexer(line);
            auto tokens = lexer.scanTokens();

            Parser parser(tokens);
            auto statements = parser.parse();

            if (statements.empty()) {
                std::cerr << "[error] Could not parse input." << std::endl;
                continue;
            }

            interpreter.interpret(statements);

        } catch (const std::runtime_error& e) {
            std::cerr << "\033[31m[error]\033[0m " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "\033[31m[error]\033[0m " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "\033[31m[error]\033[0m An unknown error occurred." << std::endl;
        }
    }
}
