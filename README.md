# PineappleCAS [![Build Status](https://travis-ci.org/nathanfarlow/PineappleCAS.svg)](https://travis-ci.org/nathanfarlow/PineappleCAS)

A generic CAS targeted for the TI-84+ CE calculators. Designed as a faster, cleaner, more useful, memory-leak-free alternative to the [SymbolicDerivative](https://github.com/nathanfarlow/SymbolicDerivative) project.

PineappleCAS uses the imath library for arbitrary precision math: https://github.com/creachadair/imath

## TODO
- [X] Testing
- [X] Parse expression
- [X] Export expression
- [X] Implement arbitrary precision integer math
  - [X] Addition/Subtraction
  - [X] Multiplication/Division
  - [X] Power
  - [X] Factorial
  - [X] Abs/Int
- [X] Simple factoring A + AB becomes A(B + 1) (no polynomial)
- [X] Expand
- [X] Simplify expression
  - [X] Canonical form
  - [X] Rationals
  - [X] Like terms
  - [X] Identities
- [X] Complex numbers
- [X] Solve derivative
