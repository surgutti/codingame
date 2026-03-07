CXX := g++
CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -pedantic

BUILD_DIR := build
TEST_BIN := $(BUILD_DIR)/test_engine
DUMMY_BOT_BIN := $(BUILD_DIR)/dummy_bot

SIM_SRCS := \
	src/sim/engine.cc \
	src/sim/java_math.cc \
	src/sim/mapgen.cc \
	src/sim/pod.cc \
	src/sim/unit.cc \
	src/sim/vector.cc

TEST_SRCS := \
	src/test/test_engine.cc \
	$(SIM_SRCS)

DUMMY_BOT_SRCS := \
	src/test/dummy_bot.cpp

.PHONY: all dummy_bot validate clean

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TEST_BIN): $(TEST_SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(TEST_SRCS) -o $(TEST_BIN)

$(DUMMY_BOT_BIN): $(DUMMY_BOT_SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DUMMY_BOT_SRCS) -o $(DUMMY_BOT_BIN)

dummy_bot: $(DUMMY_BOT_BIN)

$(ARENA_BIN): $(ARENA_SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(ARENA_SRCS) -lz -o $(ARENA_BIN)

validate: $(TEST_BIN)
	python3 scripts/validate_replays.py --binary $(TEST_BIN) --replays scripts/replays

clean:
	rm -rf $(BUILD_DIR)
