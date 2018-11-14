# prng-c
[![Build Status](https://travis-ci.org/Tuxonomics/prng-c.svg?branch=master)](https://travis-ci.org/Tuxonomics/prng-c)

PRNG-C provides an interface for pseudo random number generators compatible 
with C (C89 and up) and C++ (C++98 and up). This library is header-only.

Its primary intention is serve statistical simulations. Therefore, this library
includes specific distributions which a random sample can be drawn from.

## Table of Contents
- [Namespacing](#namespacing)
- [The Pseudo Random Number Generator (PRNG) Interface](#prng)
- [List of PRNGs](#list-prngs)
- [List of Univariate Distributions](#univariate)
- [List of Multivariate Distributions](#multivariate)
- [Examples](#examples)
- [Changelog](#changelog)
 

## [Namespacing](#namespacing)
This library is namespaced for both C and C++. 

In C all functions and complex
data structures are prefixed by `PRNG_` if it is meant to be a consumer API or
by `prng_` if it is meant to be used internally.

In C++ the namespace `prng` is set. All functions and complex data structures
are written in Pascal case if they are meant for consumer APIs and camelCase if
they are used internally.

In the following only the C namespace is used.


## [The Pseudo Random Number Generator (PRNG) Interface](#prng)
Every PRNG implemented will offer four basic functions:
* Advancing to the next state: `PRNG_Next`.
* Advancing to the next state and converting it to a 64bit float number on the
interval [0,1]: `PRNG_NextFloat`.
* Jumping ahead which is equivalent to calling `PRNG_Next` a certain number of
times: `PRNG_Jump`.
* Setting the seed: `PRNG_Seed`.


## [List of PRNGs](#list-prngs)
All PRNGs used here will work on an output of unsigned 64bit integers. The
state of the PRNGs will be at least 64bit large.

* `xorshift1024*` ([here](http://vigna.di.unimi.it/ftp/papers/xorshift.pdf)) is 
given as `struct PRNG_Xorshift1024Star`.

* `xorshiro256**` ([here](http://xoshiro.di.unimi.it/xoshiro256starstar.c)) is
given as `struct PRNG_Xorshiro256StarStar`.

* `splitmix64` ([here](https://github.com/svaarala/duktape/blob/master/misc/splitmix64.c))
is only used for seeding. It is given as `struct prng_sm64`.


## [List of Univariate Distributions](#univariate)

* Normal distribution with mean 0 and standard deviation 1: `PRNG_BoxMuller`.


## [List of Multivariate Distributions](#multivariate)
... to be done ...


## [Examples](#examples)
C as well as C++ examples can be found in the `examples` directory.

## [Changelog](#changelog)
- Version 0.1: First version.
