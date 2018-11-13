# prng-c
PRNG-C provides an interface for pseudo random number generators compatible 
with C (C89 and up) and C++ (C++98 and up). This library is header-only.

Its primary intention is serve statistical simulations. Therefore, this library
includes specific distributions which a random sample can be drawn from.


## Namespacing
This library is namespaced for both C and C++. 

In C all functions and complex
data structures are prefixed by `PRNG_` if it is meant to be a consumer API or
by `prng_` if it is meant to be used internally.

In C++ the namespace `prng` is set. All functions and complex data structures
are written in **PascalCase** if they are meant for consumer APIs and **camelCase** if
they are used internally.

In the following only the C namespace is used.


## List of Pseudo Random Number Generators (PRNGs)

* `xorshift1024*` ([here](http://vigna.di.unimi.it/ftp/papers/xorshift.pdf)) is 
given as `struct PRNG_Xorshift1024Star`.

* `splitmix64` ([here](https://github.com/svaarala/duktape/blob/master/misc/splitmix64.c))
is only used for seeding. It is given as `struct prng_sm64`.


## List of Univariate Distributions

* Normal distribution with mean 0 and standard deviation 1: `PRNG_BoxMuller`.


## List of Multivariate Distributions
... to be done ...


## Examples
[C](https://github.com/Tuxonomics/prng-c/blob/master/examples/example_1.c) as well as [C++](https://github.com/Tuxonomics/prng-c/blob/master/examples/example_1.cpp) examples can be found in the `examples` directory.

