ifeq ($(origin CC),default)
  CC = g++
endif


SFML_FLAGS = -lsfml-graphics -lsfml-window -lsfml-system

CFLAGS ?= 
LDFLAGS = $(SFML_FLAGS)

# BUILD_TYPE ?= debug
BUILD_TYPE ?= release


ifeq ($(BUILD_TYPE), release)
	CFLAGS += -DNDEBUG -Ofast -g0 -fvisibility=hidden -march=native -s
endif 

ifeq ($(BUILD_TYPE), debug)
	CFLAGS += -g -D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ 								         \
			  -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported 							      \
			  -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations 					           \
			  -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op 					            \
			  -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith 				         \
			  -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral 			              \
			  -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wstack-usage=8192                	   \
			  -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel              \
			  -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types             \
			  -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused           \
			  -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing          \
			  -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector         \
			  -pie -fPIE -Werror=vla 																		                  \
			  -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr \


	LDFLAGS += -fsanitize=address,undefined -lasan -lubsan
endif


OUT_O_DIR ?= bin
EXECUTABLE_DIR ?= build
COMMONINC = -I./include
SRC = ./src
ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
EXECUTABLE ?= processor
ASM_DIR  ?=
CODE_DIR ?=
ASM_FILE ?= programm.asm
BIN_FILE ?= code.bin
EXAMPLE_DIR = examples

-include local.mk

override CFLAGS += $(COMMONINC)

CSRC =  main.cpp 					\
		src/lib/lib.cpp  			 \
		src/stack/stack.cpp 		  \
		src/stack/hash.cpp   		   \
		src/fileread/fileread.cpp       \
		src/console/consoleCmd.cpp       \
		src/assembler/assembler.cpp       \
		src/processor/processor.cpp        \


ifeq ($(BUILD_TYPE), debug)
	CSRC += src/log/log.cpp
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

.PHONY: processor compile run rebuild makeCodeDir

ifeq ($(CODE_DIR), )
makeCodeDir:
else
makeCodeDir:
	@mkdir -p $(CODE_DIR)
endif

run:
	@make makeCodeDir
	@./$(EXECUTABLE_DIR)/$(EXECUTABLE) --assemble $(ASM_DIR)/$(ASM_FILE) $(CODE_DIR)/$(BIN_FILE) --execute $(CODE_DIR)/$(BIN_FILE)


assemble:
	@make makeCodeDir
	@./$(EXECUTABLE_DIR)/$(EXECUTABLE) --assemble $(ASM_DIR)/$(ASM_FILE) $(CODE_DIR)/$(BIN_FILE)

exe:
	@make makeCodeDir
	@./$(EXECUTABLE_DIR)/$(EXECUTABLE) --execute $(CODE_DIR)/$(BIN_FILE)

rebuild:
	make clean && make

#======= clean ========================================

.PHONY: clean cleanDirs cleanLog
clean:
	rm -rf $(COBJ) $(DEPS) $(EXECUTABLE_DIR)/$(EXECUTABLE) $(OUT_O_DIR)/$(SRC)

cleanDirs:
	rm -rf $(OUT_O_DIR) $(EXECUTABLE_DIR)

cleanLog:
	rm -rf Log/

#========== examples ===================================

EXAMPLE1_DIR  = example1
EXAMPLE1_FILE = HelloWorld

example1:
	@make									\
	ASM_DIR=$(EXAMPLE_DIR)/$(EXAMPLE1_DIR)   \
	CODE_DIR=$(EXAMPLE_DIR)/$(EXAMPLE1_DIR)   \
	ASM_FILE=$(EXAMPLE1_FILE).asm         	   \
	BIN_FILE=$(EXAMPLE1_FILE).bin               \
	run      									 \


EXAMPLE2_DIR  = example2
EXAMPLE2_FILE = Math

example2:
	@make                                  \
	ASM_DIR=$(EXAMPLE_DIR)/$(EXAMPLE2_DIR)  \
	CODE_DIR=$(EXAMPLE_DIR)/$(EXAMPLE2_DIR)  \
	ASM_FILE=$(EXAMPLE2_FILE).asm             \
	BIN_FILE=$(EXAMPLE2_FILE).bin              \
	run                                         \


EXAMPLE3_DIR  = example3
EXAMPLE3_FILE = Cycle

example3:
	@make                                  \
	ASM_DIR=$(EXAMPLE_DIR)/$(EXAMPLE3_DIR)  \
	CODE_DIR=$(EXAMPLE_DIR)/$(EXAMPLE3_DIR)  \
	ASM_FILE=$(EXAMPLE3_FILE).asm        	  \
	BIN_FILE=$(EXAMPLE3_FILE).bin              \
	run                                         \



EXAMPLE4_DIR  = example4
EXAMPLE4_FILE = Factorial

example4:
	@make                                  \
	ASM_DIR=$(EXAMPLE_DIR)/$(EXAMPLE4_DIR)  \
	CODE_DIR=$(EXAMPLE_DIR)/$(EXAMPLE4_DIR)  \
	ASM_FILE=$(EXAMPLE4_FILE).asm         	  \
	BIN_FILE=$(EXAMPLE4_FILE).bin              \
	run                                         \


EXAMPLE5_DIR  = example5
EXAMPLE5_FILE = circle

example5:
	@make                                  \
	ASM_DIR=$(EXAMPLE_DIR)/$(EXAMPLE5_DIR)  \
	CODE_DIR=$(EXAMPLE_DIR)/$(EXAMPLE5_DIR)  \
	ASM_FILE=$(EXAMPLE5_FILE).asm         	  \
	BIN_FILE=$(EXAMPLE5_FILE).bin              \
	run                                         \

#============ git =========================================

commit ?= "ZoV"

git:
	git add --all
	git commit -m $(commit)
	git push --all

git_clean:
	git push --force origin main

git_mega_clean: # to dangearous
	git reset --hard origin/main


#=======================================================

NODEPS = clean 

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
include $(DEPS)
endif
