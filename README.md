<p align="center"><img src="https://raw.githubusercontent.com/nathanfarlow/PineappleCAS/master/img/logo.png"></p>
<h2 align="center"><b>PineappleCAS</b></h2>
<p align="center">
<a href="https://travis-ci.org/nathanfarlow/PineappleCAS" alt="Build Status"><img src="https://travis-ci.org/nathanfarlow/PineappleCAS.svg"></a>
</p>
<p>
PineappleCAS is a computer algebra system for the TI-84 Plus CE calculators. It is designed as a faster, cleaner, more useful, memory-leak-free alternative to the <a href="https://github.com/nathanfarlow/SymbolicDerivative">SymbolicDerivative</a> project. PineappleCAS uses the <a href="https://github.com/creachadair/imath">imath library</a> for arbitrary precision math: 
</p>

<hr>

## Screenshots
![Main screen](https://raw.githubusercontent.com/nathanfarlow/PineappleCAS/master/img/simplify.png "GUI")
![Complex example](https://raw.githubusercontent.com/nathanfarlow/PineappleCAS/master/img/i^i.png "Complex simplification")
![Trig example](https://raw.githubusercontent.com/nathanfarlow/PineappleCAS/master/img/trig.png "Trig identity simplification")
![Derivative example](https://raw.githubusercontent.com/nathanfarlow/PineappleCAS/master/img/deriv.png "Derivative with respect to X")
![Exponent example](https://raw.githubusercontent.com/nathanfarlow/PineappleCAS/master/img/eval_exponent.png "Large exponent")
![Factorial example](https://raw.githubusercontent.com/nathanfarlow/PineappleCAS/master/img/eval_factorial.png "Large factorial")
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
