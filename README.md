# Lambda

A lambda calculus evaluator.

This is a work-in-progress and shouldn't be considered near a complete state. However, it currently does everything I want it to: parses lambda expressions, applies beta-reductions in normal order until impossible, then prints the resuling lambda term.

## Building

Lambda requires nothing more than a C++11 compiler and standard library. Use CMake to generate your favorite build system's files, then build with those. For GNU Make, you could do this from the source directory:

    mkdir build
    cd build
    cmake -G "Unix Makefiles" ../
    make

## Using

After building, you should have an executable called `repl`. Run this; if you want line history, run under `rlwrap`.

The syntax used is pretty much exactly the same as standard lambda calculus, except that abstraction is represented with '^' instead of 'λ'. You may omit parentheses and use shorthand for abstractions of several variables (i.e. you can write `^x y. t` instead of `^x. ^y. t`).

Since the evaluation engine uses normal reduction order, it should always terminate if a normal form for an input exists. However, it doesn't make any attempt to detect infinite loops. If you type `(^x. x x) (^x. x x)` it'll just repeatedly apply the same beta reduction until you kill the process.

## Future steps

I have some features in mind for improving Lambda:

* Do lazy evaluation
* Represent lambda terms with DAGs instead of trees in the evaluation engine (related to doing lazy evaluation)
* Attempt to detect simple infinite loops
* Make lambda term printing more user friendly
* Allow user to define symbols for convenience
