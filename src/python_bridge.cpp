#include "../include/python_bridge.h"
#include <cstdio>
#include <array>
#include <algorithm>
#include <filesystem>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#define popen _popen
#define pclose _pclose
#endif

namespace fs = std::filesystem;

PythonBridge::PythonBridge()
    : varCounter(0), plotCounter(0), pythonChecked(false), pythonAvailable(false) {
    // Create plot output directory in current working directory
    plotDir = fs::current_path().string() + "/satan_plots";
    std::replace(plotDir.begin(), plotDir.end(), '\\', '/');
    fs::create_directories(plotDir);
}

PythonBridge::~PythonBridge() {
    // Clean up temp session directory (scripts, pickles) but NOT plots
    try {
        if (!sessionDir.empty() && fs::exists(sessionDir)) {
            fs::remove_all(sessionDir);
        }
    } catch (...) {}
}

void PythonBridge::initSession() {
    const char* tmp = std::getenv("TEMP");
    if (!tmp) tmp = std::getenv("TMP");
    if (!tmp) tmp = ".";
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    sessionDir = std::string(tmp) + "/satan_session_" + std::to_string(now);
    // Replace backslashes with forward slashes for Python compatibility
    std::replace(sessionDir.begin(), sessionDir.end(), '\\', '/');
    fs::create_directories(sessionDir);
}

bool PythonBridge::checkPython() {
    if (pythonChecked) return pythonAvailable;
    pythonChecked = true;
    int result = std::system("python --version > nul 2>&1");
    if (result == 0) { pythonAvailable = true; return true; }
    result = std::system("python3 --version > /dev/null 2>&1");
    pythonAvailable = (result == 0);
    return pythonAvailable;
}

bool PythonBridge::installDependencies() {
    std::cout << "[Satan ML] Installing Python dependencies..." << std::endl;
    int result = std::system("pip install numpy pandas scikit-learn matplotlib seaborn nltk wordcloud Pillow 2>&1");
    if (result != 0) {
        result = std::system("pip3 install numpy pandas scikit-learn matplotlib seaborn nltk wordcloud Pillow 2>&1");
    }
    return result == 0;
}

std::string PythonBridge::newPyVar() {
    return "__s" + std::to_string(varCounter++);
}

void PythonBridge::addCode(const std::string& code) {
    sessionScript += code + "\n";
}

std::string PythonBridge::nextPlotPath() {
    if (sessionDir.empty()) initSession();
    // Save plots to ./satan_plots/ with descriptive timestamped names
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    std::string name = "plot_" + std::to_string(plotCounter++) + "_" + std::to_string(now % 100000);
    return plotDir + "/" + name + ".png";
}

std::string PythonBridge::getPreamble() {
    return R"PY(
import warnings
warnings.filterwarnings('ignore')
import os, sys, json
if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    sys.stderr.reconfigure(encoding='utf-8', errors='replace')
import numpy as np
import pandas as pd

def _satan_safe_import(module_name):
    try:
        return __import__(module_name)
    except ImportError:
        return None

)PY";
}

void PythonBridge::writeFile(const std::string& path, const std::string& content) {
    std::ofstream f(path);
    if (!f.is_open()) throw std::runtime_error("Cannot write to: " + path);
    f << content;
    f.close();
}

std::string PythonBridge::readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return "";
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::string PythonBridge::writeTempAndRun(const std::string& script) {
    if (sessionDir.empty()) initSession();

    std::string scriptPath = sessionDir + "/temp_script.py";
    std::string outputPath = sessionDir + "/temp_output.txt";
    std::string errorPath = sessionDir + "/temp_error.txt";

    writeFile(scriptPath, script);

    std::string cmd = "python \"" + scriptPath + "\" > \"" + outputPath + "\" 2> \"" + errorPath + "\"";
    int result = std::system(cmd.c_str());

    std::string output = readFile(outputPath);
    std::string error = readFile(errorPath);

    if (result != 0 && !error.empty()) {
        // Filter out common warnings
        if (error.find("Error") != std::string::npos || error.find("error") != std::string::npos
            || error.find("Traceback") != std::string::npos) {
            throw std::runtime_error("Python error:\n" + error);
        }
    }

    return output;
}

std::string PythonBridge::executeImmediate(const std::string& code) {
    std::string fullScript = getPreamble() + code;
    return writeTempAndRun(fullScript);
}

std::string PythonBridge::flushAndExecute() {
    if (sessionScript.empty()) return "";
    std::string fullScript = getPreamble() + sessionScript;
    sessionScript.clear();
    return writeTempAndRun(fullScript);
}

SatanValue PythonBridge::parseResult(const std::string& output) {
    // Find the last line with __SATAN_RESULT__
    auto pos = output.rfind("__SATAN_RESULT__:");
    if (pos != std::string::npos) {
        std::string result = output.substr(pos + 17);
        // Trim whitespace
        while (!result.empty() && (result.back() == '\n' || result.back() == '\r' || result.back() == ' '))
            result.pop_back();

        // Try to parse as number
        try {
            double num = std::stod(result);
            return SatanValue(num);
        } catch (...) {}

        return SatanValue(result);
    }
    // Return full output as string
    std::string trimmed = output;
    while (!trimmed.empty() && (trimmed.back() == '\n' || trimmed.back() == '\r'))
        trimmed.pop_back();
    if (trimmed.empty()) return SatanValue();
    return SatanValue(trimmed);
}

void PythonBridge::openImage(const std::string& path) {
#ifdef _WIN32
    std::string cmd = "start \"\" \"" + path + "\"";
#elif __APPLE__
    std::string cmd = "open \"" + path + "\"";
#else
    std::string cmd = "xdg-open \"" + path + "\" &";
#endif
    std::system(cmd.c_str());
}

// =================== Code Generators ===================

std::string PythonBridge::genLoadCSV(const std::string& pyVar, const std::string& filepath) {
    return pyVar + " = pd.read_csv('" + filepath + "')\n"
         + "print(f'Loaded {" + pyVar + ".shape[0]} rows x {" + pyVar + ".shape[1]} columns')\n"
         + "print(f'Columns: {list(" + pyVar + ".columns)}')\n";
}

std::string PythonBridge::genCreateModel(const std::string& pyVar, const std::string& modelType,
                                          const std::vector<std::pair<std::string, std::string>>& kwargs) {
    std::string code;
    std::string args;
    for (const auto& kv : kwargs) {
        if (!args.empty()) args += ", ";
        args += kv.first + "=" + kv.second;
    }

    if (modelType == "LinearRegression") {
        code = "from sklearn.linear_model import LinearRegression\n";
        code += pyVar + " = LinearRegression(" + args + ")\n";
    } else if (modelType == "LogisticRegression") {
        code = "from sklearn.linear_model import LogisticRegression\n";
        code += pyVar + " = LogisticRegression(max_iter=1000" + (args.empty() ? "" : ", " + args) + ")\n";
    } else if (modelType == "DecisionTree") {
        code = "from sklearn.tree import DecisionTreeClassifier\n";
        code += pyVar + " = DecisionTreeClassifier(" + args + ")\n";
    } else if (modelType == "RandomForest") {
        code = "from sklearn.ensemble import RandomForestClassifier\n";
        code += pyVar + " = RandomForestClassifier(" + args + ")\n";
    } else if (modelType == "SVM") {
        code = "from sklearn.svm import SVC\n";
        code += pyVar + " = SVC(" + args + ")\n";
    } else if (modelType == "KMeans") {
        code = "from sklearn.cluster import KMeans\n";
        code += pyVar + " = KMeans(" + args + ")\n";
    } else if (modelType == "PCA") {
        code = "from sklearn.decomposition import PCA\n";
        code += pyVar + " = PCA(" + args + ")\n";
    } else if (modelType == "KNN") {
        code = "from sklearn.neighbors import KNeighborsClassifier\n";
        code += pyVar + " = KNeighborsClassifier(" + args + ")\n";
    } else if (modelType == "GradientBoosting") {
        code = "from sklearn.ensemble import GradientBoostingClassifier\n";
        code += pyVar + " = GradientBoostingClassifier(" + args + ")\n";
    } else if (modelType == "XGBoost") {
        code = "from xgboost import XGBClassifier\n";
        code += pyVar + " = XGBClassifier(eval_metric='logloss', use_label_encoder=False" + (args.empty() ? "" : ", " + args) + ")\n";
    } else if (modelType == "LightGBM") {
        code = "from lightgbm import LGBMClassifier\n";
        code += pyVar + " = LGBMClassifier(verbose=-1" + (args.empty() ? "" : ", " + args) + ")\n";
    } else {
        code = pyVar + " = " + modelType + "(" + args + ")\n";
    }
    return code;
}

