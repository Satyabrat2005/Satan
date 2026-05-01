#include "../include/setup.h"
#include "../include/python_bridge.h"
#include <iostream>
#include <cstdlib>

void printVersion() {
    std::cout << "\033[1;31m";
    std::cout << R"(
  ____    _  _____  _    _   _
 / ___|  / \|_   _|/ \  | \ | |
 \___ \ / _ \ | | / _ \ |  \| |
  ___) / ___ \| |/ ___ \| |\  |
 |____/_/   \_\_/_/   \_\_| \_|
    )" << std::endl;
    std::cout << "\033[0m";
    std::cout << "\033[1;36m Satan Programming Language v2.0.0\033[0m" << std::endl;
    std::cout << "\033[90m Built for AI/ML/DL/NLP/Data Science\033[0m" << std::endl;
    std::cout << std::endl;
    std::cout << " Modules:" << std::endl;
    std::cout << "   \033[32m✓\033[0m satan.core    — Variables, functions, control flow" << std::endl;
    std::cout << "   \033[32m✓\033[0m satan.data    — DataFrames, CSV, statistics" << std::endl;
    std::cout << "   \033[32m✓\033[0m satan.ml      — Linear/Logistic Regression, Trees, SVM, KNN" << std::endl;
    std::cout << "   \033[32m✓\033[0m satan.nn      — Neural Networks (PyTorch backend)" << std::endl;
    std::cout << "   \033[32m✓\033[0m satan.nlp     — Sentiment, tokenization, word clouds" << std::endl;
    std::cout << "   \033[32m✓\033[0m satan.plot    — Scatter, histogram, heatmaps" << std::endl;
    std::cout << std::endl;
}

void checkDependencies() {
    std::cout << "\033[1;36m=== Satan Dependency Check ===\033[0m" << std::endl;

    PythonBridge bridge;

    // Check Python
    bool hasPython = bridge.checkPython();
    std::cout << (hasPython ? " \033[32m✓\033[0m" : " \033[31m✗\033[0m") << " Python" << std::endl;

    if (!hasPython) {
        std::cout << "\n\033[31m Python is required for AI/ML features.\033[0m" << std::endl;
        std::cout << " Install from: https://python.org" << std::endl;
        return;
    }

    // Check Python packages
    const char* packages[] = {
        "numpy", "pandas", "sklearn", "matplotlib", "seaborn",
        "torch", "nltk", "wordcloud", nullptr
    };

    for (int i = 0; packages[i]; i++) {
        std::string cmd = "python -c \"import " + std::string(packages[i]) + "\" > nul 2>&1";
        int result = std::system(cmd.c_str());
        std::cout << (result == 0 ? " \033[32m✓\033[0m " : " \033[31m✗\033[0m ")
                  << packages[i] << std::endl;
    }
    std::cout << std::endl;
}

void installMLDependencies() {
    std::cout << "\033[1;36m=== Installing Satan ML Dependencies ===\033[0m" << std::endl;
    std::cout << std::endl;

    PythonBridge bridge;
    if (!bridge.checkPython()) {
        std::cout << "\033[31m Python not found! Install Python first.\033[0m" << std::endl;
        std::cout << " Install from: https://python.org" << std::endl;
        return;
    }

    bool allGood = true;

    // Step 1: Core packages
    std::cout << " [1/3] Installing core packages..." << std::endl;
    int r1 = std::system("python -m pip install numpy pandas scikit-learn matplotlib seaborn wordcloud Pillow nltk");
    if (r1 != 0) {
        std::cout << " Retrying with pip3..." << std::endl;
        r1 = std::system("python3 -m pip install numpy pandas scikit-learn matplotlib seaborn wordcloud Pillow nltk");
    }
    if (r1 != 0) {
        std::cout << "\033[33m  Warning: Some core packages may have failed to install.\033[0m" << std::endl;
        allGood = false;
    } else {
        std::cout << "\033[32m  Core packages installed.\033[0m" << std::endl;
    }

    // Step 2: PyTorch
    std::cout << std::endl;
    std::cout << " [2/3] Installing PyTorch (CPU)..." << std::endl;
    int r2 = std::system("python -m pip install torch --index-url https://download.pytorch.org/whl/cpu");
    if (r2 != 0) {
        r2 = std::system("python3 -m pip install torch --index-url https://download.pytorch.org/whl/cpu");
    }
    if (r2 != 0) {
        std::cout << "\033[33m  Warning: PyTorch install failed. Deep learning features may not work.\033[0m" << std::endl;
        allGood = false;
    } else {
        std::cout << "\033[32m  PyTorch installed.\033[0m" << std::endl;
    }

    // Step 3: NLTK data — only if nltk is importable
    std::cout << std::endl;
    std::cout << " [3/3] Downloading NLTK data..." << std::endl;
    int nltkCheck = std::system("python -c \"import nltk\" > nul 2>&1");
    if (nltkCheck != 0) nltkCheck = std::system("python3 -c \"import nltk\" > nul 2>&1");
    if (nltkCheck == 0) {
        std::system("python -c \"import nltk; nltk.download('vader_lexicon', quiet=True); nltk.download('punkt_tab', quiet=True)\"");
        std::cout << "\033[32m  NLTK data downloaded.\033[0m" << std::endl;
    } else {
        std::cout << "\033[31m  NLTK not found! NLP features will not work.\033[0m" << std::endl;
        std::cout << "  Try manually: pip install nltk" << std::endl;
        allGood = false;
    }

    std::cout << std::endl;
    if (allGood) {
        std::cout << "\033[32m All dependencies installed successfully! 🔱\033[0m" << std::endl;
    } else {
        std::cout << "\033[33m Setup completed with warnings. Some features may not work.\033[0m" << std::endl;
        std::cout << " Run 'satan --check' to see which packages are missing." << std::endl;
    }
}
