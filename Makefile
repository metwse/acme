CC=gcc
CFLAGS=-O2 -Wall -Wextra
RM=rm -rf

# test flags
TFLAGS=-O0 -g3 -Wall -Wextra -DB_ASSERT

BIN_DIR=bin

TARGET=bs

# no need to change anything below this line
OBJS=bio.o bmem.o

TESTS=bmem

ifdef DEBUG
CFLAGS=$(TFLAGS)
endif

ifdef ASSERTIONS
CFLAGS=-DB_ASSERT $(CFLAGS)
endif


$(BIN_DIR)/$(TARGET): $(OBJS) bs.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_DIR)/%.test: $(OBJS) tests/%.test.o | $(BIN_DIR)
	$(CC) $(TFLAGS) -o $@ $^


clean:
	$(RM) $(wildcard b*.o) \
		$(BIN_DIR)

.PHONY: clean

$(foreach target,$(target),$(shell $(CC) -MM b*.c))

$(foreach test_target,$(test_target),$(shell $(CC) -MM tests/b*.c))

$(BIN_DIR):
	mkdir $@
