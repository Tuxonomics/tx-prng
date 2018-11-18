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

    
// xorshiro256** http://xoshiro.di.unimi.it/xoshiro256starstar.c

#ifdef __cplusplus
typedef struct PF(Xorshiro256StarStar) Xorshiro256StarStar;
#endif

struct PF(Xorshiro256StarStar) {
    u64 s[4];
};

#define XORSHIRO struct PF(Xorshiro256StarStar)


PRNG_NEXT_FUNC(PF2(Xorshiro256StarStarNext))
{
    XORSHIRO *x = (XORSHIRO *) state;
    
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
PRNG_JUMP_FUNC(PF2(Xorshiro256StarStarJump))
{
    XORSHIRO *x = (XORSHIRO *) state;
    
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
            PF2(Xorshiro256StarStarNext)( state );
        }
    }
    
    x->s[0] = s0;
    x->s[1] = s1;
    x->s[2] = s2;
    x->s[3] = s3;
}


PRNG_SEED_FUNC(PF2(Xorshiro256StarStarSeed))
{
    XORSHIRO *x = (XORSHIRO *) state;
    
    SMIX sp = (SMIX) { .s = seed };
    
    for ( u32 i=0; i<4; ++i ) {
        x->s[i] = PF2(sm64Next)( &sp );
    }
}


PRNG PF(InitXorshiro256StarStar)( XORSHIRO *state, u64 seed )
{
    PF2(Xorshiro256StarStarSeed)( state, seed );
    
    PRNG g;
    g.next      = PF2(Xorshiro256StarStarNext);
    g.jump      = PF2(Xorshiro256StarStarJump);
    g.seed      = PF2(Xorshiro256StarStarSeed);
    g.state     = state;
    return g;
}
    
    
#if TEST
void test_xorshiro256_star_star()
{
    XORSHIRO x;
    PF2(Xorshiro256StarStarSeed)( &x, 37473 );
    
    TEST_ASSERT( PF2(Xorshiro256StarStarNext)( &x ) > 0 );
    
    PF2(Xorshiro256StarStarJump)( &x);
    
    TEST_ASSERT( PF2(Xorshiro256StarStarNext)( &x ) > 0 );
}
#endif
    
#undef XORSHIRO


#if TEST
void test_generator()
{
#define STATE struct PRNG_Xorshiro256StarStar
#define RNG struct PRNG_Generator

    u64 seed = PRNG_SeedValue();
    
    STATE state;
    RNG rng = PRNG_InitXorshiro256StarStar( &state, seed );

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


#if TEST
void test_uniform()
{
#define STATE struct PRNG_Xorshiro256StarStar
#define RNG struct PRNG_Generator

    STATE state;
    RNG rng = PF(InitXorshiro256StarStar)( &state, 3747 );
    
    TEST_ASSERT( PRNG_Uniform( rng ) <= 1.0 );
    
#undef STATE
#undef RNG
}
#endif


// Standard Normal Random Number with Box-Muller Transformation.
// https://en.wikipedia.org/wiki/Box–Muller_transform .*/

f64 PF(BoxMuller)( PRNG g )
{
    f64 u = PF(Uniform)( g );
    f64 v = PF(Uniform)( g );
    
    f64 s = sqrt( -2.0 * log( u ) );
    f64 t = cos( 2.0 * M_PI * v );
    
    return s * t;
}


#if TEST
void test_box_muller()
{
    u32 N = 100;
    
    struct PF(Xorshift1024Star) state;
    PRNG rng = PF(InitXorshift1024Star)( &state, 3747 );
    
    f64 mean = 0;
    
    for ( u32 i=0; i<N; ++i ) {
        mean += PF(BoxMuller)( rng );
    }
    
    mean = mean / N;
    
    TEST_ASSERT( PF2(f64Equal)( mean, 0, 0.1 ) );
}
#endif


f64 PF(Exponential)( PRNG g, f64 lambda )
{
    ASSERT( lambda > 0 );
    
    f64 u = PF(Uniform)( g );
    
    return - log( u ) / lambda;
}


#if TEST
void test_exponential()
{
#define STATE struct PRNG_Xorshiro256StarStar
#define RNG struct PRNG_Generator

    u32 N = 1000;
    
    STATE state;
    RNG rng = PF(InitXorshiro256StarStar)( &state, 3547 );
    
    f64 lambda = 3.5;
    f64 mean = 0;
    
    for ( u32 i=0; i<N; ++i ) {
        mean += PF(Exponential)( rng, lambda );
    }
    
    mean = mean / N;

    TEST_ASSERT( PF2(f64Equal)( mean, 1.0/lambda, 0.02 ) );

#undef STATE
#undef RNG
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