std::string PythonBridge::genFitModel(const std::string& modelVar, const std::string& dataVar,
                                       const std::vector<std::pair<std::string, std::string>>& kwargs) {
    std::string code;
    code += "import pickle, time\n";
    code += "import numpy as np\n";
    code += "_X = " + dataVar + ".iloc[:, :-1].values\n";
    code += "_y = " + dataVar + ".iloc[:, -1].values\n";
    code += "from sklearn.model_selection import train_test_split, cross_val_score\n";
    code += "from sklearn.preprocessing import StandardScaler\n";
    code += "_X_train, _X_test, _y_train, _y_test = train_test_split(_X, _y, test_size=0.2, random_state=42)\n";
    code += "\n";
    code += "# Scale features for better results\n";
    code += "_scaler = StandardScaler()\n";
    code += "_X_train_scaled = _scaler.fit_transform(_X_train)\n";
    code += "_X_test_scaled = _scaler.transform(_X_test)\n";
    code += "\n";
    code += "# Train with timing\n";
    code += "_t_start = time.perf_counter()\n";
    code += modelVar + ".fit(_X_train_scaled, _y_train)\n";
    code += "_t_end = time.perf_counter()\n";
    code += "_train_time = _t_end - _t_start\n";
    code += "\n";
    code += "# Scores\n";
    code += "_train_score = " + modelVar + ".score(_X_train_scaled, _y_train)\n";
    code += "_test_score = " + modelVar + ".score(_X_test_scaled, _y_test)\n";
    code += "\n";
    code += "# Cross-validation (5-fold)\n";
    code += "_cv_scores = cross_val_score(" + modelVar + ", _scaler.transform(_X), _y, cv=5)\n";
    code += "\n";
    code += "# Detect if classification or regression\n";
    code += "from sklearn.base import is_classifier as _sklearn_is_clf\n";
    code += "_is_classifier = _sklearn_is_clf(" + modelVar + ")\n";
    code += "\n";
    code += "# Display results\n";
    code += "print()\n";
    code += "print('\\033[1;36m╔══════════════════════════════════════════╗')\n";
    code += "print('║         🔱 SATAN ML — Training Report     ║')\n";
    code += "print('╚══════════════════════════════════════════╝\\033[0m')\n";
    code += "print()\n";
    code += "print(f'  \\033[90mModel:\\033[0m        {type(" + modelVar + ").__name__}')\n";
    code += "print(f'  \\033[90mTrain size:\\033[0m   {len(_X_train)} samples')\n";
    code += "print(f'  \\033[90mTest size:\\033[0m    {len(_X_test)} samples')\n";
    code += "print(f'  \\033[90mFeatures:\\033[0m     {_X.shape[1]}')\n";
    code += "print(f'  \\033[90mTrain time:\\033[0m   {_train_time*1000:.2f} ms')\n";
    code += "print()\n";
    code += "\n";
    code += "if _is_classifier:\n";
    code += "    from sklearn.metrics import classification_report, accuracy_score, f1_score\n";
    code += "    _y_pred = " + modelVar + ".predict(_X_test_scaled)\n";
    code += "    _acc = accuracy_score(_y_test, _y_pred)\n";
    code += "    _f1 = f1_score(_y_test, _y_pred, average='weighted', zero_division=0)\n";
    code += "    print(f'  \\033[1;32m► Train Accuracy:\\033[0m  {_train_score:.4f}  ({_train_score*100:.1f}%)')\n";
    code += "    print(f'  \\033[1;33m► Test Accuracy:\\033[0m   {_test_score:.4f}  ({_test_score*100:.1f}%)')\n";
    code += "    print(f'  \\033[1;35m► F1 Score:\\033[0m        {_f1:.4f}')\n";
    code += "    print(f'  \\033[1;34m► Cross-Val (5F):\\033[0m  {_cv_scores.mean():.4f} ± {_cv_scores.std():.4f}')\n";
    code += "    if _train_score - _test_score > 0.1:\n";
    code += "        print(f'  \\033[33m⚠ Overfitting detected! Train-Test gap: {(_train_score - _test_score)*100:.1f}%\\033[0m')\n";
    code += "    print()\n";
    code += "    print('  \\033[90m--- Classification Report ---\\033[0m')\n";
    code += "    print(classification_report(_y_test, _y_pred, zero_division=0))\n";
    code += "    _score = _test_score\n";
    code += "else:\n";
    code += "    from sklearn.metrics import mean_squared_error, mean_absolute_error, r2_score\n";
    code += "    _y_pred = " + modelVar + ".predict(_X_test_scaled)\n";
    code += "    _r2 = r2_score(_y_test, _y_pred)\n";
    code += "    _mse = mean_squared_error(_y_test, _y_pred)\n";
    code += "    _rmse = np.sqrt(_mse)\n";
    code += "    _mae = mean_absolute_error(_y_test, _y_pred)\n";
    code += "    print(f'  \\033[1;32m► Train R²:\\033[0m       {_train_score:.4f}')\n";
    code += "    print(f'  \\033[1;33m► Test R²:\\033[0m        {_r2:.4f}')\n";
    code += "    print(f'  \\033[1;35m► MSE:\\033[0m            {_mse:.6f}')\n";
    code += "    print(f'  \\033[1;35m► RMSE:\\033[0m           {_rmse:.6f}')\n";
    code += "    print(f'  \\033[1;34m► MAE:\\033[0m            {_mae:.6f}')\n";
    code += "    print(f'  \\033[1;34m► Cross-Val (5F):\\033[0m  {_cv_scores.mean():.4f} ± {_cv_scores.std():.4f}')\n";
    code += "    if _train_score - _r2 > 0.1:\n";
    code += "        print(f'  \\033[33m⚠ Overfitting detected! Train-Test gap: {(_train_score - _r2)*100:.1f}%\\033[0m')\n";
    code += "    print()\n";
    code += "    _score = _r2\n";
    code += "\n";
    code += "# Save state (with scaler for consistent predictions)\n";
    code += "import pickle, os\n";
    code += "os.makedirs('" + sessionDir + "', exist_ok=True)\n";
    code += "with open('" + sessionDir + "/" + modelVar + ".pkl', 'wb') as f:\n";
    code += "    pickle.dump({'model': " + modelVar + ", 'scaler': _scaler, 'X_train': _X_train_scaled, 'X_test': _X_test_scaled, 'y_train': _y_train, 'y_test': _y_test, 'X': _X, 'y': _y, 'df': " + dataVar + ", 'is_classifier': _is_classifier}, f)\n";
    code += "print('__SATAN_RESULT__:' + str(_score))\n";
    return code;
}


