NAME = acme

CC = gcc
RM = rm -rf

CFLAGS_COMMON = -std=gnu17 -Wall -Wextra
LIBS = -lX11

CFLAGS = $(CFLAGS_COMMON) -O2
TFLAGS = $(CFLAGS_COMMON) -O0 -g3 --coverage

SRC_DIR = src
TEST_DIR = tests
DIST_DIR = target

# no need to change below this line
SRCS = $(wildcard $(SRC_DIR)/*.c)
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)

ifndef DEBUG
MODE = release
else
MODE = debug
CFLAGS = $(TFLAGS)
endif

OBJ_DIR = $(DIST_DIR)/$(MODE)/obj
TEST_OBJ_DIR = $(DIST_DIR)/$(MODE)/obj/test

OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c,$(TEST_OBJ_DIR)/%.o,$(TEST_SRCS))

LIB_OBJS = $(filter-out $(DIST_DIR)/$(MODE)/obj/main.o,$(OBJS))

TEST_TARGETS = $(patsubst $(TEST_DIR)/%.c,$(DIST_DIR)/%.test.$(MODE),$(TEST_SRCS))

default: $(DIST_DIR)/$(NAME).$(MODE)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MMD

$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.c | $(TEST_OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MMD

$(DIST_DIR)/$(NAME).$(MODE): $(OBJS) | $(DIST_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(DIST_DIR)/%.test.$(MODE): $(TEST_OBJ_DIR)/%.o $(LIB_OBJS) | $(DIST_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(DIST_DIR) $(TEST_OBJ_DIR) $(OBJ_DIR):
	mkdir -p $@

tests: $(TEST_TARGETS)

all: default tests

docs:
	doxygen

clean:
	$(RM) $(DIST_DIR) docs

.SECONDARY: $(OBJS) $(TEST_OBJS)
-include $(OBJS:.o=.d)
-include $(TEST_OBJS:.o=.d)

.PHONY: default tests all clean
