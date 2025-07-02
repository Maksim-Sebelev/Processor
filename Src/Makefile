.PHONY: all  run      rerun      rebuild      clean     \
        asm  run_asm  rerun_asm  rebuild_asm  clean_asm  \
        proc run_proc rerun_proc rebuild_proc clean_proc  \


ASM_BUILD_TYPE  ?= release
# ASM_BUILD_TYPE  ?= debug

PROC_BUILD_TYPE ?= release
# PROC_BUILD_TYPE ?= debug

ASM_MAKEFILE       := assembler/Makefile
PROCESSOR_MAKEFILE := processor/Makefile

BUILD_DIR          := ../build
OBJ_DIR            := ../bin

ASM_EXECUTABLE     := asm
PROC_EXECUTABLE    := exe

EXAMPLES_DIR = ../examples

#=======================

all:     asm         proc
run:     run_asm     run_proc
rerun:   rerun_asm   rerun_proc
rebuild: rebuild_asm rebuild_proc
clean:   clean_asm   clean_proc

examples: all
	cp -r $(BUILD_DIR)/ $(EXAMPLES_DIR)/

# ===== asm ======

ASM_MAKE_PATTERN := $(MAKE)                             \
					        EXECUTABLE_DIR=$(BUILD_DIR)  \
							EXECUTABLE=$(ASM_EXECUTABLE)  \
							OUT_O_DIR=$(OBJ_DIR)           \
							BUILD_TYPE=$(ASM_BUILD_TYPE)    \
					-f $(ASM_MAKEFILE)                       \

asm:
	$(ASM_MAKE_PATTERN)

run_asm:
	$(ASM_MAKE_PATTERN) run

rerun_asm:
	$(ASM_MAKE_PATTERN) rerun

rebuild_asm:
	$(ASM_MAKE_PATTERN) rebuild

clean_asm:
	$(ASM_MAKE_PATTERN) clean

# ===== proc =====

PROC_MAKE_PATTERN := $(MAKE)                              \
							EXECUTABLE_DIR=$(BUILD_DIR)    \
						    EXECUTABLE=$(PROC_EXECUTABLE)   \
						    OUT_O_DIR=$(OBJ_DIR)             \
							BUILD_TYPE=$(PROC_BUILD_TYPE)     \
					 -f $(PROCESSOR_MAKEFILE)                  \

proc:
	$(PROC_MAKE_PATTERN)

run_proc:
	$(PROC_MAKE_PATTERN) run

rerun_proc:
	$(PROC_MAKE_PATTERN) rerun

rebuild_proc:
	$(PROC_MAKE_PATTERN) rebuild

clean_proc:
	$(PROC_MAKE_PATTERN) clean