std::string PythonBridge::genScore(const std::string& modelVar, const std::string& dataVar) {
    std::string code;
    code += "import pickle, numpy as np\n";
    code += "with open('" + sessionDir + "/" + modelVar + ".pkl', 'rb') as f:\n";
    code += "    _data = pickle.load(f)\n";
    code += "_score = _data['model'].score(_data['X_test'], _data['y_test'])\n";
    code += "_is_clf = _data.get('is_classifier', False)\n";
    code += "if _is_clf:\n";
    code += "    from sklearn.metrics import accuracy_score, f1_score\n";
    code += "    _y_pred = _data['model'].predict(_data['X_test'])\n";
    code += "    _f1 = f1_score(_data['y_test'], _y_pred, average='weighted', zero_division=0)\n";
    code += "    print(f'  Accuracy: {_score:.4f} ({_score*100:.1f}%)  |  F1: {_f1:.4f}')\n";
    code += "else:\n";
    code += "    from sklearn.metrics import mean_squared_error\n";
    code += "    _y_pred = _data['model'].predict(_data['X_test'])\n";
    code += "    _mse = mean_squared_error(_data['y_test'], _y_pred)\n";
    code += "    print(f'  R²: {_score:.4f}  |  MSE: {_mse:.6f}  |  RMSE: {np.sqrt(_mse):.6f}')\n";
    code += "print('__SATAN_RESULT__:' + str(_score))\n";
    return code;
}

std::string PythonBridge::genPredict(const std::string& modelVar, const std::string& inputExpr) {
    std::string code;
    code += "import pickle\n";
    code += "with open('" + sessionDir + "/" + modelVar + ".pkl', 'rb') as f:\n";
    code += "    _data = pickle.load(f)\n";
    code += "_pred = _data['model'].predict(" + inputExpr + ")\n";
    code += "print('Predictions:', _pred)\n";
    code += "print('__SATAN_RESULT__:' + str(list(_pred)))\n";
    return code;
}

std::string PythonBridge::genPlotModel(const std::string& modelVar, const std::string& dataVar,
                                        const std::string& plotPath) {
    std::string code;
    code += "import pickle, numpy as np\n";
    code += "import matplotlib\nmatplotlib.use('Agg')\nimport matplotlib.pyplot as plt\nimport seaborn as sns\n";
    code += "sns.set_theme(style='darkgrid', palette='husl')\n";
    code += "plt.rcParams['figure.facecolor'] = '#1a1a2e'\n";
    code += "plt.rcParams['axes.facecolor'] = '#16213e'\n";
    code += "plt.rcParams['text.color'] = '#e0e0e0'\n";
    code += "plt.rcParams['axes.labelcolor'] = '#e0e0e0'\n";
    code += "plt.rcParams['xtick.color'] = '#e0e0e0'\n";
    code += "plt.rcParams['ytick.color'] = '#e0e0e0'\n";
    code += "with open('" + sessionDir + "/" + modelVar + ".pkl', 'rb') as f:\n";
    code += "    _data = pickle.load(f)\n";
    code += "_pred = _data['model'].predict(_data['X_test'])\n";
    code += "_is_clf = _data.get('is_classifier', False)\n";
    code += "\n";
    code += "if _is_clf:\n";
    code += "    from sklearn.metrics import confusion_matrix, classification_report\n";
    code += "    fig, axes = plt.subplots(1, 2, figsize=(14, 6))\n";
    code += "    fig.suptitle('🔱 Satan ML — Classification Results', fontsize=16, fontweight='bold', color='#e94560')\n";
    code += "    # Confusion Matrix\n";
    code += "    _cm = confusion_matrix(_data['y_test'], _pred)\n";
    code += "    sns.heatmap(_cm, annot=True, fmt='d', cmap='RdYlBu_r', ax=axes[0], linewidths=0.5,\n";
    code += "                xticklabels=[f'Class {i}' for i in range(_cm.shape[0])],\n";
    code += "                yticklabels=[f'Class {i}' for i in range(_cm.shape[0])])\n";
    code += "    axes[0].set_xlabel('Predicted', fontsize=11)\n";
    code += "    axes[0].set_ylabel('Actual', fontsize=11)\n";
    code += "    axes[0].set_title('Confusion Matrix', color='#e94560', fontsize=13)\n";
    code += "    # Per-class accuracy bar\n";
    code += "    _classes = np.unique(_data['y_test'])\n";
    code += "    _per_class = []\n";
    code += "    for c in _classes:\n";
    code += "        _mask = _data['y_test'] == c\n";
    code += "        _per_class.append(np.mean(_pred[_mask] == c))\n";
    code += "    _bars = axes[1].bar([f'Class {int(c)}' for c in _classes], _per_class, color='#e94560', edgecolor='white', alpha=0.85)\n";
    code += "    for bar, val in zip(_bars, _per_class):\n";
    code += "        axes[1].text(bar.get_x() + bar.get_width()/2., bar.get_height() + 0.02, f'{val:.1%}', ha='center', fontsize=10, color='#e0e0e0')\n";
    code += "    axes[1].set_ylim(0, 1.15)\n";
    code += "    axes[1].set_ylabel('Accuracy', fontsize=11)\n";
    code += "    axes[1].set_title('Per-Class Accuracy', color='#e94560', fontsize=13)\n";
    code += "else:\n";
    code += "    from sklearn.metrics import r2_score\n";
    code += "    fig, axes = plt.subplots(1, 2, figsize=(14, 6))\n";
    code += "    fig.suptitle('🔱 Satan ML — Regression Results', fontsize=16, fontweight='bold', color='#e94560')\n";
    code += "    # Actual vs Predicted\n";
    code += "    axes[0].scatter(_data['y_test'], _pred, alpha=0.7, c='#e94560', edgecolors='white', s=50)\n";
    code += "    _mn, _mx = min(_data['y_test'].min(), _pred.min()), max(_data['y_test'].max(), _pred.max())\n";
    code += "    axes[0].plot([_mn, _mx], [_mn, _mx], '--', lw=2, color='#0f3460', label='Perfect Fit')\n";
    code += "    axes[0].set_xlabel('Actual', fontsize=11)\n";
    code += "    axes[0].set_ylabel('Predicted', fontsize=11)\n";
    code += "    _r2 = r2_score(_data['y_test'], _pred)\n";
    code += "    axes[0].set_title(f'Actual vs Predicted (R²={_r2:.4f})', color='#e94560', fontsize=13)\n";
    code += "    axes[0].legend()\n";
    code += "    # Residuals\n";
    code += "    _residuals = _data['y_test'] - _pred\n";
    code += "    axes[1].hist(_residuals, bins=20, color='#e94560', edgecolor='white', alpha=0.8)\n";
    code += "    axes[1].axvline(x=0, color='#0f3460', linestyle='--', lw=2)\n";
    code += "    axes[1].set_xlabel('Residual', fontsize=11)\n";
    code += "    axes[1].set_ylabel('Frequency', fontsize=11)\n";
    code += "    axes[1].set_title(f'Residual Distribution (μ={_residuals.mean():.4f})', color='#e94560', fontsize=13)\n";
    code += "\n";
    code += "plt.tight_layout()\n";
    code += "plt.savefig('" + plotPath + "', dpi=150, bbox_inches='tight')\n";
    return code;
}


