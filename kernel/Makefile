OBJDIRS	+= kernel

KERN_LDFLAGS	:= $(LDFLAGS) -nostdlib -T kernel/kernel.ld

KERN_SRCFILES := kernel/entry.S \
				 kernel/trapentry.S\
				 kernel/entrypgdir.c \
				 kernel/init.c \
				 kernel/console.c \
				 kernel/printf.c \
				 kernel/monitor.c \
				 kernel/kdebug.c \
				 kernel/pmap.c \
				 kernel/kclock.c \
				 kernel/env.c \
				 kernel/trap.c\
				 lib/printfmt.c \
				 lib/string.c \
				 lib/assert.c \
				 lib/readline.c

# Only build files if they exist.
# KERN_SRCFILES	:= $(wildcard $(KERN_SRCFILES))

KERN_BINFILES := user/hello

KERN_OBJFILES	:= $(patsubst %.c, $(OBJDIR)/%.o, $(KERN_SRCFILES))
KERN_OBJFILES	:= $(patsubst %.S, $(OBJDIR)/%.o, $(KERN_OBJFILES))
KERN_OBJFILES	:= $(patsubst $(OBJDIR)/lib/%, $(OBJDIR)/kernel/%, $(KERN_OBJFILES))

KERN_BINFILES := $(patsubst %, $(OBJDIR)/%, $(KERN_BINFILES))

$(OBJDIR)/kernel/%.o: kernel/%.S
	@echo + cc $<
	@mkdir -p $(@D)
	$(CC) -nostdinc $(KERN_CFLAGS) -c -o $@ $<

$(OBJDIR)/kernel/%.o: kernel/%.c
	@echo + cc $<
	@mkdir -p $(@D)
	$(CC) -nostdinc $(KERN_CFLAGS) -c -o $@ $<

$(OBJDIR)/kernel/%.o: lib/%.c
	@echo + cc $<
	@mkdir -p $(@D)
	$(CC) -nostdinc $(KERN_CFLAGS) -c -o $@ $<

# How to build the kernel itself
$(OBJDIR)/kernel/kernel: $(KERN_OBJFILES) $(KERN_BINFILES) 
	@echo + ld $@ $<
	$(LD) -o $@ $(KERN_LDFLAGS) $(KERN_OBJFILES) $(GCC_LIB) -b binary $(KERN_BINFILES)
	$(OBJDUMP) -S $@ > $@.asm
	$(NM) -n $@ > $@.sym

# How to build the kenel disk image
$(OBJDIR)/kernel/kernel.img: $(OBJDIR)/kernel/kernel $(OBJDIR)/boot/boot
	@echo + make $@
	dd if=/dev/zero of=$(OBJDIR)/kernel/kernel.img~ count=10000 2>/dev/null
	dd if=$(OBJDIR)/boot/boot of=$(OBJDIR)/kernel/kernel.img~ conv=notrunc 2>/dev/null
	dd if=$(OBJDIR)/kernel/kernel of=$(OBJDIR)/kernel/kernel.img~ seek=1 conv=notrunc 2>/dev/null
	mv $(OBJDIR)/kernel/kernel.img~ $(OBJDIR)/kernel/kernel.img

all: $(OBJDIR)/kernel/kernel.img
