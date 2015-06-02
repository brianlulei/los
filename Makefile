OBJDIR := obj
OBJDUMP := objdump
OBJCOPY := objcopy

all: $(OBJDIR)/boot/boot $(OBJDIR)/kernel/entry.o $(OBJDIR)/kernel/entrypgdir.o $(OBJDIR)/kernel/console.o $(OBJDIR)/kernel/init.o $(OBJDIR)/kernel/console.o $(OBJDIR)/kernel/printf.o $(OBJDIR)/kernel/printfmt.o $(OBJDIR)/kernel/string.o

include boot/Makefile
include kernel/Makefile

clean:
	rm -rf obj
