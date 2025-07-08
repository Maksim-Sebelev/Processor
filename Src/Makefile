# BUILD TYPES

# ASM_BUILD_TYPE  ?= release
ASM_BUILD_TYPE  ?= debug

# PROC_BUILD_TYPE ?= release
PROC_BUILD_TYPE ?= debug

# DASM_BUILD_TYPE ?= release
DASM_BUILD_TYPE ?= debug

# ==========================

ASM_MAKEFILE    := assembler/Makefile
PROC_MAKEFILE   := processor/Makefile
DASM_MAKEFILE   := disassembler/Makefile

BUILD_DIR       := ../build
OBJ_DIR         := ../bin

ASM_EXECUTABLE  := asm
PROC_EXECUTABLE := exe
DASM_EXECUTABLE := dasm

EXAMPLES_DIR = ../examples

#=======================

all:     asm         proc
run:     run_asm     run_proc
rebuild: rebuild_asm rebuild_proc
clean:   clean_asm   clean_proc

examples: all
	cp -r $(BUILD_DIR)/ $(EXAMPLES_DIR)/

# ===== asm ======

ASM_MAKE_PATTERN := $(MAKE)                                 \
							MAKEFILE_NAME=$(ASM_MAKEFILE)    \
					        EXECUTABLE_DIR=$(BUILD_DIR)       \
							EXECUTABLE=$(ASM_EXECUTABLE)       \
							OUT_O_DIR=$(OBJ_DIR)                \
							BUILD_TYPE=$(ASM_BUILD_TYPE)         \
					-f $(ASM_MAKEFILE)                            \

asm:
	$(ASM_MAKE_PATTERN)

run_asm:
	$(ASM_MAKE_PATTERN) run

rebuild_asm:
	$(ASM_MAKE_PATTERN) rebuild

clean_asm:
	$(ASM_MAKE_PATTERN) clean

# ===== proc =====

PROC_MAKE_PATTERN := $(MAKE)                                   \
							MAKEFILE_NAME=$(PROC_MAKEFILE)      \
							EXECUTABLE_DIR=$(BUILD_DIR)          \
						    EXECUTABLE=$(PROC_EXECUTABLE)         \
						    OUT_O_DIR=$(OBJ_DIR)                   \
							BUILD_TYPE=$(PROC_BUILD_TYPE)           \
					 -f $(PROC_MAKEFILE)                             \

proc:
	$(PROC_MAKE_PATTERN)

run_proc:
	$(PROC_MAKE_PATTERN) run

rebuild_proc:
	$(PROC_MAKE_PATTERN) rebuild

clean_proc:
	$(PROC_MAKE_PATTERN) clean


# ==== dasm =====

DASM_MAKE_PATTERN := $(MAKE)                                   \
							MAKEFILE_NAME=$(DASM_MAKEFILE)      \
							EXECUTABLE_DIR=$(BUILD_DIR)          \
						    EXECUTABLE=$(DASM_EXECUTABLE)         \
						    OUT_O_DIR=$(OBJ_DIR)                   \
							BUILD_TYPE=$(DASM_BUILD_TYPE)           \
					 -f $(DASM_MAKEFILE)                             \

dasm:
	$(DASM_MAKE_PATTERN)

run_dasm:
	$(DASM_MAKE_PATTERN) run

rebuild_dasm:
	$(DASM_MAKE_PATTERN) rebuild

clean_dasm:
	$(DASM_MAKE_PATTERN) clean

# ===================

.PHONY: all  run       rebuild      clean     \
        asm  run_asm   rebuild_asm  clean_asm  \
        proc run_proc  rebuild_proc clean_proc  \
		dasm run_dasm  rebuild_dasm clean_dasm   \
		examples                                  \
