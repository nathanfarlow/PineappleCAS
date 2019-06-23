# PineappleCAS [![Build Status](https://travis-ci.org/nathanfarlow/PineappleCAS.svg)](https://travis-ci.org/nathanfarlow/PineappleCAS)

A generic CAS targeted for the TI-84+ CE calculators. Designed as a faster, cleaner, more useful, memory-leak-free alternative to the [SymbolicDerivative](https://github.com/nathanfarlow/SymbolicDerivative) project.

PineappleCAS uses the imath library for arbitrary precision math: https://github.com/creachadair/imath

## TODO
- [X] Parse expression
- [X] Export expression
- [X] Implement arbitrary precision integer math
  - [X] Addition/Subtraction
  - [X] Multiplication/Division
  - [X] Power
  - [X] Factorial
  - [X] Abs/Int
- [ ] Factor
  - [X] Simple A + AB becomes A(B + 1)
  - [ ] Polynomial x^3 + 2x^2 - 5x - 6 becomes (x + 1)(x + 3)(x - 2)
- [X] Expand
- [ ] Simplify expression
  - [X] Canonical form
  - [X] Rationals
  - [X] Like terms
  - [ ] Identities
- [ ] Solve derivative
- [ ] Solve limits
- [ ] Integrals? (Probably not)
