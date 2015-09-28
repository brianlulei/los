OBJDIRS += lib

LIB_SRCFILES := lib/readline.c \
				lib/string.c \
				lib/printfmt.c \
				lib/printf.c \
				lib/libmain.c \
				lib/syscall.c \
				lib/panic.c \
				lib/exit.c \
				lib/pfentry.S \
				lib/pgfault.c \
				lib/fork.c \
				lib/ipc.c \
				lib/pageref.c

LIB_OBJFILES := $(patsubst lib/%.c, $(OBJDIR)/lib/%.o, $(LIB_SRCFILES))
LIB_OBJFILES := $(patsubst lib/%.S, $(OBJDIR)/lib/%.o, $(LIB_OBJFILES))

$(OBJDIR)/lib/%.o: lib/%.c
	@echo + cc[USER] $<
	@mkdir -p $(@D)
	$(CC) -nostdinc $(USER_CFLAGS) -c -o $@ $<

$(OBJDIR)/lib/%.o: lib/%.S
	@echo + as[USER] $<
	@mkdir -p $(@D)
	$(CC) -nostdinc $(USER_CFLAGS) -c -o $@ $<

$(OBJDIR)/lib/liblos.a: $(LIB_OBJFILES)
	@echo + ar[USER] $@
	$(AR) r $@ $(LIB_OBJFILES)
