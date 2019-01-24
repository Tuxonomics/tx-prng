#include "prng.h"

#define STATE struct PRNG_Xorshift1024Star
#define PRNG struct PRNG_Generator

#ifndef TEST
int main( int argn, const char *argv[] ) {

    u64 seed = PRNG_SeedValue();

    STATE state;
    PRNG rng = PRNG_InitXorshift1024Star( &state, seed );

    printf("A random U[0, 1] value: %.6f\n", PRNG_Uniform( rng ));

    PRNG_Jump( rng );

    printf("A random integer: %llu\n", PRNG_Next( rng ));
    
    return 0;
}
#endif
