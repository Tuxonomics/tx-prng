#include "../src/prng.h"


typedef prng::Generator PRNG;


int main( int argn, char *argv[] ) {
    
    u64 seed = prng::SeedValue();
    
    prng::Xorshift1024Star state;
    PRNG rng = prng::InitXorshift1024Star( &state, seed );
    
    printf("A random U[0, 1] value: %.6f\n", prng::NextFloat( rng ));
    
    prng::Jump( rng );
    
    printf("A random integer: %llu\n", prng::Next( rng ));
    
    return 0;
}

