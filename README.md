# hrLib
ho-ri1991's utility library.
See test codes for the usage of this library.
- `include/hrlib/type_traits` some type meta functions
- `include/hrlib/error_handling` some utility classes for error handling
  - `Result`: monad like object which is similar to the Result in Rust and support the sequence operation. This class has monadic interface but the sequence operation is applicative like.
- `include/hrlib/integer_sequence_util` some utility (meta) functions for std::integer_sequence class

## requirement
- C++ compiler and standard library >= C++17
- boost >= 1.62.0

We have compiled by g++7.2.0 with boost 1.62.0
