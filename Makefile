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
all: 

# Eliminate default suffix rules
.SUFFIXES:

.DELETE_ON_ERROR:

# make it so that no intermediate .o files are ever deleted
.PRECIOUS: $(OBJDIR)/kernel/%.o

KERN_CFLAGS	:= $(CFLAGS) -DLOS_KERNEL -gstabs
USER_CFLAGS	:= $(CFLAGS) -DLOS_USER -gstabs

include boot/Makefile
include kernel/Makefile

clean:
	rm -rf obj


.PHONY: all clean
