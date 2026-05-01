# SatanLang Language Specification v2.0

---

## Overview

**SatanLang** is an AI/ML-first programming language built in C++20.  
It combines the simplicity of Python syntax with C++ performance and built-in support for
Machine Learning, Deep Learning, NLP, and Data Science.

---

## 1. Lexical Structure

- **Identifiers:** Begin with letter or `_`, followed by letters, digits, underscores.
- **Keywords:** `let`, `var`, `func`, `fun`, `if`, `else`, `for`, `while`, `return`, `summon`, `print`, `assemble`, `break`, `continue`, `and`, `or`, `not`, `true`, `false`, `import`
- **Comments:**
  - Single-line: `// comment`
  - Multi-line: `/* comment */`

---

## 2. Data Types

| Type | Examples |
|---|---|
| `number` | `42`, `3.14`, `-7` |
| `string` | `"Hello"`, `"world"` |
| `boolean` | `true`, `false` |
| `array` | `[1, 2, 3]`, `["a", "b"]` |
| `object` | ML models, DataFrames |
| `function` | `func add(a, b) { ... }` |
| `nil` | Absence of value |

---

## 3. Variables

```satan
let x = 10;           // immutable
let name = "Satan";
var counter = 0;      // mutable
counter = counter + 1;
```

---

## 4. Operators

- **Arithmetic:** `+`, `-`, `*`, `/`, `%`
- **Comparison:** `==`, `!=`, `<`, `>`, `<=`, `>=`
- **Logical:** `and`, `or`, `not`, `!`
- **String:** `+` (concatenation)

---

## 5. Control Flow

```satan
if (x > 10) {
    summon "big";
} else {
    summon "small";
}

for (var i = 0; i < 5; i = i + 1) {
    summon i;
}

while (counter < 10) {
    counter = counter + 1;
}
```

---

## 6. Functions

```satan
func factorial(n) {
    if (n <= 1) { return 1; }
    return n * factorial(n - 1);
}

summon factorial(10);  // 3628800
```

---

## 7. Arrays

```satan
let arr = [10, 20, 30];
summon arr[0];          // 10
summon arr.length;      // 3
arr.push(40);
summon arr;             // [10, 20, 30, 40]

let nums = range(5);   // [0, 1, 2, 3, 4]
```

---

## 8. Strings

```satan
let s = "Hello World";
summon s.upper();         // HELLO WORLD
summon s.lower();         // hello world
summon s.split(" ");      // ["Hello", "World"]
summon s.length;          // 11
summon s[0];              // H
```

---

## 9. Built-in Functions

### Math
`abs(x)`, `sqrt(x)`, `pow(x, n)`, `round(x)`, `min(a, b)`, `max(a, b)`

### Utility
`len(x)`, `type(x)`, `range(n)`, `str(x)`, `num(x)`

### I/O
`summon expr;`, `print expr;`, `assemble expr;`

---

## 10. AI/ML Functions

### Data Science
```satan
let data = load_csv("file.csv");
data.head(5);
data.describe();
data.shape();
data.corr();       // shows heatmap
data.plot();       // auto-visualize
```

### Machine Learning
```satan
let model = LinearRegression();   // or LogisticRegression, DecisionTree,
                                   // RandomForest, SVM, KMeans, PCA, KNN
model.fit(data);
summon model.score();
model.plot();
```

### Deep Learning
```satan
let net = NeuralNet([784, 128, 10]);
net.train(data, epochs=50, lr=0.01);
net.plot_loss();
```

### NLP
```satan
let s = sentiment("I love this!");
let tokens = tokenize("Hello world");
word_cloud("machine learning deep learning AI");
```

### Plotting
```satan
scatter(x_array, y_array);
histogram(data_array, 30);
```

---

## 11. Named Arguments

Functions support named arguments with `=`:

```satan
net.train(data, epochs=100, lr=0.001);
let model = KMeans(3);
```

---

## 12. CLI Interface

```
satan                   Launch interactive REPL
satan script.satan      Execute a Satan script
satan --version         Version and module info
satan --check           Check Python dependencies
satan --setup-ml        Install ML packages
satan --help            Show help
```
