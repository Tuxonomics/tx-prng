#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <execinfo.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>


#if 1 // if defined elsewhere
    typedef uint8_t  u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;

    typedef int8_t   i8;
    typedef int16_t  i16;
    typedef int32_t  i32;
    typedef int64_t  i64;

    typedef float    f32;
    typedef double   f64;

    typedef i8       b8;
    typedef i32      b32;
#endif


#if 1 // if defined elsewhere
#if defined(_MSC_VER)
    #if _MSC_VER < 1300
        #define DEBUG_TRAP() __asm int 3
    #else
        #define DEBUG_TRAP() __debugbreak()
    #endif
#else
    #define DEBUG_TRAP() __builtin_trap()
#endif
    
#ifndef TEST
#if !defined(RELEASE) && !defined(ASSERTS)
    #define ASSERT_MSG_VA(cond, msg, ...) do { \
        if (!(cond)) { \
        assertHandler(__FILE__, (i32)__LINE__, msg, __VA_ARGS__); \
        DEBUG_TRAP(); \
        } \
        } while(0)

    #define ASSERT_MSG(cond, msg) ASSERT_MSG_VA(cond, msg, 0)

    #define ASSERT(cond) ASSERT_MSG_VA(cond, 0, 0)
    #define PANIC(msg) ASSERT_MSG_VA(0, msg, 0)
    #define UNIMPLEMENTED() ASSERT_MSG_VA(0, "unimplemented", 0);
#else
    #define ASSERT_MSG_VA(cond, msg, ...)
    #define ASSERT_MSG(cond, msg)
    #define ASSERT(cond)
    #define PANIC(msg)
    #define UNIMPLEMENTED()
#endif
#endif
    
    
#if !defined(Inline)
    #if defined(_MSC_VER)
        #if _MSC_VER < 1300
            #define Inline
        #else
            #define Inline __forceinline
        #endif
    #else
        #define Inline __attribute__ ((__always_inline__))
    #endif
#endif
    
    
#if !defined(_Threadlocal)
    #if defined(_MSC_VER)
        #define _Threadlocal __declspec( thread )
    #else
        #define _Threadlocal __thread
    #endif
#endif
    
    
void Backtrace() {
#define BACKTRACE_MAX_STACK_DEPTH 50
#if SYSTEM_POSIX
    void* callstack[BACKTRACE_MAX_STACK_DEPTH];
    int i, frames = backtrace(callstack, BACKTRACE_MAX_STACK_DEPTH);
    char** strs = backtrace_symbols(callstack, frames);
    for (i = 0; i < frames; ++i) {
        fprintf(stderr, "%s\n", strs[i]);
    }
    free(strs);
#elif SYSTEM_WINDOWS
    UNIMPLEMENTED();
#endif
}
    
void assertHandler(char const *file, i32 line, char const *msg, ...) {
    va_list args;
    va_start(args, msg);
    Backtrace();
    
    if (msg) {
        fprintf(stderr, "Assert failure: %s:%d: ", file, line);
        vfprintf(stderr, msg, args);
        fprintf(stderr, "\n");
    } else {
        fprintf(stderr, "Assert failure: %s:%d\n", file, line);
    }
    va_end(args);
}
#endif


