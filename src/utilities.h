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

#ifdef __cplusplus
}
#endif


