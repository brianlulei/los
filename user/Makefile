OBJDIRS += user

$(OBJDIR)/user/%.o: user/%.c
	@echo + cc[USER] $<
	@mkdir -p $(@D)
	$(CC) -nostdinc $(USER_CFLAGS) -c -o $@ $<

$(OBJDIR)/user/%: $(OBJDIR)/user/%.o $(OBJDIR)/lib/entry.o user/user.ld $(OBJDIR)/lib/liblos.a
	@echo + ld $@
	$(LD) -o $@ $(ULDFLAGS) $(LDFLAGS) -nostdlib $(OBJDIR)/lib/entry.o $@.o -L$(OBJDIR)/lib -llos $(GCC_LIB)
	$(OBJDUMP) -S $@ > $@.asm
	$(NM) -n $@ > $@.sym
