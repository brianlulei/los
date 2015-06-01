OBJDIR := obj
OBJDUMP := objdump
OBJCOPY := objcopy

all: $(OBJDIR)/boot/boot $(OBJDIR)/kernel/entry.o $(OBJDIR)/kernel/entrypgdir.o $(OBJDIR)/kernel/console.o $(OBJDIR)/kernel/init.o

include boot/Makefile
include kernel/Makefile

clean:
	rm -rf obj