std::string PythonBridge::genDescribe(const std::string& dataVar) {
    return "print(" + dataVar + ".describe().to_string())\n";
}

std::string PythonBridge::genHead(const std::string& dataVar, int n) {
    return "print(" + dataVar + ".head(" + std::to_string(n) + ").to_string())\n";
}

std::string PythonBridge::genShape(const std::string& dataVar) {
    return "print(f'Shape: {" + dataVar + ".shape}')\n"
         + "print('__SATAN_RESULT__:' + str(list(" + dataVar + ".shape)))\n";
}

std::string PythonBridge::genCorrelation(const std::string& dataVar, const std::string& plotPath) {
    std::string code;
    code += "import matplotlib\nmatplotlib.use('Agg')\nimport matplotlib.pyplot as plt\nimport seaborn as sns\n";
    code += "sns.set_theme(style='darkgrid')\n";
    code += "plt.rcParams['figure.facecolor'] = '#1a1a2e'\n";
    code += "plt.rcParams['axes.facecolor'] = '#16213e'\n";
    code += "plt.rcParams['text.color'] = '#e0e0e0'\n";
    code += "fig, ax = plt.subplots(figsize=(10, 8))\n";
    code += "_corr = " + dataVar + ".select_dtypes(include=[np.number]).corr()\n";
    code += "sns.heatmap(_corr, annot=True, cmap='RdYlBu_r', center=0, ax=ax, fmt='.2f', linewidths=0.5)\n";
    code += "ax.set_title('Correlation Matrix', fontsize=16, fontweight='bold', color='#e94560')\n";
    code += "plt.tight_layout()\n";
    code += "plt.savefig('" + plotPath + "', dpi=150, bbox_inches='tight')\n";
    code += "print('Correlation plot saved.')\n";
    return code;
}

std::string PythonBridge::genPlotData(const std::string& dataVar, const std::string& plotPath, const std::string& kind) {
    std::string code;
    code += "import matplotlib\nmatplotlib.use('Agg')\nimport matplotlib.pyplot as plt\nimport seaborn as sns\n";
    code += "sns.set_theme(style='darkgrid', palette='husl')\n";
    code += "plt.rcParams['figure.facecolor'] = '#1a1a2e'\n";
    code += "plt.rcParams['axes.facecolor'] = '#16213e'\n";
    code += "plt.rcParams['text.color'] = '#e0e0e0'\n";
    code += "plt.rcParams['axes.labelcolor'] = '#e0e0e0'\n";
    code += "_numeric_cols = " + dataVar + ".select_dtypes(include=[np.number]).columns\n";
    code += "n_cols = len(_numeric_cols)\n";
    code += "fig, axes = plt.subplots(1, min(n_cols, 4), figsize=(5*min(n_cols, 4), 5))\n";
    code += "if n_cols == 1: axes = [axes]\n";
    code += "for i, col in enumerate(_numeric_cols[:4]):\n";
    code += "    axes[i].hist(" + dataVar + "[col], bins=25, color='#e94560', edgecolor='white', alpha=0.8)\n";
    code += "    axes[i].set_title(col, color='#e94560')\n";
    code += "fig.suptitle('Data Overview', fontsize=16, fontweight='bold', color='#e94560')\n";
    code += "plt.tight_layout()\n";
    code += "plt.savefig('" + plotPath + "', dpi=150, bbox_inches='tight')\n";
    code += "print('Data plot saved.')\n";
    return code;
}

// Deep Learning generators
std::string PythonBridge::genNeuralNet(const std::string& pyVar, const std::vector<int>& layers) {
    std::string code;
    code += "import torch\nimport torch.nn as nn\n";
    code += "class _SatanNet(nn.Module):\n";
    code += "    def __init__(self):\n";
    code += "        super().__init__()\n";
    code += "        self.layers = nn.Sequential(\n";
    for (size_t i = 0; i + 1 < layers.size(); i++) {
        code += "            nn.Linear(" + std::to_string(layers[i]) + ", " + std::to_string(layers[i+1]) + "),\n";
        if (i + 2 < layers.size()) code += "            nn.ReLU(),\n            nn.Dropout(0.2),\n";
    }
    code += "        )\n";
    code += "    def forward(self, x): return self.layers(x)\n";
    code += pyVar + " = _SatanNet()\n";
    code += "print(f'Neural Network created: {" + pyVar + "}')\n";
    return code;
}

std::string PythonBridge::genTrainNN(const std::string& modelVar, const std::string& dataVar,
                                      int epochs, double lr) {
    std::string code;
    code += "import torch\nimport torch.nn as nn\nimport torch.optim as optim\n";
    code += "_X = torch.FloatTensor(" + dataVar + ".iloc[:, :-1].values)\n";
    code += "_y_raw = " + dataVar + ".iloc[:, -1].values\n";
    code += "if _y_raw.dtype == object:\n";
    code += "    from sklearn.preprocessing import LabelEncoder\n";
    code += "    _y = torch.LongTensor(LabelEncoder().fit_transform(_y_raw))\n";
    code += "    _criterion = nn.CrossEntropyLoss()\n";
    code += "else:\n";
    code += "    _y = torch.FloatTensor(_y_raw).unsqueeze(1)\n";
    code += "    _criterion = nn.MSELoss()\n";
    code += "_optimizer = optim.Adam(" + modelVar + ".parameters(), lr=" + std::to_string(lr) + ")\n";
    code += "_losses = []\n";
    code += "for _epoch in range(" + std::to_string(epochs) + "):\n";
    code += "    _optimizer.zero_grad()\n";
    code += "    _out = " + modelVar + "(_X)\n";
    code += "    _loss = _criterion(_out, _y)\n";
    code += "    _loss.backward()\n";
    code += "    _optimizer.step()\n";
    code += "    _losses.append(_loss.item())\n";
    code += "    if (_epoch+1) % " + std::to_string(std::max(1, epochs / 10)) + " == 0:\n";
    code += "        print(f'  Epoch {_epoch+1}/" + std::to_string(epochs) + " | Loss: {_loss.item():.6f}')\n";
    code += "print(f'Training complete! Final loss: {_losses[-1]:.6f}')\n";
    code += "import pickle, os\n";
    code += "os.makedirs('" + sessionDir + "', exist_ok=True)\n";
    code += "with open('" + sessionDir + "/" + modelVar + ".pkl', 'wb') as f:\n";
    code += "    pickle.dump({'model': " + modelVar + ", 'losses': _losses, 'X': _X, 'y': _y}, f)\n";
    code += "print('__SATAN_RESULT__:' + str(_losses[-1]))\n";
    return code;
}

