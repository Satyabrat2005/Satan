#ifndef PYTHON_BRIDGE_H
#define PYTHON_BRIDGE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include "satan_value.h"

class PythonBridge {
public:
    PythonBridge();
    ~PythonBridge();

    // Initialize session directory
    void initSession();

    // Check if Python is available
    bool checkPython();

    // Install dependencies
    bool installDependencies();

    // Generate unique Python variable name
    std::string newPyVar();

    // Add Python code to the session script
    void addCode(const std::string& code);

    // Add code and immediately execute, return captured output
    std::string executeImmediate(const std::string& code);

    // Execute accumulated session script and return output
    std::string flushAndExecute();

    // Parse return value from Python output
    SatanValue parseResult(const std::string& output);

    // Get session directory
    std::string getSessionDir() const { return sessionDir; }

    // Get next plot filename
    std::string nextPlotPath();

    // Python code generators for ML operations
    std::string genLoadCSV(const std::string& pyVar, const std::string& filepath);
    std::string genCreateModel(const std::string& pyVar, const std::string& modelType,
                               const std::vector<std::pair<std::string, std::string>>& kwargs = {});
    std::string genFitModel(const std::string& modelVar, const std::string& dataVar,
                            const std::vector<std::pair<std::string, std::string>>& kwargs = {});
    std::string genScore(const std::string& modelVar, const std::string& dataVar);
    std::string genPredict(const std::string& modelVar, const std::string& inputExpr);
    std::string genPlotModel(const std::string& modelVar, const std::string& dataVar, const std::string& plotPath);
    std::string genDescribe(const std::string& dataVar);
    std::string genHead(const std::string& dataVar, int n = 5);
    std::string genShape(const std::string& dataVar);
    std::string genCorrelation(const std::string& dataVar, const std::string& plotPath);
    std::string genPlotData(const std::string& dataVar, const std::string& plotPath, const std::string& kind = "auto");

    // Deep Learning generators
    std::string genNeuralNet(const std::string& pyVar, const std::vector<int>& layers);
    std::string genTrainNN(const std::string& modelVar, const std::string& dataVar,
                           int epochs = 100, double lr = 0.01);
    std::string genPlotLoss(const std::string& modelVar, const std::string& plotPath);

    // NLP generators
    std::string genSentiment(const std::string& text);
    std::string genTokenize(const std::string& text);
    std::string genWordCloud(const std::string& text, const std::string& plotPath);

    // Plotting generators
    std::string genScatter(const std::string& xExpr, const std::string& yExpr, const std::string& plotPath);
    std::string genLinePlot(const std::string& xExpr, const std::string& yExpr, const std::string& plotPath);
    std::string genHistogram(const std::string& dataExpr, const std::string& plotPath, int bins = 30);
    std::string genHeatmap(const std::string& dataExpr, const std::string& plotPath);
    std::string genBarChart(const std::string& labels, const std::string& values, const std::string& plotPath);
    std::string genConfusionMatrix(const std::string& yTrue, const std::string& yPred, const std::string& plotPath);

    // Feature 1: Hyperparameter Tuning
    std::string genTuneModel(const std::string& modelVar, const std::string& dataVar);

    // Feature 2: Advanced Visualizations
    std::string genFeatureImportance(const std::string& modelVar, const std::string& plotPath);
    std::string genROCCurve(const std::string& modelVar, const std::string& plotPath);
    std::string genLearningCurve(const std::string& modelVar, const std::string& plotPath);

    // Feature 4: Data Preprocessing
    std::string genFillMissing(const std::string& dataVar, const std::string& src);
    std::string genDropNulls(const std::string& dataVar, const std::string& src);
    std::string genEncodeText(const std::string& dataVar, const std::string& src);
    std::string genNormalize(const std::string& dataVar, const std::string& src);

    // Feature 5: Model Save / Load
    std::string genSaveModel(const std::string& modelVar, const std::string& filepath);
    std::string genLoadModel(const std::string& filepath, const std::string& newVar);

    // Feature 6: AutoML
    std::string genAutoML(const std::string& dataVar, const std::string& winnerVar);


private:
    std::string sessionDir;
    std::string plotDir;
    std::string sessionScript;
    int varCounter;
    int plotCounter;
    bool pythonChecked;
    bool pythonAvailable;

    // Helper to write and execute a Python file
    std::string writeTempAndRun(const std::string& script);

    // Helper to read a file
    std::string readFile(const std::string& path);

    // Helper to write a file
    void writeFile(const std::string& path, const std::string& content);

    // Get the Python runtime preamble
    std::string getPreamble();

    // Open an image file with the default viewer
    void openImage(const std::string& path);
};

#endif
