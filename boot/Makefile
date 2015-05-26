$(OBJDIR)/boot/%.o: boot/%.S
	@echo + as $<
	@mkdir -p $(@D)
	$(CC) -I. -nostdinc -Os -c -o $@ $<

$(OBJDIR)/boot/%.o: boot/%.c
	@echo + cc -Os $<
	@mkdir -p $(@D)
	$(CC) -I. -nostdinc -Os -c -o $@ $<