std::string PythonBridge::genPlotLoss(const std::string& modelVar, const std::string& plotPath) {
    std::string code;
    code += "import pickle\nimport matplotlib\nmatplotlib.use('Agg')\nimport matplotlib.pyplot as plt\n";
    code += "plt.rcParams['figure.facecolor'] = '#1a1a2e'\nplt.rcParams['axes.facecolor'] = '#16213e'\n";
    code += "plt.rcParams['text.color'] = '#e0e0e0'\nplt.rcParams['axes.labelcolor'] = '#e0e0e0'\n";
    code += "plt.rcParams['xtick.color'] = '#e0e0e0'\nplt.rcParams['ytick.color'] = '#e0e0e0'\n";
    code += "with open('" + sessionDir + "/" + modelVar + ".pkl', 'rb') as f:\n    _data = pickle.load(f)\n";
    code += "fig, ax = plt.subplots(figsize=(10, 6))\n";
    code += "ax.plot(_data['losses'], color='#e94560', linewidth=2)\n";
    code += "ax.fill_between(range(len(_data['losses'])), _data['losses'], alpha=0.3, color='#e94560')\n";
    code += "ax.set_xlabel('Epoch', fontsize=12)\nax.set_ylabel('Loss', fontsize=12)\n";
    code += "ax.set_title('Training Loss Curve', fontsize=16, fontweight='bold', color='#e94560')\n";
    code += "ax.grid(True, alpha=0.3)\nplt.tight_layout()\n";
    code += "plt.savefig('" + plotPath + "', dpi=150, bbox_inches='tight')\nprint('Loss plot saved.')\n";
    return code;
}

// NLP Generators
std::string PythonBridge::genSentiment(const std::string& text) {
    std::string code;
    code += "try:\n";
    code += "    from transformers import pipeline\n";
    code += "    _analyzer = pipeline('sentiment-analysis', model='distilbert-base-uncased-finetuned-sst-2-english')\n";
    code += "    _result = _analyzer('" + text + "')[0]\n";
    code += "    _score = _result['score'] if _result['label'] == 'POSITIVE' else -_result['score']\n";
    code += "    print(f'Sentiment: {_result[\"label\"]} (confidence: {_result[\"score\"]:.4f})')\n";
    code += "    print('__SATAN_RESULT__:' + str(_score))\n";
    code += "except ImportError:\n";
    code += "    from nltk.sentiment import SentimentIntensityAnalyzer\n";
    code += "    import nltk\n";
    code += "    try: nltk.data.find('sentiment/vader_lexicon.zip')\n";
    code += "    except: nltk.download('vader_lexicon', quiet=True)\n";
    code += "    _sia = SentimentIntensityAnalyzer()\n";
    code += "    _scores = _sia.polarity_scores('" + text + "')\n";
    code += "    print(f'Sentiment: compound={_scores[\"compound\"]:.4f}')\n";
    code += "    print('__SATAN_RESULT__:' + str(_scores['compound']))\n";
    return code;
}

std::string PythonBridge::genTokenize(const std::string& text) {
    std::string code;
    code += "import nltk\n";
    code += "try: nltk.data.find('tokenizers/punkt_tab')\n";
    code += "except: nltk.download('punkt_tab', quiet=True)\n";
    code += "_tokens = nltk.word_tokenize('" + text + "')\n";
    code += "print(f'Tokens: {_tokens}')\n";
    code += "print('__SATAN_RESULT__:' + json.dumps(_tokens))\n";
    return code;
}

std::string PythonBridge::genWordCloud(const std::string& text, const std::string& plotPath) {
    std::string code;
    code += "from wordcloud import WordCloud\nimport matplotlib\nmatplotlib.use('Agg')\nimport matplotlib.pyplot as plt\n";
    code += "_wc = WordCloud(width=1200, height=600, background_color='#1a1a2e', colormap='magma', max_words=100).generate('" + text + "')\n";
    code += "fig, ax = plt.subplots(figsize=(12, 6))\nax.imshow(_wc, interpolation='bilinear')\nax.axis('off')\n";
    code += "fig.patch.set_facecolor('#1a1a2e')\n";
    code += "plt.tight_layout()\nplt.savefig('" + plotPath + "', dpi=150, bbox_inches='tight')\nprint('Word cloud saved.')\n";
    return code;
}

// Plot Generators
std::string PythonBridge::genScatter(const std::string& xExpr, const std::string& yExpr, const std::string& plotPath) {
    std::string code;
    code += "import matplotlib\nmatplotlib.use('Agg')\nimport matplotlib.pyplot as plt\n";
    code += "plt.rcParams['figure.facecolor'] = '#1a1a2e'\nplt.rcParams['axes.facecolor'] = '#16213e'\n";
    code += "plt.rcParams['text.color'] = '#e0e0e0'\nplt.rcParams['axes.labelcolor'] = '#e0e0e0'\n";
    code += "fig, ax = plt.subplots(figsize=(10, 6))\n";
    code += "ax.scatter(" + xExpr + ", " + yExpr + ", c='#e94560', edgecolors='white', s=60, alpha=0.8)\n";
    code += "ax.set_title('Scatter Plot', fontsize=14, fontweight='bold', color='#e94560')\n";
    code += "ax.grid(True, alpha=0.3)\nplt.tight_layout()\nplt.savefig('" + plotPath + "', dpi=150)\nprint('Scatter plot saved.')\n";
    return code;
}

std::string PythonBridge::genLinePlot(const std::string& xExpr, const std::string& yExpr, const std::string& plotPath) {
    std::string code;
    code += "import matplotlib\nmatplotlib.use('Agg')\nimport matplotlib.pyplot as plt\n";
    code += "plt.rcParams['figure.facecolor'] = '#1a1a2e'\nplt.rcParams['axes.facecolor'] = '#16213e'\n";
    code += "plt.rcParams['text.color'] = '#e0e0e0'\n";
    code += "fig, ax = plt.subplots(figsize=(10, 6))\n";
    code += "ax.plot(" + xExpr + ", " + yExpr + ", color='#e94560', linewidth=2, marker='o', markersize=4)\n";
    code += "ax.fill_between(range(len(" + yExpr + ")), " + yExpr + ", alpha=0.2, color='#e94560')\n";
    code += "ax.set_title('Line Plot', fontsize=14, fontweight='bold', color='#e94560')\n";
    code += "ax.grid(True, alpha=0.3)\nplt.tight_layout()\nplt.savefig('" + plotPath + "', dpi=150)\n";
    return code;
}

std::string PythonBridge::genHistogram(const std::string& dataExpr, const std::string& plotPath, int bins) {
    std::string code;
    code += "import matplotlib\nmatplotlib.use('Agg')\nimport matplotlib.pyplot as plt\n";
    code += "plt.rcParams['figure.facecolor'] = '#1a1a2e'\nplt.rcParams['axes.facecolor'] = '#16213e'\n";
    code += "plt.rcParams['text.color'] = '#e0e0e0'\n";
    code += "fig, ax = plt.subplots(figsize=(10, 6))\n";
    code += "ax.hist(" + dataExpr + ", bins=" + std::to_string(bins) + ", color='#e94560', edgecolor='white', alpha=0.8)\n";
    code += "ax.set_title('Histogram', fontsize=14, fontweight='bold', color='#e94560')\n";
    code += "ax.grid(True, alpha=0.3)\nplt.tight_layout()\nplt.savefig('" + plotPath + "', dpi=150)\n";
    return code;
}

std::string PythonBridge::genHeatmap(const std::string& dataExpr, const std::string& plotPath) {
    return genCorrelation(dataExpr, plotPath);
}

std::string PythonBridge::genBarChart(const std::string& labels, const std::string& values, const std::string& plotPath) {
    std::string code;
    code += "import matplotlib\nmatplotlib.use('Agg')\nimport matplotlib.pyplot as plt\n";
    code += "plt.rcParams['figure.facecolor'] = '#1a1a2e'\nplt.rcParams['axes.facecolor'] = '#16213e'\n";
    code += "plt.rcParams['text.color'] = '#e0e0e0'\n";
    code += "fig, ax = plt.subplots(figsize=(10, 6))\n";
    code += "ax.bar(" + labels + ", " + values + ", color='#e94560', edgecolor='white', alpha=0.8)\n";
    code += "ax.set_title('Bar Chart', fontsize=14, fontweight='bold', color='#e94560')\n";
    code += "plt.tight_layout()\nplt.savefig('" + plotPath + "', dpi=150)\n";
    return code;
}

