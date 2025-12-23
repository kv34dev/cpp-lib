# C++ Library

It is a  C++ library that provides reusable functionality for C++ projects.
The library is designed to be simple, portable, and easy to integrate into existing codebases.

## Compilation

The library can be compiled using a standard C++ compiler.

Example using `g++`:

```
g++ -std=c++17 -Iinclude -c src/mycpplib.cpp
ar rcs libmycpplib.a mycpplib.o
```

```
g++ -std=c++17 -Iinclude main.cpp -L. -lmycpplib -o app
```
