# Copyright 2021 Mitchell Levy

# This file is a part of super-glue

# super-glue is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# super-glue is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with super-glue.  If not, see <https://www.gnu.org/licenses/>.

# =============================================================================
# Set up

# -----------------------------------------------------------------------------
# Build settings

# To build a debug version of super-glue, run make as such:
# `make [target] BUILD=debug`
# This Makefile will build a release version of super-glue by default

OK_BUILDS ::= debug release

ifndef BUILD
  BUILD ::= release
else
  ifeq "$(filter $(BUILD),$(OK_BUILDS))" ""
    $(error Invalid BUILD "$(BUILD)". Valid options are: $(OK_BUILDS))
  endif
endif

# -----------------------------------------------------------------------------
# Common variables

# Set up all the necessary directory/file variables
BUILD_DIR ::= build
OBJ_DIR ::= $(BUILD_DIR)/obj/$(BUILD)
DEP_DIR ::= $(BUILD_DIR)/dep
SRC_DIR ::= src
LIB_DIR ::= lib
SRCS ::= $(wildcard $(SRC_DIR)/*.c) $(wildcard $(LIB_DIR)/*.c)
OBJS ::= $(foreach SRC,$(SRCS),$(OBJ_DIR)/$(notdir $(SRC:.c=.o)))
DEPS ::= $(foreach SRC,$(SRCS),$(DEP_DIR)/$(notdir $(SRC:.c=.d)))

# Keep track of executables for the `clean` target
EXES ::= super-glue

# Set which programs we're using for building
CC ::= gcc
LD ::= ld

# -----------------------------------------------------------------------------
# Flags

# Set flags for $(CC), based on the value of $(BUILD)
CFLAGS.base ::= -Wall -Wextra -pthread -std=c17 -I./$(SRC_DIR)/include -I./$(LIB_DIR)/include
CFLAGS.debug ::= -g
CFLAGS.release ::= -O3
CFLAGS ::= $(CFLAGS.$(BUILD)) $(CFLAGS.base)

# Set flags for generating dependencies, with the GCC options as default
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$*.d

# =============================================================================
# Recipies to build super-glue

# -----------------------------------------------------------------------------
# Phony targets
.PHONY: all clean
all: super-glue
clean: testclean
	rm -rf $(BUILD_DIR) $(EXES)

# -----------------------------------------------------------------------------
# Targets that build executables
super-glue: $(OBJS) 
	$(CC) $(CFLAGS) -o $@ $^

# -----------------------------------------------------------------------------
# Help `make` deal properly with generated dependencies
$(DEPS):
-include $(DEPS)

# -----------------------------------------------------------------------------
# Generate object files

# Delete the implicit .c -> .o rule
%.o: %.c
# Targets that build intermediates
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(DEP_DIR)/%.d $(OBJ_DIR) $(DEP_DIR)
	$(CC) $(DEPFLAGS) $(CFLAGS) -c -o $@ $<
$(OBJ_DIR)/%.o: $(LIB_DIR)/%.c | $(DEP_DIR)/%.d $(OBJ_DIR) $(DEP_DIR)
	$(CC) $(DEPFLAGS) $(CFLAGS) -c -o $@ $<
# =============================================================================
# Build test executables

# -----------------------------------------------------------------------------
# Set needed test-specific variables

# Base variables
TEST_ROOT_DIR ::= test
TEST_BUILD_DIR ::= $(BUILD_DIR)/test

# Code directories
TEST_SRC_DIR ::= $(TEST_ROOT_DIR)/tests
TEST_INCLUDE_DIR ::= $(TEST_SRC_DIR)/include

# We link all the super-glue objects except main.o into a shared lib, so they
# need to be compiled with -fpic. This should not be the default because -fpic
# prevents certain kinds of optimizations.
SUPER_GLUE_PIC_OBJ_DIR_BASE ::= $(TEST_BUILD_DIR)/super-glue-pic
SUPER_GLUE_PIC_OBJ_DIR ::= $(SUPER_GLUE_PIC_OBJ_DIR_BASE)/$(BUILD)
SUPER_GLUE_PIC_OBJS ::= $(foreach OBJ,$(filter-out main.o,$(notdir $(OBJS))),$(SUPER_GLUE_PIC_OBJ_DIR)/$(OBJ))

# Code that lives in ./test/tests should end up being built into files that
# live here.
TEST_OBJ_DIR ::= $(TEST_BUILD_DIR)/obj
TEST_DEP_DIR ::= $(TEST_BUILD_DIR)/dep

# Lists of the files we're operating on
TEST_DRIVER_SRC ::= $(TEST_ROOT_DIR)/test_driver.c
TEST_DRIVER_NAME ::= $(notdir $(basename $(TEST_DRIVER_SRC)))
TEST_SRCS ::= $(wildcard $(TEST_SRC_DIR)/*.c) $(TEST_DRIVER_SRC)
TEST_OBJS ::= $(foreach SRC,$(TEST_SRCS),$(TEST_OBJ_DIR)/$(notdir $(SRC:.c=.o)))
TEST_DEPS ::= $(foreach SRC,$(TEST_SRCS),$(TEST_DEP_DIR)/$(notdir $(SRC:.c=.d)))

TEST_EXE ::= $(TEST_BUILD_DIR)/test-$(BUILD)
TEST_SUPER_GLUE_SO ::= $(TEST_BUILD_DIR)/super-glue.$(BUILD).so
TEST_SUPER_GLUE_SO_LN ::= $(TEST_BUILD_DIR)/libsuper-glue.so

# Extra flags for building test targets
# Makes sure dependency information for files in $(TEST_SRC_DIR) goes where it
# should, not to where it goes for main sources.
DEPFLAGS.test = -MT $@ -MMD -MP -MF $(TEST_DEP_DIR)/$(basename $(notdir $@)).d
# Makes sure that the runtime path for the test executable contains the current
# directory so we don't have to copy the shared object file to /usr/lib or
# something like that.
RPATH_CURRENT_DIR ::= -Wl,-rpath,'$$ORIGIN' -Wl,-z,origin
# Add extra include directories for test code
CFLAGS.test ::= $(CFLAGS) -I./$(TEST_INCLUDE_DIR)
# Libraries needed to run build against libcheck
LDLIBS.test ::= -lcheck -lm -lrt -lsubunit

# -----------------------------------------------------------------------------
# Phony targets
.PHONY: test testclean test-mem
test: $(TEST_EXE)
	@./$(TEST_EXE)
test-mem: $(TEST_EXE)
	$(info Testing under valgrind...)
ifneq ($(BUILD),debug)
	$(warning WARNING: Without BUILD being set to debug, valgrind won\'t be able \
    to report line numbers inside the super-glue shared object!)
endif
	@CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all ./$(TEST_EXE)
testclean:
	rm -rf $(TEST_BUILD_DIR) $(TEST_EXE)

# -----------------------------------------------------------------------------
# Build intermediaries
$(TEST_OBJ_DIR)/$(TEST_DRIVER_NAME).o: $(TEST_DRIVER_SRC) | $(TEST_DEP_DIR)/$(TEST_DRIVER_NAME).d $(TEST_OBJ_DIR) $(TEST_DEP_DIR)
	$(CC) $(DEPFLAGS.test) $(CFLAGS.test) -c -o $@ $<

$(SUPER_GLUE_PIC_OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(DEP_DIR)/%.d $(SUPER_GLUE_PIC_OBJ_DIR) $(DEP_DIR)
	$(CC) $(DEPFLAGS) $(CFLAGS.test) -fpic -c -o $@ $<
$(SUPER_GLUE_PIC_OBJ_DIR)/%.o: $(LIB_DIR)/%.c | $(DEP_DIR)/%.d $(SUPER_GLUE_PIC_OBJ_DIR) $(DEP_DIR)
	$(CC) $(DEPFLAGS) $(CFLAGS.test) -fpic -c -o $@ $<
$(TEST_OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.c | $(TEST_DEP_DIR)/%.d $(TEST_OBJ_DIR) $(TEST_DEP_DIR)
	$(CC) $(DEPFLAGS.test) $(CFLAGS.test) -c -o $@ $<

# -----------------------------------------------------------------------------
# Build final object files
$(TEST_EXE): $(TEST_OBJS) $(TEST_SUPER_GLUE_SO) | $(TEST_BUILD_DIR)
	@# FIXME rebuilding with different BUILD back to back can result in the wrong
	@# shared object being linked to
	@ln -frs $(TEST_SUPER_GLUE_SO) $(TEST_SUPER_GLUE_SO_LN)
	$(CC) $(CFLAGS.test) $(RPATH_CURRENT_DIR) -L./$(TEST_BUILD_DIR) -o $@ $(filter-out $(TEST_SUPER_GLUE_SO),$^) $(LDLIBS.test) -lsuper-glue
$(TEST_SUPER_GLUE_SO): $(SUPER_GLUE_PIC_OBJS)
	$(CC) $(CFLAGS.test) -shared -o $@ $^ $(LDLIBS.test)

# -----------------------------------------------------------------------------
# Handle dependencies
$(TEST_DEPS):
-include $(TEST_DEPS)

# =============================================================================
# Order-only targets so we have the needed directory structure
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)
$(DEP_DIR):
	@mkdir -p $(DEP_DIR)
$(TEST_BUILD_DIR):
	@mkdir -p $(TEST_BUILD_DIR)
$(SUPER_GLUE_PIC_OBJ_DIR):
	@mkdir -p $(SUPER_GLUE_PIC_OBJ_DIR)
$(TEST_OBJ_DIR):
	@mkdir -p $(TEST_OBJ_DIR)
$(TEST_DEP_DIR):
	@mkdir -p $(TEST_DEP_DIR)
