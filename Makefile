BIN_DIR := bin

TARGET_EXEC_TRACER := tracer
TARGET_EXEC_MONITOR := monitor

DEBUG_EXEC_TRACER := tracer-debug
DEBUG_EXEC_MONITOR := monitor-debug

BASE_BUILD_DIR := build
BUILD_DIR_TRACER := $(BASE_BUILD_DIR)/obj/tracer
BUILD_DIR_MONITOR := $(BASE_BUILD_DIR)/obj/monitor

DEBUG_BUILD_DIR_TRACER := $(BASE_BUILD_DIR)/obj_debug/tracer
DEBUG_BUILD_DIR_MONITOR := $(BASE_BUILD_DIR)/obj_debug/monitor

SRC_DIR_TRACER := src/tracer
SRC_DIR_MONITOR := src/monitor

INC_DIR_TRACER := include/tracer
INC_DIR_MONITOR := include/monitor
INC_DIR_COMMON := include/common

CC := gcc

GLIBFLAGS := $(shell pkg-config --cflags --libs gobject-2.0)
STD_FLAGS := -O2 $(GLIBFLAGS) -Wall -Wextra -pedantic -Wno-unused-parameter $(PROFILING_OPTS) # -lm -pthread -Wconversion
STD_DEBUG_FLAGS := -O0 -g3 $(GLIBFLAGS) -Wall -Wextra -pedantic -Wno-unused-parameter $(PROFILING_OPTS)
TRACER_FLAGS := -I$(INC_DIR_TRACER) -I$(INC_DIR_COMMON) $(STD_FLAGS)
MONITOR_FLAGS := -I$(INC_DIR_MONITOR) -I$(INC_DIR_COMMON) $(STD_FLAGS)

DEBUG_FLAGS_TRACER := -I$(INC_DIR_TRACER) -I$(INC_DIR_COMMON) $(STD_DEBUG_FLAGS)
DEBUG_FLAGS_MONITOR := -I$(INC_DIR_MONITOR) -I$(INC_DIR_COMMON) $(STD_DEBUG_FLAGS)

LDFLAGS := $(PROFILING_OPTS)

# is there a better way to do this?????????????
SRC_TRACER := $(shell ls $(SRC_DIR_TRACER) | grep '.c')
OBJ_TRACER := $(subst .c,.o,$(SRC_TRACER))
OBJ_DEBUG_TRACER := $(OBJ_TRACER:%=$(DEBUG_BUILD_DIR_TRACER)/%)
OBJ_TRACER := $(OBJ_TRACER:%=$(BUILD_DIR_TRACER)/%)

SRC_MONITOR := $(shell ls $(SRC_DIR_MONITOR) | grep '.c')
OBJ_MONITOR := $(subst .c,.o,$(SRC_MONITOR))
# isto é sequer preciso? não dá para fazer depois???
OBJ_DEBUG_MONITOR := $(OBJ_MONITOR:%=$(DEBUG_BUILD_DIR_MONITOR)/%)
OBJ_MONITOR := $(OBJ_MONITOR:%=$(BUILD_DIR_MONITOR)/%)

# make .d
DEPS := $(OBJ_TRACER:.o=.d) $(OBJ_MONITOR:.o=.d) $(OBJ_DEBUG_TRACER:.o=.d) $(OBJ_DEBUG_MONITOR:.o=.d)

CPPFLAGS := -MMD -MP

# .PHONY: folders
# folders:
# @mkdir

.PHONY: all
all: client server

.PHONY: client
client: $(TARGET_EXEC_TRACER)

$(TARGET_EXEC_TRACER): $(OBJ_TRACER)
	mkdir -p $(dir $@)
	$(CXX) $(OBJ_TRACER) $(GLIBFLAGS) -o $@ $(LDFLAGS)

$(BUILD_DIR_TRACER)/%.o: $(SRC_DIR_TRACER)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(TRACER_FLAGS) -c $< -o $@

.PHONY: server
server: $(TARGET_EXEC_MONITOR)

$(TARGET_EXEC_MONITOR): $(OBJ_MONITOR)
	mkdir -p $(dir $@)
	$(CXX) $(OBJ_MONITOR) $(GLIBFLAGS) -o $@ $(LDFLAGS)

$(BUILD_DIR_MONITOR)/%.o: $(SRC_DIR_MONITOR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(MONITOR_FLAGS) -c $< -o $@

.PHONY: debug
debug: debug-client debug-server

.PHONY: debug-client
debug-client: $(DEBUG_EXEC_TRACER)

$(DEBUG_EXEC_TRACER): $(OBJ_DEBUG_TRACER)
	mkdir -p $(dir $@)
	$(CXX) $(OBJ_DEBUG_TRACER) $(GLIBFLAGS) -o $@ $(LDFLAGS)

$(DEBUG_BUILD_DIR_TRACER)/%.o: $(SRC_DIR_TRACER)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(DEBUG_FLAGS_TRACER) -c $< -o $@

.PHONY: debug-server
debug-server: $(DEBUG_EXEC_MONITOR)

$(DEBUG_EXEC_MONITOR): $(OBJ_DEBUG_MONITOR)
	mkdir -p $(dir $@)
	$(CXX) $(OBJ_DEBUG_MONITOR) $(GLIBFLAGS) -o $@ $(LDFLAGS)

$(DEBUG_BUILD_DIR_MONITOR)/%.o: $(SRC_DIR_MONITOR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(DEBUG_FLAGS_MONITOR) -c $< -o $@

.PHONY: clean
RM_DIRS := $(BASE_BUILD_DIR) $(BIN_DIR) $(TARGET_EXEC_TRACER) $(TARGET_EXEC_MONITOR) pipes/*
RED := \033[0;31m
NC := \033[0m
clean:
	@echo -ne '$(RED)Removing:\n$(NC) $(RM_DIRS:%=%\n)'
	-@rm -r $(RM_DIRS) 2>/dev/null || true

-include $(DEPS)