#ifdef __cplusplus
namespace prng {
#endif
    
#ifndef __cplusplus
    #define PF(name) PRNG_##name
#else
    #define PF(name) name
#endif

    
#ifndef __cplusplus
    #define PF2(name) prng_##name
#else
    #define PF2(name) name
#endif


#define ARRAY_SELECTION( selection, sign ) \
    f64 PF2(Array##selection)( f64 *array, u64 length ) \
    { \
        f64 tmp; \
        f64 output = array[0]; \
        for ( u64 i=1; i<length; ++i ) { \
            if ( array[i] sign output ) { \
                output = array[i]; \
            } \
        } \
        return output; \
    }


ARRAY_SELECTION(Max, > );
ARRAY_SELECTION(Min, > );

#undef ARRAY_SELECTION


f64 PF2(ArrayMean)( f64 *array, u64 length )
{
    f64 sum = 0.0;
    for ( u64 i=1; i<length; ++i ) {
        sum += array[i];
    }
    return sum / (f64) length;
}


f64 PF2(ArrayVariance)( f64 *array, u64 length )
{
    f64 mean = PF2(ArrayMean)( array, length );
    f64 tmp, sum  = 0.0;
    for ( u64 i=1; i<length; ++i ) {
        tmp = ( array[i] - mean );
        sum += tmp * tmp;
    }
    return sum / (f64) (length - 1);
}


b32 PF2(f64Equal)( f64 a, f64 b, f64 eps )
{
    return fabs( a - b ) < eps;
}


Inline
u64 PF2(rotl)(const u64 x, i32 k) {
    return (x << k) | (x >> (64 - k));
}


#define PRNG_NEXT_FUNC(name) Inline u64 name(void *state)
typedef u64 PRNG_nextFunc(void *state);

#define PRNG_JUMP_FUNC(name) void name(void *state)
typedef void PRNG_jumpFunc(void *state);

#define PRNG_SEED_FUNC(name) void name(void *state, u64 seed)
typedef void PRNG_seedFunc(void *state, u64 seed);

    
#ifdef __cplusplus
typedef struct PF(Generator) Generator;
#endif

struct PF(Generator) {
    void *state;

    PRNG_nextFunc      *next;
    PRNG_jumpFunc      *jump;
    PRNG_seedFunc      *seed;
};

#define PRNG struct PF(Generator)


Inline
u64 PF(Next)( PRNG g )
{
    return g.next( g.state );
}


Inline
void PF(Jump)( PRNG g )
{
    g.jump( g.state );
}


Inline
void PF(Seed)( PRNG g, u64 seed )
{
    g.seed( g.state, seed );
}


u64 PF(SeedValue)( void )
{
    return (u64) time( NULL );
}


// The SplitMix64 PRNG.
// https://github.com/svaarala/duktape/blob/master/misc/splitmix64.c
// Seed with any value and use to seed other PRNGs.
struct PF2(sm64) {
    u64 s;
};

#define SMIX struct PF2(sm64)


u64 PF2(sm64Next)( SMIX *state )
{
    u64
    z = (state->s += UINT64_C(0x9E3779B97F4A7C15));
    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
    return z ^ (z >> 31);
}


/* Convert u64 to uniform f64. See http://xoshiro.di.unimi.it.
 This will cut the number of possible values in half as the lowest significant
 bit will be set to 0 for all returned values. */
f64 PF2(toFloat)(u64 x)
{
    const union { u64 i; f64 d; }
    u = { .i = UINT64_C(0x3FF) << 52 | x >> 12 };
    return u.d - 1.0;
}



/* xorshift1024*, see http://vigna.di.unimi.it/ftp/papers/xorshift.pdf */

#ifdef __cplusplus
typedef struct PF(Xorshift1024Star) Xorshift1024Star;
#endif

struct PF(Xorshift1024Star) {
    u64 s[16];
    i32 p;
};
    
#define XORSHIFT struct PF(Xorshift1024Star)


PRNG_NEXT_FUNC(PF2(Xorshift1024StarNext))
{
    XORSHIFT *x = (XORSHIFT *) state;
    
    const u64 s0 = x->s[x->p];
    u64       s1 = x->s[ x->p = (x->p + 1) & 15 ];
    
    s1 ^= s1 << 31;
    
    x->s[x->p] = s1 ^ s0 ^ (s1 >> 11) ^ (s0 >> 30);
    
    return x->s[x->p] * UINT64_C(1181783497276652981);
}


// NOTE(jonas): the jump is equivalent to 2^512 calls to next
PRNG_JUMP_FUNC(PF2(Xorshift1024StarJump))
{
    XORSHIFT *x = (XORSHIFT *) state;
    
    static const u64 JUMP[] = {
        0x84242f96eca9c41d, 0xa3c65b8776f96855, 0x5b34a39f070b5837,
        0x4489affce4f31a1e, 0x2ffeeb0a48316f40, 0xdc2d9891fe68c022,
        0x3659132bb12fea70, 0xaac17d8efa43cab8, 0xc4cb815590989b13,
        0x5ee975283d71c93b, 0x691548c86c1bd540, 0x7910c41d10a1e6a5,
        0x0b5fc64563b3e2a8, 0x047f7684e9fc949d, 0xb99181f2d8f685ca,
        0x284600e3f30e38c3
    };
    
    u64 t[16] = { 0 };
    
    for ( i32 i = 0; i < ( sizeof(JUMP) / sizeof(*JUMP) ); ++i ) {
        for (i32 b = 0; b < 64; ++b) {
            if ( JUMP[i] & 1ULL << b ) {
                for (i32 j = 0; j < 16; ++j ) {
                    t[j] ^= x->s[(j + x->p) & 15];
                }
            }
            PF2(Xorshift1024StarNext( state ));
        }
    }
    
    for ( i32 j = 0; j < 16; j++ ) {
        x->s[(j + x->p) & 15] = t[j];
    }
}


PRNG_SEED_FUNC(PF2(Xorshift1024StarSeed))
{
    XORSHIFT *x = (XORSHIFT *) state;
    
    SMIX sp = (SMIX) { .s = seed };
    
    for ( u32 i=0; i<16; ++i ) {
        x->s[i] = PF2(sm64Next)( &sp );
    }
    
    x->p = 0;
}

    
PRNG PF(InitXorshift1024Star)( XORSHIFT *state, u64 seed )
{
    PF2(Xorshift1024StarSeed)( state, seed );
    
    PRNG g;
    g.next      = PF2(Xorshift1024StarNext);
    g.jump      = PF2(Xorshift1024StarJump);
    g.seed      = PF2(Xorshift1024StarSeed);
    g.state     = state;
    return g;
}


#if TEST
void test_xorshift1024_star()
{
    XORSHIFT x;
    PF2(Xorshift1024StarSeed)( &x, 37473 );
    
    TEST_ASSERT( PF2(Xorshift1024StarNext)( &x ) > 0 );
    
    PF2(Xorshift1024StarJump)( &x);
    
    TEST_ASSERT( PF2(Xorshift1024StarNext)( &x ) > 0 );
}
#endif

#undef XORSHIFT

    
// xoshiro256** http://xoshiro.di.unimi.it/xoshiro256starstar.c

#ifdef __cplusplus
typedef struct PF(Xoshiro256StarStar) Xoshiro256StarStar;
#endif

struct PF(Xoshiro256StarStar) {
    u64 s[4];
};

#define XOSHIRO struct PF(Xoshiro256StarStar)


PRNG_NEXT_FUNC(PF2(Xoshiro256StarStarNext))
{
    XOSHIRO *x = (XOSHIRO *) state;
    
    const u64 res = PF2(rotl)(x->s[1] * 5, 7) * 9;
    
    const u64 t = x->s[1] << 17;
    
    x->s[2] ^= x->s[0];
    x->s[3] ^= x->s[1];
    x->s[1] ^= x->s[2];
    x->s[0] ^= x->s[3];
    
    x->s[2] ^= t;
    
    x->s[3]  = PF2(rotl)(x->s[3], 45);
    
    return res;
}


// NOTE(jonas): the jump is equivalent to 2^128 calls to next
PRNG_JUMP_FUNC(PF2(Xoshiro256StarStarJump))
{
    XOSHIRO *x = (XOSHIRO *) state;
    
    static const u64 JUMP[] = {
        0x180ec6d33cfd0aba, 0xd5a61266f0c9392c,
        0xa9582618e03fc9aa, 0x39abdc4529b1661c
    };
    
    u64 s0 = 0;
    u64 s1 = 0;
    u64 s2 = 0;
    u64 s3 = 0;
    for ( i32 i = 0; i < sizeof( JUMP ) / sizeof( *JUMP ); ++i ) {
        for ( i32 b = 0; b < 64; ++b ) {
            if ( JUMP[i] & UINT64_C(1) << b ) {
                s0 ^= x->s[0];
                s1 ^= x->s[1];
                s2 ^= x->s[2];
                s3 ^= x->s[3];
            }
            PF2(Xoshiro256StarStarNext)( state );
        }
    }
    
    x->s[0] = s0;
    x->s[1] = s1;
    x->s[2] = s2;
    x->s[3] = s3;
}


PRNG_SEED_FUNC(PF2(Xoshiro256StarStarSeed))
{
    XOSHIRO *x = (XOSHIRO *) state;
    
    SMIX sp = (SMIX) { .s = seed };
    
    for ( u32 i=0; i<4; ++i ) {
        x->s[i] = PF2(sm64Next)( &sp );
    }
}


PRNG PF(InitXoshiro256StarStar)( XOSHIRO *state, u64 seed )
{
    PF2(Xoshiro256StarStarSeed)( state, seed );
    
    PRNG g;
    g.next      = PF2(Xoshiro256StarStarNext);
    g.jump      = PF2(Xoshiro256StarStarJump);
    g.seed      = PF2(Xoshiro256StarStarSeed);
    g.state     = state;
    return g;
}
    
    
#if TEST
void test_xoshiro256_star_star()
{
    XOSHIRO x;
    PF2(Xoshiro256StarStarSeed)( &x, 37473 );
    
    TEST_ASSERT( PF2(Xoshiro256StarStarNext)( &x ) > 0 );
    
    PF2(Xoshiro256StarStarJump)( &x);
    
    TEST_ASSERT( PF2(Xoshiro256StarStarNext)( &x ) > 0 );
}
#endif
    
#undef XORSHIRO


#if TEST
void test_generator()
{
#define STATE struct PRNG_Xoshiro256StarStar
#define RNG struct PRNG_Generator

    u64 seed = PRNG_SeedValue();
    
    STATE state;
    RNG rng = PRNG_InitXoshiro256StarStar( &state, seed );

    PRNG_Jump( rng );
    
    TEST_ASSERT( PRNG_Next( rng ) > 0 );
    
#undef STATE
#undef RNG
}
#endif


// Uniform U[0,1] random number
Inline
f64 PF(Uniform)( PRNG g )
{
    return PF2(toFloat)( PF(Next)( g ) );
}


Inline
f64 PF(UniformPositive)( PRNG g )
{
    f64 res;
    do {
        res = PF(Uniform)( g );
    } while( res == 0 );
    return res;
}


#if TEST
void test_uniform()
{
#define STATE struct PRNG_Xoshiro256StarStar
#define RNG struct PRNG_Generator

    STATE state;
    RNG rng = PF(InitXoshiro256StarStar)( &state, 3747 );
    
    TEST_ASSERT( PRNG_Uniform( rng ) <= 1.0 );

#undef STATE
#undef RNG
}
#endif


// Standard normal random number with Box-Muller transform.
// https://en.wikipedia.org/wiki/Box–Muller_transform .*/

f64 PF(Normal)( PRNG g )
{
    f64 u = PF(Uniform)( g );
    f64 v = PF(Uniform)( g );
    
    f64 s = sqrt( -2.0 * log( u ) );
    f64 t = cos( 2.0 * M_PI * v );
    
    return s * t;
}


f64 PF(NormalPDF)( f64 x, f64 m, f64 s )
{
    f64 tmp1 = 2.0 * s * s;
    f64 tmp2 = x - m;

    f64 p1 = 1.0 / sqrt( M_PI * tmp1 );
    f64 p2 = exp( - tmp2 * tmp2 / tmp1 );

    return p1 * p2;
}


#if TEST
void test_normal()
{
#define N 1000

    f64 array[N];
    
    struct PF(Xorshift1024Star) state;
    PRNG rng = PF(InitXorshift1024Star)( &state, 3747 );
    
    for ( u32 i=0; i<N; ++i ) {
        array[i] = PF(Normal)( rng );
    }
    
    f64 mean = PF2(ArrayMean)( array, N );
    f64 var  = PF2(ArrayVariance)( array, N );
    
    TEST_ASSERT( PF2(f64Equal)( mean, 0,   0.01 ) );
    TEST_ASSERT( PF2(f64Equal)( var,  1.0, 0.1 ) );

    f64 pdf = PF(NormalPDF)( 1.0, 1.0, 1.0 );

    TEST_ASSERT( PF2(f64Equal)( pdf, 0.3989423, 1E-6 ) );
#undef N
}
#endif


f64 PF(Exponential)( PRNG g, f64 lambda )
{
    ASSERT( lambda > 0 );
    
    f64 u = PF(UniformPositive)( g );

    return - log( u ) / lambda;
}


f64 PF(ExponentialPDF)( f64 x, f64 lambda )
{
    return lambda * exp( - lambda * x );
}


#if TEST
void test_exponential()
{
#define STATE struct PRNG_Xoshiro256StarStar
#define RNG struct PRNG_Generator

#define N 1000

    f64 array[N];
    
    STATE state;
    RNG rng = PF(InitXoshiro256StarStar)( &state, 3547 );
    
    f64 lambda = 3.5;
    
    for ( u32 i=0; i<N; ++i ) {
        array[i] = PF(Exponential)( rng, lambda );
    }
    
    f64 mean = PF2(ArrayMean)( array, N );
    f64 var  = PF2(ArrayVariance)( array, N );

    TEST_ASSERT( PF2(f64Equal)( mean, 1.0/lambda,            0.01 ) );
    TEST_ASSERT( PF2(f64Equal)( var,  1.0/(lambda * lambda), 0.01 ) );

    f64 pdf = PF(ExponentialPDF)( 0.45, lambda );

    TEST_ASSERT( PF2(f64Equal)( pdf, 0.7245264, 1E-6 ) );

#undef STATE
#undef RNG
#undef N
}
#endif


typedef f64 PF2(arPDF)( f64 x );
typedef f64 PF2(arProposal)( PRNG g );


Inline
b32 PF2(arRatio)
(
    PRNG            g,
    PF2(arProposal) *proposal,
    PF2(arPDF)      *targetPDF,
    PF2(arPDF)      *proposalPDF,
    f64             c,
    f64             *targetValue )
{
#define x *targetValue
    f64 u = PF(Uniform)( g );
    x     = proposal( g );

    f64 ratio = targetPDF( x ) / ( c * proposalPDF( x ) );

    return( u < ratio );
#undef x
}


f64 PF(AcceptRejectSingle)(
    PRNG            g,
    PF2(arProposal) *proposal,
    PF2(arPDF)      *targetPDF,
    PF2(arPDF)      *proposalPDF,
    f64             c )
{
    b32 accept;
    f64 x;
    for ( u32 i=0; i<1E3; ++i ) {
        accept = PF2(arRatio)( g, proposal, targetPDF, proposalPDF, c, &x );
        if ( accept ) {
            return x;
        }
    }
    return NAN;
}


void PF(AcceptReject)(
    PRNG            g,
    PF2(arProposal) *proposal,
    PF2(arPDF)      *targetPDF,
    PF2(arPDF)      *proposalPDF,
    f64             c,
    f64             *targetVals,
    u64             targetSize )
{
    b32 accept;
    f64 x;
    u64 i = 0;
    while ( i<targetSize ) {
        accept = PF2(arRatio)( g, proposal, targetPDF, proposalPDF, c, &x );
        if ( accept ) {
            targetVals[i] = x;
            i += 1;
        }
    }
}


#if TEST
void test_accept_reject()
{
    printf("... unimplemented ...\n");
}
#endif


/*
Marsaglia and Tsang (2000) "A Simple Method for
generating gamma variables". https://dl.acm.org/citation.cfm?id=358414

Can be made faster with Ziggurat as in GSL.
*/

f64 PF(Gamma)( PRNG g, f64 k, f64 theta )
{
    f64 d, c, x, v, u;
    d = k - (1.0/3.0);
    c = 1.0 / (3.0 * sqrt(d));

    while( 1 ) {
        do {
            x = PF(Normal)( g );
            v = 1.0 + c * x;
        } while ( v <= 0.0 );

        v = v * v * v;
        u = PF(UniformPositive)( g );

        if ( u < 1.0 - (0.0331 * x * x * x * x) ) {
            break;
        }
        if ( log(u) < 0.5 * x * x + d * (1.0 - v + log(v)) ) {
            break;
        }
    }
    return theta * d * v;
}


/*
http://compbio.mit.edu/spimap/pub/spimap/src/gamma.cpp
http://www.rskey.org/CMS/index.php/the-library/11
from "Numerical Recipes"
*/

f64 PF2(gammln)( f64 xx )
{
    f64 x, tmp, y, ser;
    static const f64 cof[6] = {
        76.18009172947146,     -86.50532032941677,
        24.01409824083091,     -1.231739572450155,
        0.1208650973866179e-2, -0.5395239384953e-5
    };

    x   = xx - 1.0;

    tmp  = x + 5.5;
    tmp -= ( x + 0.5 ) * log( tmp );

    ser = 1.000000000190015;

    for ( u32 j=0; j<=5; ++j ) {
        x   += 1.0;
        ser += cof[j]/x;
    }
    return -tmp+log(2.5066282746310005*ser);
}


f64 PF(GammaLPDF)( f64 x, f64 k, f64 theta )
{
    if ( x <= 0 || k <= 0 || theta <= 0 ) {
        return 0.0;
    } else {
        return -x/theta + (k - 1.0) * log(x) - k * log(theta) - PF2(gammln)(k);
    }
}


f64 PF2(gamm)( f64 x )
{
    double ret = (
        1.000000000190015 +
        76.18009172947146 / (x + 1) +
        -86.50532032941677 / (x + 2) +
        24.01409824083091 / (x + 3) +
        -1.231739572450155 / (x + 4) +
        1.208650973866179e-3 / (x + 5) +
        -5.395239384953e-6 / (x + 6)
    );

    return ret * sqrt(2*M_PI)/x * pow(x + 5.5, x+0.5) * exp(-x-5.5);
}


f64 PF(GammaPDF)( f64 x, f64 k, f64 theta )
{
    if ( x <= 0 || k <= 0 || theta <= 0 ) {
        return 0.0;
    } else {
        return exp(-x/theta) * pow(x, k - 1.0) * pow(theta, -k) / PF2(gamm)(k);
    }
}


#if TEST
void test_gamma()
{
#define STATE struct PRNG_Xoshiro256StarStar
#define RNG struct PRNG_Generator

#define N 1000

    f64 array[N];
    
    STATE state;
    RNG rng = PF(InitXoshiro256StarStar)( &state, 37 );
    
    f64 k     = 1.0;
    f64 theta = 3.0;
    
    for ( u32 i=0; i<N; ++i ) {
        array[i] = PF(Gamma)( rng, k, theta );
    }
    
    f64 mean = PF2(ArrayMean)( array, N );
    f64 var  = PF2(ArrayVariance)( array, N );

    TEST_ASSERT( PF2(f64Equal)( mean, k*theta,           0.1 ) );
    TEST_ASSERT( PF2(f64Equal)( var,  k*(theta * theta), 0.2 ) );

    f64 pdf  = PF(GammaPDF)( 0.5, k, theta );
    f64 lpdf = PF(GammaLPDF)( 0.5, k, theta );

    TEST_ASSERT( PF2(f64Equal)( log(pdf), lpdf,      1E-6 ) );
    TEST_ASSERT( PF2(f64Equal)( pdf,      0.2821606, 1E-6 ) );

#undef STATE
#undef RNG
#undef N
}
#endif


u32 PF(Bernoulli)( PRNG g, f64 p )
{
    if ( PF(Uniform)( g ) < p ) {
        return 1;
    } else {
        return 0;
    }
}


#if TEST
void test_bernoulli()
{
#define STATE struct PRNG_Xoshiro256StarStar
#define RNG struct PRNG_Generator

#define N 1000

    f64 array[N];

    STATE state;
    RNG rng = PF(InitXoshiro256StarStar)( &state, 37 );

    f64 p = 0.75;

    for ( u32 i=0; i<N; ++i ) {
        array[i] = (f64) PF(Bernoulli)( rng, p );
    }

    f64 mean = PF2(ArrayMean)( array, N );

    TEST_ASSERT( PF2(f64Equal)( mean, p, 0.01 ) );

#undef STATE
#undef RNG
#undef N

}
#endif


/*
M. D. Joehnk (1964) "Erzeugung von betaverteilten und
gammaverteilten Zufallsvariablen" 

and

Donald E. Knuth (1997) "The Art of Computer Programming:
Seminumerical Algorithms", p. 134
*/


f64 PF(Beta)( PRNG g, f64 a, f64 b )
{
    if ( a > 1.0 || b > 1.0 ) {
        f64 x1 = PF(Gamma)( g, a, 1.0 );
        f64 x2 = PF(Gamma)( g, b, 1.0 );
        return ( x1 / (x1 + x2) );
    }
    else {
        f64 r1, r2, x, y;
        while( 1 ) {
            r1 = PF(UniformPositive)( g );
            r2 = PF(UniformPositive)( g );

            x = pow( r1, 1.0/a );
            y = pow( r2, 1.0/b );

            if ( (x + y) <= 1.0 ) {
                return ( x / (x + y) );
            }
        }
    }
}


#if TEST
void test_beta()
{
#define STATE struct PRNG_Xoshiro256StarStar
#define RNG struct PRNG_Generator

#define N 1000

    f64 array[N];

    STATE state;
    RNG rng = PF(InitXoshiro256StarStar)( &state, 371 );

    f64 a = 1.0;
    f64 b = 1./3.;

    for ( u32 i=0; i<N; ++i ) {
        array[i] = (f64) PF(Beta)( rng, a, b );
    }

    f64 mean = PF2(ArrayMean)( array, N );
    f64 var  = PF2(ArrayVariance)( array, N );

    TEST_ASSERT( PF2(f64Equal)( mean, a/(a+b), 0.01 ) );
    TEST_ASSERT( PF2(f64Equal)( var,  a*b/((a+b)*(a+b)*(a+b+1.0)), 0.01 ) );

    a = 1.5;
    b = 1.2;

    for ( u32 i=0; i<N; ++i ) {
        array[i] = (f64) PF(Beta)( rng, a, b );
    }

    mean = PF2(ArrayMean)( array, N );
    var  = PF2(ArrayVariance)( array, N );

    TEST_ASSERT( PF2(f64Equal)( mean, a/(a+b), 0.01 ) );
    TEST_ASSERT( PF2(f64Equal)( var,  a*b/((a+b)*(a+b)*(a+b+1.0)), 0.001 ) );

#undef STATE
#undef RNG
#undef N

}
#endif


#undef PF
#undef PF2
#undef SMIX
#undef PRNG

#undef PRNG_NEXT_FUNC
#undef PRNG_NEXT_FLOAT_FUNC
#undef PRNG_JUMP_FUNC
#undef PRNG_SEED_FUNC


#ifdef __cplusplus
} // prng namespace
}
#endif