std::string PythonBridge::genConfusionMatrix(const std::string& yTrue, const std::string& yPred, const std::string& plotPath) {
    std::string code;
    code += "import matplotlib\nmatplotlib.use('Agg')\nimport matplotlib.pyplot as plt\nimport seaborn as sns\n";
    code += "from sklearn.metrics import confusion_matrix\n";
    code += "plt.rcParams['figure.facecolor'] = '#1a1a2e'\nplt.rcParams['axes.facecolor'] = '#16213e'\n";
    code += "plt.rcParams['text.color'] = '#e0e0e0'\n";
    code += "_cm = confusion_matrix(" + yTrue + ", " + yPred + ")\n";
    code += "fig, ax = plt.subplots(figsize=(8, 6))\n";
    code += "sns.heatmap(_cm, annot=True, fmt='d', cmap='RdYlBu_r', ax=ax)\n";
    code += "ax.set_title('Confusion Matrix', fontsize=14, fontweight='bold', color='#e94560')\n";
    code += "plt.tight_layout()\nplt.savefig('" + plotPath + "', dpi=150)\n";
    return code;
}

// ============================================================
// Feature 1: Hyperparameter Tuning
// ============================================================
std::string PythonBridge::genTuneModel(const std::string& modelVar, const std::string& dataVar) {
    std::string code;
    code += "import pickle, time, numpy as np\n";
    code += "from sklearn.model_selection import RandomizedSearchCV, train_test_split\n";
    code += "from sklearn.preprocessing import StandardScaler\n";
    code += "from sklearn.base import is_classifier as _sklearn_is_clf\n";
    code += "_X = " + dataVar + ".iloc[:, :-1].values\n";
    code += "_y = " + dataVar + ".iloc[:, -1].values\n";
    code += "_scaler = StandardScaler()\n";
    code += "_X_scaled = _scaler.fit_transform(_X)\n";
    code += "_X_train, _X_test, _y_train, _y_test = train_test_split(_X_scaled, _y, test_size=0.2, random_state=42)\n";
    code += "_model_name = type(" + modelVar + ").__name__\n";
    code += "_param_grids = {\n";
    code += "    'RandomForestClassifier': {'n_estimators': [50,100,200,300], 'max_depth': [None,5,10,20], 'min_samples_split': [2,5,10]},\n";
    code += "    'DecisionTreeClassifier': {'max_depth': [None,3,5,10,20], 'min_samples_split': [2,5,10], 'criterion': ['gini','entropy']},\n";
    code += "    'GradientBoostingClassifier': {'n_estimators': [50,100,200], 'learning_rate': [0.05,0.1,0.2], 'max_depth': [3,5,7]},\n";
    code += "    'SVC': {'C': [0.1,1,10,100], 'kernel': ['rbf','linear','poly'], 'gamma': ['scale','auto']},\n";
    code += "    'KNeighborsClassifier': {'n_neighbors': [3,5,7,9,11], 'weights': ['uniform','distance']},\n";
    code += "    'LogisticRegression': {'C': [0.01,0.1,1,10,100], 'solver': ['lbfgs','saga']},\n";
    code += "    'XGBClassifier': {'n_estimators': [50,100,200], 'max_depth': [3,5,7], 'learning_rate': [0.05,0.1,0.2]},\n";
    code += "    'LGBMClassifier': {'n_estimators': [50,100,200], 'max_depth': [-1,5,10], 'learning_rate': [0.05,0.1,0.2]},\n";
    code += "}\n";
    code += "_params = _param_grids.get(_model_name, {})\n";
    code += "_scoring = 'accuracy' if _sklearn_is_clf(" + modelVar + ") else 'r2'\n";
    code += "print()\nprint('\\033[1;35m=== Hyperparameter Tuning ===')\n";
    code += "print(f'  Model: {_model_name}')\n";
    code += "if _params:\n";
    code += "    _t = time.perf_counter()\n";
    code += "    _search = RandomizedSearchCV(" + modelVar + ", _params, n_iter=20, cv=5, n_jobs=-1, random_state=42, scoring=_scoring)\n";
    code += "    _search.fit(_X_train, _y_train)\n";
    code += "    _t2 = time.perf_counter()\n";
    code += "    _best = _search.best_estimator_\n";
    code += "    _best_score = _best.score(_X_test, _y_test)\n";
    code += "    print(f'  Best CV score:  {_search.best_score_:.4f}')\n";
    code += "    print(f'  Test score:     {_best_score:.4f}')\n";
    code += "    print(f'  Tuning time:    {(_t2-_t):.1f}s')\n";
    code += "    print(f'  Best params:    {_search.best_params_}')\n";
    code += "    import pickle, os\n";
    code += "    os.makedirs('" + sessionDir + "', exist_ok=True)\n";
    code += "    with open('" + sessionDir + "/" + modelVar + ".pkl', 'wb') as f:\n";
    code += "        pickle.dump({'model': _best, 'scaler': _scaler, 'X_train': _X_train, 'X_test': _X_test, 'y_train': _y_train, 'y_test': _y_test, 'X': _X, 'y': _y, 'df': " + dataVar + ", 'is_classifier': _sklearn_is_clf(_best)}, f)\n";
    code += "    print('\\033[0m')\n";
    code += "    print('__SATAN_RESULT__:' + str(_best_score))\n";
    code += "else:\n";
    code += "    print('  No param grid defined for this model type.')\n";
    code += "    print('__SATAN_RESULT__:0')\n";
    return code;
}

// ============================================================
// Feature 2: Advanced Visualizations
// ============================================================
std::string PythonBridge::genFeatureImportance(const std::string& modelVar, const std::string& plotPath) {
    std::string code;
    code += "import pickle, numpy as np\n";
    code += "import matplotlib\nmatplotlib.use('Agg')\nimport matplotlib.pyplot as plt\n";
    code += "plt.rcParams.update({'figure.facecolor':'#1a1a2e','axes.facecolor':'#16213e','text.color':'#e0e0e0','axes.labelcolor':'#e0e0e0','xtick.color':'#e0e0e0','ytick.color':'#e0e0e0'})\n";
    code += "with open('" + sessionDir + "/" + modelVar + ".pkl', 'rb') as f:\n    _d = pickle.load(f)\n";
    code += "_m = _d['model']\n";
    code += "_col_names = [f'Feature {i}' for i in range(_d['X'].shape[1])]\n";
    code += "if hasattr(_d.get('df', None), 'columns'):\n";
    code += "    _col_names = list(_d['df'].iloc[:, :-1].columns)\n";
    code += "if hasattr(_m, 'feature_importances_'):\n";
    code += "    _imp = _m.feature_importances_\n";
    code += "elif hasattr(_m, 'coef_'):\n";
    code += "    _imp = np.abs(_m.coef_[0] if _m.coef_.ndim > 1 else _m.coef_)\n";
    code += "else:\n";
    code += "    print('  This model does not support feature importance.')\n";
    code += "    print('__SATAN_RESULT__:nil')\n";
    code += "    import sys; sys.exit(0)\n";
    code += "_idx = np.argsort(_imp)\n";
    code += "fig, ax = plt.subplots(figsize=(10, max(4, len(_col_names)*0.6)))\n";
    code += "_colors = ['#e94560' if v == _imp.max() else '#0f3460' for v in _imp[_idx]]\n";
    code += "bars = ax.barh([_col_names[i] for i in _idx], _imp[_idx], color=_colors, edgecolor='white', alpha=0.9)\n";
    code += "for bar, val in zip(bars, _imp[_idx]):\n";
    code += "    ax.text(bar.get_width()+0.001, bar.get_y()+bar.get_height()/2., f'{val:.4f}', va='center', fontsize=9, color='#e0e0e0')\n";
    code += "ax.set_xlabel('Importance Score', fontsize=12)\n";
    code += "ax.set_title(f'Feature Importance — {type(_m).__name__}', fontsize=14, fontweight='bold', color='#e94560')\n";
    code += "plt.tight_layout()\nplt.savefig('" + plotPath + "', dpi=150, bbox_inches='tight')\n";
    code += "print('__SATAN_RESULT__:' + str(list(_imp)))\n";
    return code;
}

