CC = clang

tests:   CFLAGS = -O0
debug:   CFLAGS = -g -O0 -DDEBUG -std=c89
release: CFLAGS = -O3 -march=native -std=c89

LFLAGS = -lm 

TARGET = main
TEST_TARGET = $(TARGET)_tests
TEST_MAIN = $(TARGET)_tests.c
TEST_LOG = $(TARGET)_tests.log

all: debug

debug:   clean $(TARGET)
release: clean $(TARGET)

$(TARGET):
	$(CC) src/main.c -o $(TARGET) $(CFLAGS) $(LFLAGS)

tests:
	@rm -f $(TEST_TARGET) $(TEST_LOG) $(TEST_MAIN)
	@./scripts/gen_test_main.sh > $(TEST_MAIN)
	@echo "$(CC) $(TEST_MAIN) -o $(TEST_TARGET) $(CFLAGS) -DTEST $(LFLAGS)"
	@$(CC) $(TEST_MAIN) -o $(TEST_TARGET) $(CFLAGS) -DTEST $(LFLAGS)
	@./$(TEST_TARGET) 2> $(TEST_LOG)
	@rm -f $(TEST_TARGET) $(TEST_MAIN)

clean:
	rm -f $(TARGET)

.PHONY: all clean debug release tests


