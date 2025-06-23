MAKEFILE_NAME := make/make-asm.mk

ifeq ($(origin CC),default)
  CC = g++
endif


CFLAGS  ?= 
LDFLAGS ?=

BUILD_TYPE ?= debug
# BUILD_TYPE ?= release


ifeq ($(BUILD_TYPE), release)
	CFLAGS += -DNDEBUG -Ofast -g0 -fvisibility=hidden -march=native -s
endif 

ifeq ($(BUILD_TYPE), debug)
	CFLAGS += -g -D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++                                          \
			  -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported                                \
			  -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations                                \
			  -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op                                \
			  -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith                         \
			  -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral                         \
			  -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wstack-usage=8192                      \
			  -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel              \
			  -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types             \
			  -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused           \
			  -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing          \
			  -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector         \
			  -pie -fPIE -Werror=vla                                                                                         \
			  -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr \

endif


OUT_O_DIR      ?= bin
EXECUTABLE_DIR ?= build
COMMONINC       = -I./assembler/include -I./common/include
SRC             = ./src
EXECUTABLE     ?= asm
ASM_DIR        ?=
BIN_DIR        ?=
ASM_FILE       ?= programm.asm
BIN_FILE       ?= programm.bin

-include make/local.mk

override CFLAGS += $(COMMONINC)

CSRC =  assembler/main.cpp                          \
		assembler/src/flags/flags.cpp                \
		assembler/src/fileread/fileread.cpp           \
		assembler/src/assembler/assembler.cpp          \
		assembler/src/assembler/tokenizer/tokenizer.cpp \
		common/src/lib/lib.cpp                           \
		common/src/functions_for_files/files.cpp          \


ifeq ($(BUILD_TYPE), debug)
	CSRC += common/src/logger/log.cpp
endif

COBJ := $(addprefix $(OUT_O_DIR)/,$(CSRC:.cpp=.o))
DEPS = $(COBJ:.o=.d)

.PHONY: all
all: $(EXECUTABLE_DIR)/$(EXECUTABLE)

$(EXECUTABLE_DIR)/$(EXECUTABLE): $(COBJ)
	@mkdir -p $(@D)
	$(CC) $^ -o $@ $(LDFLAGS)

$(COBJ) : $(OUT_O_DIR)/%.o : %.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(DEPS) : $(OUT_O_DIR)/%.d : %.cpp
	@mkdir -p $(@D)
	@$(CC) -E $(CFLAGS) $< -MM -MT $(@:.d=.o) > $@


#======= run ==========================================

.PHONY: run rerun rebuild makeCodeDir

ifeq ($(BIN_DIR), )
makeCodeDir:
else
makeCodeDir:
	@mkdir -p $(BIN_DIR)
endif

run:
	@make -f $(MAKEFILE_NAME) makeCodeDir
	@./$(EXECUTABLE_DIR)/$(EXECUTABLE) --src $(ASM_DIR)/$(ASM_FILE) --bin $(BIN_DIR)/$(BIN_FILE)

rebuild:
	make -f $(MAKEFILE_NAME) clean
	make -f $(MAKEFILE_NAME)

rerun:
	make -f $(MAKEFILE_NAME)
	make -f $(MAKEFILE_NAME) run

#======= clean ========================================

.PHONY: clean
clean:
	rm -rf $(COBJ) $(DEPS) $(EXECUTABLE_DIR)/$(EXECUTABLE) $(OUT_O_DIR)/$(SRC)

#============ git =========================================

NODEPS = clean

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
include $(DEPS)
endif
