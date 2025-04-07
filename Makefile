#------------------------------------------------------------------------------
#
# Makefile, approach 3
# * implicit rule for first dependency not for all
# * dependency gather filtered
# * pattern substitution
#
#------------------------------------------------------------------------------
#
# This file is licensed after LGPL v3
# Look at: https://www.gnu.org/licenses/lgpl-3.0.en.html for details
#
#------------------------------------------------------------------------------

# make OUT_O_DIR=debug CC=clang CFLAGS="-g -O0" -f makefile.v3.mak
# make -f makefile.v3.mak
# touch include/cache.h
# make -f makefile.v3.mak
# make OUT_O_DIR=debug -f makefile.v3.mak
# make -f makefile.v3.mak testrun
# make -f makefile.v3.mak testrun -j4
# make OUT_O_DIR=debug -f makefile.v3.mak testrun -j4
# make OUT_O_DIR=debug -f makefile.v3.mak clean
# make -f makefile.v3.mak clean

ifeq ($(origin CC),default)
  CC = g++
endif

CFLAGS ?= # -D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -Werror=vla -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr
OUT_O_DIR ?= bin
TARGET_DIR ?= build
COMMONINC = -I./include
TESTS = ./Tests
SRC = ./src
ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
TARGET = processor

override CFLAGS += $(COMMONINC)

CSRC =  main.cpp 					 \
		src/assembler/assembler.cpp   \
		src/onegin/onegin.cpp          \
		src/processor/processor.cpp     \
		src/stack/stack.cpp 			 \
		src/lib/lib.cpp  			      \
		# src/console/consoleCmd.cpp          \

# reproducing source tree in object tree
COBJ := $(addprefix $(OUT_O_DIR)/,$(CSRC:.cpp=.o))
DEPS = $(COBJ:.o=.d)

.PHONY: all
all: $(TARGET_DIR)/$(TARGET)

$(TARGET_DIR)/$(TARGET): $(COBJ)
	@mkdir -p $(@D)
	$(CC) $^ -o $@ $(LDFLAGS)

# static pattern rule to not redefine generic one
$(COBJ) : $(OUT_O_DIR)/%.o : %.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(DEPS) : $(OUT_O_DIR)/%.d : %.cpp
	@mkdir -p $(@D)
	$(CC) -E $(CFLAGS) $< -MM -MT $(@:.d=.o) > $@

.PHONY: run
run:
	./$(TARGET_DIR)/$(TARGET)

TESTFILES=$(wildcard $(TESTS)/*.dat)

.PHONY: testrun
testrun: $(TESTFILES)

.PHONY: $(TESTFILES)
$(TESTFILES): $(OUT_O_DIR)/$(TARGET)
	@$(ROOT_DIR)/runtest.sh $@ $(OUT_O_DIR)/$(TARGET)

.PHONY: clean cleanDirs
clean:
	rm -rf $(COBJ) $(DEPS) $(OUT_O_DIR)/$(TARGET) $(OUT_O_DIR)/*.log

cleanDirs:
	rm -rf $(OUT_O_DIR) $(TARGET_DIR)

# targets which we have no need to recollect deps
NODEPS = clean

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
include $(DEPS)
endif