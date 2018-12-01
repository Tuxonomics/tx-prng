# prng-c
[![Build Status](https://travis-ci.org/Tuxonomics/prng-c.svg?branch=master)](https://travis-ci.org/Tuxonomics/prng-c)

PRNG-C provides an interface for pseudo random number generators compatible 
with C (C89 and up) and C++ (C++98 and up). This library is header-only.

Its primary intention is serve statistical simulations. Therefore, this library
includes specific distributions which a random sample can be drawn from. For
now this library will only include univariate distributions.

## Table of Contents
- [Namespacing](#namespacing)
- [The Pseudo Random Number Generator (PRNG) Interface](#prng)
- [List of PRNGs](#list-prngs)
- [List of Univariate Distributions](#univariate)
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
* Advancing to the next state and returning a 64 bit unsigned integer:
`PRNG_Next`.
* Jumping ahead which is equivalent to calling `PRNG_Next` a certain number of
times: `PRNG_Jump`.
* Setting the seed: `PRNG_Seed`.


## [List of PRNGs](#list-prngs)
All PRNGs used here will work on an output of unsigned 64 bit integers. The
state of the PRNGs will be at least 64 bit large.

* `xorshift1024*` ([from](http://vigna.di.unimi.it/ftp/papers/xorshift.pdf)) is 
given as `struct PRNG_Xorshift1024Star`. A state of 1024 bit for massively
parallel applications.

* `xoshiro256**` ([from](http://xoshiro.di.unimi.it/xoshiro256starstar.c)) is
given as `struct PRNG_Xorshiro256StarStar`. A state of 256 bit for moderately
parallel applications.

* `splitmix64` ([from](https://dl.acm.org/citation.cfm?doid=2714064.2660195))
is only used for seeding. It is given as `struct prng_sm64`.


## [List of Univariate Distributions](#univariate)
* Uniform U[0,1] distribution: `PRNG_Uniform`.

* Uniform U(0,1] distribution: `PRNG_UniformPositive`.

* Normal distribution: `PRNG_Normal`.

* Exponential distribution: `PRNG_Exponential`.

* Gamma distribution: `PRNG_Gamma`.

* Accept-reject method for any univariate distribution: 
`PRNG_AcceptRejectSingle` for a single value, `PRNG_AcceptReject` for an array
of random values.

* Bernoulli distribution: `PRNG_Bernoulli`.

* Beta distribution: `PRNG_Beta`.


## [Examples](#examples)
C as well as C++ examples can be found in the `examples` directory.

## [Changelog](#changelog)
- Version 0.1: First version.
