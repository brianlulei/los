OBJDIR	:= obj
TOP		:= .

CC	:= gcc -pipe
AS	:= as
AR	:= ar
LD	:= ld
OBJDUMP := objdump
OBJCOPY := objcopy
NM	:= nm

# Compiler flags
# -fno-builtin is required to avoid refs to undefined functions in the kernel.
# Only optimize to -o1

CFLAGS	:= $(CFLAGS) -O1 -fno-builtin -I$(TOP) -MD
CFLAGS	+= -fno-omit-frame-pointer
CFLAGS	+= -Wall -Wno-format -Wno-unused -Werror -gstabs -m32
CFLAGS	+= -fno-tree-ch

LDFLAGS	:= -m elf_i386

GCC_LIB	:= $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

# List that the */Makefile fragments will add to 
OBJDIRS	:=

# Make sure that all is the first target
KERN_CFLAGS	:= $(CFLAGS) -DLOS_KERNEL -gstabs
USER_CFLAGS	:= $(CFLAGS) -DLOS_USER -gstabs

all: $(OBJDIR)/boot/boot $(OBJDIR)/kernel/entry.o $(OBJDIR)/kernel/entrypgdir.o $(OBJDIR)/kernel/console.o $(OBJDIR)/kernel/init.o $(OBJDIR)/kernel/console.o $(OBJDIR)/kernel/printf.o $(OBJDIR)/kernel/printfmt.o $(OBJDIR)/kernel/string.o

include boot/Makefile
include kernel/Makefile

clean:
	rm -rf obj