std::string PythonBridge::genROCCurve(const std::string& modelVar, const std::string& plotPath) {
    std::string code;
    code += "import pickle, numpy as np\n";
    code += "import matplotlib\nmatplotlib.use('Agg')\nimport matplotlib.pyplot as plt\n";
    code += "from sklearn.metrics import roc_curve, auc\n";
    code += "from sklearn.preprocessing import label_binarize\n";
    code += "plt.rcParams.update({'figure.facecolor':'#1a1a2e','axes.facecolor':'#16213e','text.color':'#e0e0e0','axes.labelcolor':'#e0e0e0','xtick.color':'#e0e0e0','ytick.color':'#e0e0e0'})\n";
    code += "with open('" + sessionDir + "/" + modelVar + ".pkl', 'rb') as f:\n    _d = pickle.load(f)\n";
    code += "_m, _Xt, _yt = _d['model'], _d['X_test'], _d['y_test']\n";
    code += "_classes = np.unique(_yt)\n";
    code += "fig, ax = plt.subplots(figsize=(10, 7))\n";
    code += "_colors = ['#e94560','#0f3460','#533483','#e6b325','#22a39f']\n";
    code += "if len(_classes) == 2:\n";
    code += "    _proba = _m.predict_proba(_Xt)[:,1] if hasattr(_m,'predict_proba') else _m.decision_function(_Xt)\n";
    code += "    _fpr, _tpr, _ = roc_curve(_yt, _proba)\n";
    code += "    _auc_score = auc(_fpr, _tpr)\n";
    code += "    ax.plot(_fpr, _tpr, color='#e94560', lw=2, label=f'ROC (AUC={_auc_score:.4f})')\n";
    code += "else:\n";
    code += "    _yb = label_binarize(_yt, classes=_classes)\n";
    code += "    if hasattr(_m,'predict_proba'):\n";
    code += "        _proba = _m.predict_proba(_Xt)\n";
    code += "        for i, cls in enumerate(_classes):\n";
    code += "            _fpr, _tpr, _ = roc_curve(_yb[:,i], _proba[:,i])\n";
    code += "            ax.plot(_fpr, _tpr, color=_colors[i%len(_colors)], lw=2, label=f'Class {cls} (AUC={auc(_fpr,_tpr):.3f})')\n";
    code += "ax.plot([0,1],[0,1],'--',color='#888',lw=1,label='Random')\n";
    code += "ax.set_xlabel('False Positive Rate', fontsize=12)\nax.set_ylabel('True Positive Rate', fontsize=12)\n";
    code += "ax.set_title('ROC / AUC Curve', fontsize=14, fontweight='bold', color='#e94560')\n";
    code += "ax.legend(loc='lower right', framealpha=0.3)\nax.grid(True, alpha=0.3)\n";
    code += "plt.tight_layout()\nplt.savefig('" + plotPath + "', dpi=150, bbox_inches='tight')\n";
    code += "print('__SATAN_RESULT__:done')\n";
    return code;
}

std::string PythonBridge::genLearningCurve(const std::string& modelVar, const std::string& plotPath) {
    std::string code;
    code += "import pickle, numpy as np\n";
    code += "import matplotlib\nmatplotlib.use('Agg')\nimport matplotlib.pyplot as plt\n";
    code += "from sklearn.model_selection import learning_curve\n";
    code += "from sklearn.preprocessing import StandardScaler\n";
    code += "plt.rcParams.update({'figure.facecolor':'#1a1a2e','axes.facecolor':'#16213e','text.color':'#e0e0e0','axes.labelcolor':'#e0e0e0','xtick.color':'#e0e0e0','ytick.color':'#e0e0e0'})\n";
    code += "with open('" + sessionDir + "/" + modelVar + ".pkl', 'rb') as f:\n    _d = pickle.load(f)\n";
    code += "_m, _X, _y = _d['model'], _d['X'], _d['y']\n";
    code += "_X_s = StandardScaler().fit_transform(_X)\n";
    code += "_scoring = 'accuracy' if _d.get('is_classifier') else 'r2'\n";
    code += "_sizes, _tr_sc, _val_sc = learning_curve(_m, _X_s, _y, cv=5, n_jobs=-1, train_sizes=np.linspace(0.1,1.0,10), scoring=_scoring)\n";
    code += "_tr_m, _tr_s = _tr_sc.mean(1), _tr_sc.std(1)\n";
    code += "_val_m, _val_s = _val_sc.mean(1), _val_sc.std(1)\n";
    code += "fig, ax = plt.subplots(figsize=(10, 6))\n";
    code += "ax.plot(_sizes, _tr_m, 'o-', color='#e94560', lw=2, label='Training score')\n";
    code += "ax.fill_between(_sizes, _tr_m-_tr_s, _tr_m+_tr_s, alpha=0.2, color='#e94560')\n";
    code += "ax.plot(_sizes, _val_m, 's-', color='#0f3460', lw=2, label='Cross-val score')\n";
    code += "ax.fill_between(_sizes, _val_m-_val_s, _val_m+_val_s, alpha=0.2, color='#0f3460')\n";
    code += "ax.set_xlabel('Training samples', fontsize=12)\nax.set_ylabel('Score', fontsize=12)\n";
    code += "ax.set_title('Learning Curve', fontsize=14, fontweight='bold', color='#e94560')\n";
    code += "ax.legend(framealpha=0.3)\nax.grid(True, alpha=0.3)\n";
    code += "plt.tight_layout()\nplt.savefig('" + plotPath + "', dpi=150, bbox_inches='tight')\n";
    code += "print('__SATAN_RESULT__:done')\n";
    return code;
}

// ============================================================
// Feature 4: Data Preprocessing
// ============================================================
std::string PythonBridge::genFillMissing(const std::string& dataVar, const std::string& src) {
    std::string code;
    code += "_df = pd.read_csv('" + src + "')\n";
    code += "for _col in _df.select_dtypes(include='number').columns:\n";
    code += "    _df[_col] = _df[_col].fillna(_df[_col].median())\n";
    code += "for _col in _df.select_dtypes(include='object').columns:\n";
    code += "    _df[_col] = _df[_col].fillna(_df[_col].mode()[0])\n";
    code += "_filled = _df.isnull().sum().sum()\n";
    code += "print(f'  Missing values filled. Remaining nulls: {_filled}')\n";
    code += dataVar + " = _df\n";
    code += "print('__SATAN_RESULT__:done')\n";
    return code;
}

