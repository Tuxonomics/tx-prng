CC  ?= clang
CPP ?= clang++

debug: CFLAGS = -g -O0 -DDEBUG -std=c89

LFLAGS = -lm

all: debug

debug: clean

c_examples:
	$(CC) example_1.c -o example_1_c $(CFLAGS) $(LFLAGS)

cpp_examples:
	$(CPP) example_1.cpp -o example_1_cpp $(CFLAGS)

clean:
	rm -f example_1_c example_1_cpp

.PHONY: all clean debug release tests
