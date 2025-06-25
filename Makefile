.PHONY: all  run      rerun      rebuild      clean     \
        asm  run_asm  rerun_asm  rebuild_asm  clean_asm  \
        proc run_proc rerun_proc rebuild_proc clean_proc  \


# ASM_BUILD_TYPE  ?= release
ASM_BUILD_TYPE  ?= debug
PROC_BUILD_TYPE ?= release
# PROC_BUILD_TYPE ?= debug


all: asm proc

run: run_asm run_proc

rerun: rerun_asm rerun_proc

rebuild: rebuild_asm rebuild_proc

clean: clean_asm clean_proc


# ===== asm ======

asm:
	$(MAKE) BUILD_TYPE=$(ASM_BUILD_TYPE) -f make/make-asm.mk

run_asm:
	$(MAKE) BUILD_TYPE=$(ASM_BUILD_TYPE) -f make/make-asm.mk run

rerun_asm:
	$(MAKE) BUILD_TYPE=$(ASM_BUILD_TYPE) -f make/make-asm.mk rerun

rebuild_asm:
	$(MAKE) BUILD_TYPE=$(ASM_BUILD_TYPE) -f make/make-asm.mk rebuild

clean_asm:
	$(MAKE) BUILD_TYPE=$(ASM_BUILD_TYPE) -f make/make-asm.mk clean


# ===== proc =====

proc:
	$(MAKE) BUILD_TYPE=$(PROC_BUILD_TYPE) -f make/make-proc.mk

run_proc:
	$(MAKE) BUILD_TYPE=$(PROC_BUILD_TYPE) -f make/make-proc.mk run

rerun_proc:
	$(MAKE) BUILD_TYPE=$(PROC_BUILD_TYPE) -f make/make-proc.mk rerun

rebuild_proc:
	$(MAKE) BUILD_TYPE=$(PROC_BUILD_TYPE) -f make/make-proc.mk rebuild

clean_proc:
	$(MAKE) BUILD_TYPE=$(PROC_BUILD_TYPE) -f make/make-proc.mk clean



# ===== examples =====

EXAMPLE_DIR = examples

# ==================

EXAMPLE1_DIR  = example1
EXAMPLE1_FILE = HelloWorld

example1: asm proc
	@make                                   \
	ASM_DIR=$(EXAMPLE_DIR)/$(EXAMPLE1_DIR)   \
	BIN_DIR=$(EXAMPLE_DIR)/$(EXAMPLE1_DIR)    \
	ASM_FILE=$(EXAMPLE1_FILE).asm              \
	BIN_FILE=$(EXAMPLE1_FILE).bin               \
	-f make/make-asm.mk run                      \

	@make                                          \
	BIN_DIR=$(EXAMPLE_DIR)/$(EXAMPLE1_DIR)          \
	BIN_FILE=$(EXAMPLE1_FILE).bin                    \
	-f make/make-proc.mk run                          \

# ==================

EXAMPLE2_DIR  = example2
EXAMPLE2_FILE = Math

example2: asm proc
	@make                                   \
	ASM_DIR=$(EXAMPLE_DIR)/$(EXAMPLE2_DIR)   \
	BIN_DIR=$(EXAMPLE_DIR)/$(EXAMPLE2_DIR)    \
	ASM_FILE=$(EXAMPLE2_FILE).asm              \
	BIN_FILE=$(EXAMPLE2_FILE).bin               \
	-f make/make-asm.mk run                      \

	@make                                          \
	BIN_DIR=$(EXAMPLE_DIR)/$(EXAMPLE2_DIR)          \
	BIN_FILE=$(EXAMPLE2_FILE).bin                    \
	-f make/make-proc.mk run                          \

# ==================

EXAMPLE3_DIR  = example3
EXAMPLE3_FILE = Cycle

example3: asm proc
	@make                                   \
	ASM_DIR=$(EXAMPLE_DIR)/$(EXAMPLE3_DIR)   \
	BIN_DIR=$(EXAMPLE_DIR)/$(EXAMPLE3_DIR)    \
	ASM_FILE=$(EXAMPLE3_FILE).asm              \
	BIN_FILE=$(EXAMPLE3_FILE).bin               \
	-f make/make-asm.mk run                      \

	@make                                          \
	BIN_DIR=$(EXAMPLE_DIR)/$(EXAMPLE3_DIR)          \
	BIN_FILE=$(EXAMPLE3_FILE).bin                    \
	-f make/make-proc.mk run                          \

# ==================

EXAMPLE4_DIR  = example4
EXAMPLE4_FILE = Factorial

example4: asm proc
	@make                                   \
	ASM_DIR=$(EXAMPLE_DIR)/$(EXAMPLE4_DIR)   \
	BIN_DIR=$(EXAMPLE_DIR)/$(EXAMPLE4_DIR)    \
	ASM_FILE=$(EXAMPLE4_FILE).asm              \
	BIN_FILE=$(EXAMPLE4_FILE).bin               \
	-f make/make-asm.mk run                      \

	@make                                          \
	BIN_DIR=$(EXAMPLE_DIR)/$(EXAMPLE4_DIR)          \
	BIN_FILE=$(EXAMPLE4_FILE).bin                    \
	-f make/make-proc.mk run                          \

# ==================

EXAMPLE5_DIR  = example5
EXAMPLE5_FILE = circle

example5: asm proc
	@make                                   \
	ASM_DIR=$(EXAMPLE_DIR)/$(EXAMPLE5_DIR)   \
	BIN_DIR=$(EXAMPLE_DIR)/$(EXAMPLE5_DIR)    \
	ASM_FILE=$(EXAMPLE5_FILE).asm              \
	BIN_FILE=$(EXAMPLE5_FILE).bin               \
	-f make/make-asm.mk run                      \

	@make                                          \
	BIN_DIR=$(EXAMPLE_DIR)/$(EXAMPLE5_DIR)          \
	BIN_FILE=$(EXAMPLE5_FILE).bin                    \
	-f make/make-proc.mk run                          \

# ==================
