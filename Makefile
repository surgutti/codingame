CXX ?= g++
CXXFLAGS ?= -std=c++20 -O2 -pipe
WARNFLAGS ?= -Wall -Wextra -Wpedantic

BUILD_DIR := build
TEST_BIN := $(BUILD_DIR)/core_tests
BENCH_BIN := $(BUILD_DIR)/perf_bench
BOT_BIN := $(BUILD_DIR)/brain
SUBMISSION := $(BUILD_DIR)/bot_submission.cpp
ARENA_CONFIG ?= configs/arena_variants.json

.PHONY: build-bot test bench check bundle refresh-bot arena-build arena-run arena auto-improve promote-best clean

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TEST_BIN): tests/core_tests.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(WARNFLAGS) -I. $< -o $@

$(BENCH_BIN): tests/perf_bench.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -DLOCAL -I. $< -o $@

$(BOT_BIN): src/bot/main.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -DLOCAL src/bot/main.cpp -o $@

build-bot: $(BOT_BIN)

test: $(TEST_BIN)
	./$(TEST_BIN)

bench: $(BENCH_BIN)
	./$(BENCH_BIN)

check:
	$(CXX) $(CXXFLAGS) -fsyntax-only src/bot/main.cpp
	$(CXX) $(CXXFLAGS) -fsyntax-only bot.cpp
	$(CXX) $(CXXFLAGS) -fsyntax-only brain.cpp
	$(CXX) $(CXXFLAGS) -fsyntax-only cg.cpp
	$(CXX) $(CXXFLAGS) -fsyntax-only test_fun.cpp

bundle: | $(BUILD_DIR)
	python3 tools/bundle_submission.py brain.cpp --output $(SUBMISSION)

refresh-bot:
	python3 tools/bundle_submission.py brain.cpp --output bot.cpp

arena-build:
	python3 tools/arena.py --config $(ARENA_CONFIG) --build-only

arena-run:
	python3 tools/arena.py --run-only --games 20 --workers 1

arena:
	python3 tools/arena.py --config $(ARENA_CONFIG) --games 20 --workers 1

auto-improve:
	python3 tools/auto_improve.py --generations 4 --population 8 --elite 3 --games 10 --workers 2

promote-best:
	python3 tools/promote_best.py

clean:
	rm -rf $(BUILD_DIR)
