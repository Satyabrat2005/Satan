# SatanLang Language Specification (Draft)

---

## Overview

**SatanLang** is a domain-specific programming language designed primarily for data science and machine learning workflows.  
It aims to combine the simplicity of Python with high performance and rich built-in libraries.

---

## 1. Lexical Structure

- **Identifiers:** Must begin with a letter or underscore (`_`), followed by letters, digits, or underscores.  
  Examples: `x`, `_temp`, `data123`  
- **Keywords:** Reserved words include `let`, `var`, `summon`, `if`, `else`, `for`, `while`, `func`, `return`, etc.  
- **Comments:**  
  - Single-line comments start with `//`  
  - Multi-line comments are enclosed with `/* ... */`

---

## 2. Data Types

- **Primitive Types:**  
  - `int` (e.g., `42`)  
  - `float` (e.g., `3.14`)  
  - `string` (e.g., `"Hello"`)  
  - `bool` (`true`, `false`)  

- **Composite Types:**  
  - `array` (homogeneous sequences)  
  - `dataframe` (tabular data structure) [planned]  

---

## 3. Variables and Assignment

- Variables declared using `let` keyword:  

  ```satan
  let x = 10
  let name = "Satan"
  let isReady = true


**Variables are immutable by default, but can be declared mutable with var:**

var counter = 0
counter = counter + 1

## 4. Expressions and Operators
**Arithmetic operators: +, -, *, /, %**

**Comparison operators: ==, !=, <, >, <=, >=**

**Logical operators: && (and), || (or), ! (not)**

let result = (x + 5) * 3 > 20 && isReady

## 5. Control Flow
**If-Else**

if x > 10 {
    summon "x is greater than 10"
} else {
    summon "x is less or equal to 10"
}

**For Loop:**

for i in 0..5 {
    summon i
}

**While Loop:**

while counter < 10 {
    counter = counter + 1
}

## 6. Functions

**Declare functions with func keyword:**

func add(a, b) {
    return a + b
}

let sum = add(5, 7)
summon sum

## 7. Built-in I/O

**Print to console using summon:**

summon "Hello, SatanLang!"
summon "Value of x is: " + x
