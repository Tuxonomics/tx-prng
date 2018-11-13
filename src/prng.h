#include "utilities.h"

#ifdef __cplusplus
namespace prng {
extern "C" {
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


#define PRNG_NEXT_FUNC(name) Inline u64 name(void *state)
typedef u64 PRNG_nextFunc(void *state);

#define PRNG_NEXT_FLOAT_FUNC(name) Inline f64 name(void *state)
typedef f64 PRNG_nextFloatFunc(void *state);

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
    PRNG_nextFloatFunc *nextFloat;
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
f64 PF(NextFloat)( PRNG g )
{
    return g.nextFloat( g.state );
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


PRNG_NEXT_FUNC(prngXorshift1024StarNext)
{
    XORSHIFT *x = (XORSHIFT *) state;
    
    const u64 s0 = x->s[x->p];
    u64       s1 = x->s[ x->p = (x->p + 1) & 15 ];
    
    s1 ^= s1 << 31;
    
    x->s[x->p] = s1 ^ s0 ^ (s1 >> 11) ^ (s0 >> 30);
    
    return x->s[x->p] * UINT64_C(1181783497276652981);
}


PRNG_NEXT_FLOAT_FUNC(prngXorshift1024StarNextFloat)
{
    u64 x = prngXorshift1024StarNext( state );
    
    return PF2(toFloat)( x );
}


// NOTE(jonas): the jump is equivalent to 2^512 calls to next
PRNG_JUMP_FUNC(prngXorshift1024StarJump)
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
            prngXorshift1024StarNext( state );
        }
    }
    
    for ( i32 j = 0; j < 16; j++ ) {
        x->s[(j + x->p) & 15] = t[j];
    }
}


PRNG_SEED_FUNC(prngXorshift1024StarSeed)
{
    XORSHIFT *x = (XORSHIFT *) state;
    
    SMIX sp = (SMIX) { .s = seed };
    
    for ( u32 i=0; i<16; ++i ) {
        x->s[i] = PF2(sm64Next)( &sp );
    }
    
    x->p = 0;
}

    
PRNG PF(InitXorshift1024Star)( struct PF(Xorshift1024Star) *state, u64 seed )
{
    prngXorshift1024StarSeed( state, seed );
    
    PRNG g;
    g.next      = prngXorshift1024StarNext;
    g.nextFloat = prngXorshift1024StarNextFloat;
    g.jump      = prngXorshift1024StarJump;
    g.seed      = prngXorshift1024StarSeed;
    g.state     = state;
    return g;
}


#define PRNG_XORSHIFT_INIT PF(Xorshift1024StarInit)( PF(SeedValue)() )


#if TEST
void test_xorshift1024()
{
    XORSHIFT x;
    prngXorshift1024StarSeed( &x, 37473 );
    
    TEST_ASSERT( prngXorshift1024StarNext( &x ) > 0 );
    TEST_ASSERT( prngXorshift1024StarNextFloat( &x ) < 1.0 );
    
    prngXorshift1024StarJump( &x);
    
    TEST_ASSERT( prngXorshift1024StarNext( &x ) > 0 );
    TEST_ASSERT( prngXorshift1024StarNextFloat( &x ) < 1.0 );
}
#endif

#undef XORSHIFT

    
// TODO(jonas): http://xoshiro.di.unimi.it/xoshiro256starstar.c




// Standard Normal Random Number with Box-Muller Transformation.
// https://en.wikipedia.org/wiki/Box–Muller_transform .*/


f64 PF(BoxMuller)( PRNG g )
{
    f64 u = PF(NextFloat)( g );
    f64 v = PF(NextFloat)( g );
    
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
    
//    TEST_ASSERT( f64Equal( mean, 0, 0.1 ) );
}
#endif

    
#if TEST
void test_generator()
{
    
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

