OBJDIRS += kern

KERN_SRCFILES := kernel/entry.S

KERN_SRCFILES := $(wildcard $(KERN_SRCFILES))

$(OBJDIR)/kernel/%.o: kernel/%.S
	@echo + cc $<
	@mkdir -p $(@D)
	#$(CC) -I. -c -o $@ $<
	$(CC) -pipe -nostdinc -O1 -fno-builtin -I. -MD -fno-omit-frame-pointer -Wall -Wno-format -Wno-unused -Werror -gstabs -m32 -fno-tree-ch -fno-stack-protector -gstabs -c -o $@ $<

$(OBJDIR)/kernel/%.o: kernel/%.c
	@echo + cc $<
	@mkdir -p $(@D)
	$(CC) -I. -c -o $@ $<
