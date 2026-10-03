CXX      := g++
CXXFLAGS := -std=c++20 -O3 -mtune=native -march=native \
            -fno-strict-aliasing -shared -fPIC -fopenmp

BUILD_DIR := agent
TEST_BIN  := $(BUILD_DIR)/test_engine
PERF_BIN  := $(BUILD_DIR)/perf_engine
DUMMY_BOT_BIN := $(BUILD_DIR)/dummy_bot

MODULE_NAME := engine

SIM_SRCS := \
	env/env.cc \
	env/src/engine.cc \
	env/src/java_math.cc \
	env/src/mapgen.cc \
	env/src/pod.cc \
	env/src/unit.cc \
	env/src/vector.cc

PYTHON_INC := -I/usr/include/python3.13
PYTHON_EXT := .cpython-313-x86_64-linux-gnu.so

NB_INCS    := -I./nanobind/include -I./nanobind/ext/robin_map/include
NB_SRC     := ./nanobind/src/nb_combined.cpp

INCLUDES   := $(PYTHON_INC) $(NB_INCS) -I./env
ALL_FLAGS  := $(CXXFLAGS) 

TARGET     := $(BUILD_DIR)/$(MODULE_NAME)$(PYTHON_EXT)
NB_OBJ     := $(BUILD_DIR)/nb_combined.o

$(NB_OBJ): $(NB_SRC) | $(BUILD_DIR)
	$(CXX) -O3 -std=c++17 -fPIC -fno-strict-aliasing $(PYTHON_INC) $(NB_INCS) -c $< -o $@

$(TARGET): $(SIM_SRCS) $(NB_OBJ) | $(BUILD_DIR)
	$(CXX) $(ALL_FLAGS) $(INCLUDES) $(SIM_SRCS) $(NB_OBJ) -o $(TARGET) -lgomp

all: $(TARGET)

TEST_SRCS := \
	env/test/test_engine.cc \
	$(SIM_SRCS)

DUMMY_BOT_SRCS := \
	env/test/dummy_bot.cc

PERF_SRCS := \
	env/test/perf_engine.cc \
	$(SIM_SRCS)

.PHONY: all dummy_bot validate clean

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TEST_BIN): $(TEST_SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(TEST_SRCS) -o $(TEST_BIN)

$(PERF_BIN): $(PERF_SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(PERF_SRCS) -o $(PERF_BIN)

$(DUMMY_BOT_BIN): $(DUMMY_BOT_SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DUMMY_BOT_SRCS) -o $(DUMMY_BOT_BIN)

dummy_bot: $(DUMMY_BOT_BIN)

perf: $(PERF_BIN)

$(ARENA_BIN): $(ARENA_SRCS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(ARENA_SRCS) -lz -o $(ARENA_BIN)

validate: $(TEST_BIN)
	python3 scripts/validate_replays.py --binary $(TEST_BIN) --replays scripts/replays

clean:
	rm $(TARGET) $(NB_OBJ)
