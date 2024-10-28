# NAND Logic Gate Simulation Library

## Overview
This project is a C-based implementation of NAND logic gates and their connections, simulating basic logic circuits with support for signal connections and recursive evaluation. It includes functions for creating, connecting, and evaluating NAND gates in a system, and it uses linked lists to manage output connections between gates.

## Files
- **nand.c / nand.h**: Contains core NAND gate logic, including creation, connection, and evaluation.
- **llist.c / llist.h**: Implements a linked list used for managing gate connections.

## Compilation
The project uses a `Makefile` to manage compilation, linking, and cleanup of the library files. The main compiled output is `libnand.so`, a shared library.
