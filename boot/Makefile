BOOT_OBJS = boot/boot.o boot/main.o


boot/%.o: boot/%.S
	@echo + as $<
	@mkdir -p $(@D)
	$(CC) -I. -nostdinc -Os -c -o $@ $<

boot/%.o: boot/%.c
	@echo + cc -Os $<
	$(CC) -I. -nostdinc -Os -c -o $@ $<