std::string PythonBridge::genDropNulls(const std::string& dataVar, const std::string& src) {
    std::string code;
    code += "_df = pd.read_csv('" + src + "')\n";
    code += "_before = len(_df)\n";
    code += "_df = _df.dropna()\n";
    code += "_after = len(_df)\n";
    code += "print(f'  Dropped {_before-_after} rows with nulls. Remaining: {_after} rows.')\n";
    code += dataVar + " = _df\n";
    code += "print('__SATAN_RESULT__:' + str(_after))\n";
    return code;
}

std::string PythonBridge::genEncodeText(const std::string& dataVar, const std::string& src) {
    std::string code;
    code += "_df = pd.read_csv('" + src + "')\n";
    code += "from sklearn.preprocessing import LabelEncoder as _LE\n";
    code += "_le = _LE()\n";
    code += "_encoded_cols = []\n";
    code += "for _col in _df.select_dtypes(include='object').columns:\n";
    code += "    _df[_col] = _le.fit_transform(_df[_col].astype(str))\n";
    code += "    _encoded_cols.append(_col)\n";
    code += "print(f'  Encoded {len(_encoded_cols)} text columns: {_encoded_cols}')\n";
    code += dataVar + " = _df\n";
    code += "print('__SATAN_RESULT__:done')\n";
    return code;
}

std::string PythonBridge::genNormalize(const std::string& dataVar, const std::string& src) {
    std::string code;
    code += "_df = pd.read_csv('" + src + "')\n";
    code += "from sklearn.preprocessing import MinMaxScaler as _MMS\n";
    code += "_num_cols = _df.select_dtypes(include='number').columns[:-1]\n";
    code += "_df[_num_cols] = _MMS().fit_transform(_df[_num_cols])\n";
    code += "print(f'  Normalized {len(_num_cols)} feature columns to [0,1] range.')\n";
    code += dataVar + " = _df\n";
    code += "print('__SATAN_RESULT__:done')\n";
    return code;
}

// ============================================================
// Feature 5: Model Save / Load
// ============================================================
std::string PythonBridge::genSaveModel(const std::string& modelVar, const std::string& filepath) {
    std::string code;
    code += "import pickle\n";
    code += "with open('" + sessionDir + "/" + modelVar + ".pkl', 'rb') as f:\n";
    code += "    _bundle = pickle.load(f)\n";
    code += "with open('" + filepath + "', 'wb') as f:\n";
    code += "    pickle.dump(_bundle, f)\n";
    code += "print(f'  Model saved to: " + filepath + "')\n";
    code += "print('__SATAN_RESULT__:done')\n";
    return code;
}

std::string PythonBridge::genLoadModel(const std::string& filepath, const std::string& newVar) {
    std::string code;
    code += "import pickle, os\n";
    code += "with open('" + filepath + "', 'rb') as f:\n";
    code += "    _bundle = pickle.load(f)\n";
    code += newVar + " = _bundle['model']\n";
    code += "_score = _bundle['model'].score(_bundle['X_test'], _bundle['y_test'])\n";
    code += "print(f'  Model loaded: {type(_bundle[\"model\"]).__name__}')\n";
    code += "print(f'  Stored test score: {_score:.4f}')\n";
    code += "os.makedirs('" + sessionDir + "', exist_ok=True)\n";
    code += "with open('" + sessionDir + "/" + newVar + ".pkl', 'wb') as f:\n";
    code += "    pickle.dump(_bundle, f)\n";
    code += "print('__SATAN_RESULT__:' + str(_score))\n";
    return code;
}

// ============================================================
// Feature 6: AutoML
// ============================================================
std::string PythonBridge::genAutoML(const std::string& dataVar, const std::string& winnerVar) {
    std::string code;
    code += "import pickle, time, numpy as np\n";
    code += "from sklearn.model_selection import cross_val_score, train_test_split\n";
    code += "from sklearn.preprocessing import StandardScaler\n";
    code += "from sklearn.linear_model import LogisticRegression, LinearRegression\n";
    code += "from sklearn.tree import DecisionTreeClassifier\n";
    code += "from sklearn.ensemble import RandomForestClassifier, GradientBoostingClassifier\n";
    code += "from sklearn.svm import SVC\n";
    code += "from sklearn.neighbors import KNeighborsClassifier\n";
    code += "_X = " + dataVar + ".iloc[:, :-1].values\n";
    code += "_y = " + dataVar + ".iloc[:, -1].values\n";
    code += "_scaler = StandardScaler()\n";
    code += "_X_s = _scaler.fit_transform(_X)\n";
    code += "_X_train, _X_test, _y_train, _y_test = train_test_split(_X_s, _y, test_size=0.2, random_state=42)\n";
    code += "_unique = np.unique(_y)\n";
    code += "_is_clf = len(_unique) <= 20 and all(float(v).is_integer() for v in _unique)\n";
    code += "_candidates = [LogisticRegression(max_iter=1000), DecisionTreeClassifier(), RandomForestClassifier(n_estimators=100), GradientBoostingClassifier(), SVC(probability=True), KNeighborsClassifier()] if _is_clf else [LinearRegression()]\n";
    code += "print()\nprint('\\033[1;36m+' + '='*54 + '+')\n";
    code += "print('|' + '       \U0001f531 SATAN AutoML Engine'.center(54) + '|')\n";
    code += "print('+' + '='*54 + '+\\033[0m')\n";
    code += "print(f'  Task: {\"Classification\" if _is_clf else \"Regression\"} | Features: {_X.shape[1]} | Samples: {len(_y)}')\n";
    code += "print()\n";
    code += "print(f'  {\"Model\":<35} {\"CV Score\":>10} {\"Test Score\":>12} {\"Time\":>10}')\n";
    code += "print('  ' + '-'*70)\n";
    code += "_results = []\n";
    code += "for _m in _candidates:\n";
    code += "    _t = time.perf_counter()\n";
    code += "    _cv = cross_val_score(_m, _X_s, _y, cv=5, scoring='accuracy' if _is_clf else 'r2').mean()\n";
    code += "    _m.fit(_X_train, _y_train)\n";
    code += "    _ts = _m.score(_X_test, _y_test)\n";
    code += "    _elapsed = time.perf_counter() - _t\n";
    code += "    _results.append((_cv, _ts, _m, _elapsed))\n";
    code += "    print(f'  {type(_m).__name__:<35} {_cv:>10.4f} {_ts:>12.4f} {_elapsed*1000:>8.0f}ms')\n";
    code += "_results.sort(key=lambda x: x[0], reverse=True)\n";
    code += "_best_cv, _best_ts, _best_m, _ = _results[0]\n";
    code += "print()\n";
    code += "print(f'\\033[1;32m  Winner: {type(_best_m).__name__:<30} CV: {_best_cv:.4f}  Test: {_best_ts:.4f}\\033[0m')\n";
    code += "print()\n";
    code += winnerVar + " = _best_m\n";
    code += "import os\nos.makedirs('" + sessionDir + "', exist_ok=True)\n";
    code += "with open('" + sessionDir + "/" + winnerVar + ".pkl', 'wb') as f:\n";
    code += "    pickle.dump({'model': _best_m, 'scaler': _scaler, 'X_train': _X_train, 'X_test': _X_test, 'y_train': _y_train, 'y_test': _y_test, 'X': _X, 'y': _y, 'df': " + dataVar + ", 'is_classifier': _is_clf}, f)\n";
    code += "print('__SATAN_RESULT__:' + str(_best_ts))\n";
    return code;
}
