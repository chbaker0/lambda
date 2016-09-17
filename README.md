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

Here is a quick usage sample:

    >> (^x y. y x) a b
    ((^x y . (y x)) a b)

    (((^x. (^y. (_1 _2))) a) b)

    ((^y. (_1 a)) b)

    (b a)

    >> (^p. p (^a b. b)) ((^a b p. p a b) x y)
    ((^p . (p (^a b . b))) ((^a b p . (p a b)) x y))

    ((^p. (_1 (^a. (^b. _1)))) (((^a. (^b. (^p. ((_1 _3) _2)))) x) y))

    ((((^a. (^b. (^p. ((_1 _3) _2)))) x) y) (^a. (^b. _1)))

    (((^b. (^p. ((_1 x) _2))) y) (^a. (^b. _1)))

    ((^p. ((_1 x) y)) (^a. (^b. _1)))

    (((^a. (^b. _1)) x) y)

    ((^b. _1) y)

    y

    >> (^n f z. f (n f z)) (^f z. f(f(z)))
    ((^n f z . (f (n f z))) (^f z . (f (f z))))

    ((^n. (^f. (^z. (_2 ((_3 _2) _1))))) (^f. (^z. (_2 (_2 _1)))))

    (^f. (^z. (_2 (((^f. (^z. (_2 (_2 _1)))) _2) _1))))

    (^f. (^z. (_2 ((^z. (_2 (_2 _1))) _1))))

    (^f. (^z. (_2 (_2 (_2 _1)))))

    >> (^x y. x) (^x. x) ((^x. x x) (^x. x x))
    ((^x y . x) (^x . x) ((^x . (x x)) (^x . (x x))))

    (((^x. (^y. _2)) (^x. _1)) ((^x. (_1 _1)) (^x. (_1 _1))))

    ((^y. (^x. _1)) ((^x. (_1 _1)) (^x. (_1 _1))))

    (^x. _1)

Note the `_1`, `_2`, etc; internally, the evaluation engine uses [De Bruijn indices](https://en.wikipedia.org/wiki/De_Bruijn_index) to avoid doing alpha conversions. For example, `(^x. ^y. x)` = `(^x. ^y. _2)` and `(^x. ^y. y)` = `(^x. ^y. _1)`.

## Future steps

I have some features in mind for improving Lambda:

* Do lazy evaluation
* Represent lambda terms with DAGs instead of trees in the evaluation engine (related to doing lazy evaluation)
* Attempt to detect simple infinite loops
* Make lambda term printing more user friendly
* Allow user to define symbols for convenience
