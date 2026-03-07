CXX := g++
CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -pedantic

BUILD_DIR := build
TEST_BIN := $(BUILD_DIR)/test_engine

SIM_SRCS := \
	src/sim/engine.cc \
	src/sim/java_math.cc \
	src/sim/pod.cc \
	src/sim/unit.cc \
	src/sim/vector.cc

TEST_SRCS := \
	src/test/test_engine.cc \
	$(SIM_SRCS)

.PHONY: all test validate clean

all: test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TEST_BIN): $(TEST_SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(TEST_SRCS) -o $(TEST_BIN)

test: $(TEST_BIN)

validate: $(TEST_BIN)
	python3 scripts/validate_replays.py --binary $(TEST_BIN) --replays scripts/replays

clean:
	rm -rf $(BUILD_DIR)
