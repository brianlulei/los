OBJDIR := obj
OBJDUMP := objdump
OBJCOPY := objcopy

include boot/Makefile

all: $(OBJDIR)/boot/boot
