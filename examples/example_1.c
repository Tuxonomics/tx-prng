#include "../src/prng.h"


#define PRNG struct PRNG_Generator


int main( int argn, char *argv[] ) {
    
    u64 seed = PRNG_SeedValue();
    
    struct PRNG_Xorshift1024Star state;
    PRNG rng = PRNG_InitXorshift1024Star( &state, seed );
    
    printf("A random U[0, 1] value: %.6f\n", PRNG_NextFloat( rng ));
    
    PRNG_Jump( rng );
    
    printf("A random integer: %llu\n", PRNG_Next( rng ));
    
    return 0;
}

