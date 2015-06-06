BOOT_OBJS = $(OBJDIR)/boot/boot.o $(OBJDIR)/boot/main.o

$(OBJDIR)/boot/%.o: boot/%.S
	@echo + as $<
	@mkdir -p $(@D)
	$(CC) -nostdinc $(KERN_CFLAGS) -Os -c -o $@ $<

$(OBJDIR)/boot/%.o: boot/%.c
	@echo + cc -Os $<
	@mkdir -p $(@D)
	$(CC) -nostdinc $(KERN_CFLAGS) -c -o $@ $<

$(OBJDIR)/boot/sign: boot/sign.c
	@echo + cc $<
	@mkdir -p $(@D)
	$(CC) -I. -o $@ $<

$(OBJDIR)/boot/boot: $(BOOT_OBJS) $(OBJDIR)/boot/sign
	@echo + ld boot/boot
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 -o $@.out $(BOOT_OBJS)
	$(OBJDUMP) -S $@.out > $@.asm
	$(OBJCOPY) -S -O binary -j .text $@.out $@
	$(OBJDIR)/boot/sign $(OBJDIR)/boot/boot
	
