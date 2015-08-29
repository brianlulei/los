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
CFLAGS	+= $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

# Common linker flags
LDFLAGS	:= -m elf_i386

# Linker flags for user program
ULDFLAGS := -T user/user.ld

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
include lib/Makefile
include user/Makefile

ifndef QEMU
QEMU	:= $(shell if which qemu > /dev/null; \
				   then echo qemu; exit; \
						elif which qemu-system-i386 > /dev/null; \
						then echo qemu-system-i386; exit; \
				   fi;)
endif

CPUS ?= 1 

IMAGES	:= $(OBJDIR)/kernel/kernel.img
GDBPORT := $(shell expr `id -u` % 5000 + 25000)
QEMUOPTS = -hda $(OBJDIR)/kernel/kernel.img -serial mon:stdio -gdb tcp::$(GDBPORT)
QEMUOPTS += -smp $(CPUS)

pre-qemu: .gdbinit

.gdbinit: .gdbinit.templ
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > $@

qemu: $(IMAGES)
	$(QEMU) $(QEMUOPTS)

qemu-nox: $(IMAGES)
	$(QEMU) -nographic $(QEMUOPTS)

qemu-gdb: $(IMAGES) pre-qemu
	@echo "***"
	@echo "*** Use Ctrl-a x to exit qemu"
	@echo "***"
	$(QEMU) $(QEMUOPTS) -S

include $(OBJDIR)/.deps
$(OBJDIR)/.deps: $(foreach dir, $(OBJDIRS), $(wildcard $(OBJDIR)/$(dir)/*.d))
	@mkdir -p $(@D)

clean:
	rm -rf obj

.PHONY: all clean
