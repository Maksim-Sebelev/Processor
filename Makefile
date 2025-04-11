ifeq ($(origin CC),default)
  CC = g++
endif


CFLAGS ?= 

BUILD_TYPE ?= debug

ifeq ($(BUILD_TYPE), release)
	CFLAGS += -O2 -D NDEBUG
else
	CFLAGS += -D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -Werror=vla
endif


OUT_O_DIR ?= bin
TARGET_DIR ?= build
COMMONINC = -I./include
SRC = ./src
ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
TARGET = processor
ASM_FILE ?= programm.asm
BIN_FILE ?= code.bin
CODE_DIR ?= code
ASM_DIR  ?= asm

override CFLAGS += $(COMMONINC)

CSRC =  main.cpp 					 \
		src/lib/lib.cpp  			  \
		src/stack/stack.cpp 		   \
		src/fileread/fileread.cpp       \
		src/console/consoleCmd.cpp       \
		src/assembler/assembler.cpp       \
		src/processor/processor.cpp        \


COBJ := $(addprefix $(OUT_O_DIR)/,$(CSRC:.cpp=.o))
DEPS = $(COBJ:.o=.d)

.PHONY: all
all: $(TARGET_DIR)/$(TARGET)

$(TARGET_DIR)/$(TARGET): $(COBJ)
	@mkdir -p $(@D)
	$(CC) $^ -o $@ $(LDFLAGS)

$(COBJ) : $(OUT_O_DIR)/%.o : %.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(DEPS) : $(OUT_O_DIR)/%.d : %.cpp
	@mkdir -p $(@D)
	@$(CC) -E $(CFLAGS) $< -MM -MT $(@:.d=.o) > $@


#======= run ==========================================

.PHONY: processor compile run
processor:
	@mkdir -p $(CODE_DIR)
	@./$(TARGET_DIR)/$(TARGET) -compile $(ASM_DIR)/$(ASM_FILE) $(CODE_DIR)/$(BIN_FILE) -run $(CODE_DIR)/$(BIN_FILE)

compile:
	@mkdir -p $(CODE_DIR)
	@./$(TARGET_DIR)/$(TARGET) -compile $(ASM_DIR)/$(ASM_FILE) $(CODE_DIR)/$(BIN_FILE)

run:
	@mkdir -p $(CODE_DIR)
	@./$(TARGET_DIR)/$(TARGET) -run $(CODE_DIR)/$(BIN_FILE)

#======= clean ========================================

.PHONY: clean cleanDirs
clean:
	rm -rf $(COBJ) $(DEPS) $(TARGET_DIR)/$(TARGET) $(OUT_O_DIR)/$(SRC)

cleanDirs:
	rm -rf $(OUT_O_DIR) $(TARGET_DIR)


#======================================================

# targets which we have no need to recollect deps
NODEPS = clean

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
include $(DEPS)
endif