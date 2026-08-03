# 💻 Cache Memory Simulator (C Implementation)

## 📌 Overview
This repository contains a C implementation of a **Cache Memory Simulator**, developed as part of the **Computer Architecture** course at Bar-Ilan University.

The program simulates low-level cache behavior, including set-associative cache mapping, line allocation, byte-level read/write operations, and cache state visualization (hits, misses, and block inspection).

## ✨ Features
* **Cache Initialization (`_initialize_cache`):** Dynamically allocates and configures cache parameters (set count, line size, block size).
* **Read/Write Handling (`_read_byte`, `_write_byte`):** Simulates byte-level access, address breakdown (Tag, Index, Offset), and dirty/valid bit updates.
* **Cache Inspection (`_print_cache`):** Outputs current cache state and memory set mappings for analysis.
* **Memory Management:** Robust dynamic allocation and cleanup routines (`_free_cache`).

## 🛠️ Tech Stack & Concepts
* **Language:** C (C11 / GCC)
* **Build System:** Makefile
* **Concepts:** Computer Architecture, Cache Memory, Set-Associativity, Memory Address Translation (Tag/Index/Offset), Bitwise Manipulation

## 🚀 How to Run
1. Clone the repository:
   ```bash
   git clone [https://github.com/fatimas11/cache-memory-simulator.git](https://github.com/fatimas11/cache-memory-simulator.git)
