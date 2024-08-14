ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/gba_rules

# Compiler and linker flags
# -------------------------

ASFLAGS		:= -x assembler-with-cpp
ARCHARM7	:= -mcpu=arm7tdmi

# Targets
# -------

OBJS		:= gba_xram_crt0.o

.PHONY: all clean

all: $(OBJS)

clean:
	@echo "  CLEAN"
	@rm -rf $(OBJS)

# Rules
# -----

%_crt0.o: %_crt0.s
	@echo "  Compile $@"
	$(CC) $(ASFLAGS) $(ARCHARM7) -c -o $@ $<
