OBJDIRS += fs

FSOFILES := $(OBJDIR)/fs/serv.o \
			$(OBJDIR)/fs/fs.o \
			$(OBJDIR)/fs/ide.o \

USERAPPS :=	$(OBJDIR)/user/init

FSIMGTXTFILES :=	fs/newmotd

$(OBJDIR)/fs/%.o: fs/%.c
	@echo + cc[USER] $<
	@mkdir -p $(@D)
	$(CC) -nostdinc $(USER_CFLAGS) -c -o $@ $<

$(OBJDIR)/fs/fs: $(FSOFILES) $(OBJDIR)/lib/entry.o $(OBJDIR)/lib/liblos.a user/user.ld
	@echo + ld $@
	@mkdir -p $(@D)
	$(LD) -o $@ $(ULDFLAGS) $(LDFLAGS) -nostdlib \
		$(OBJDIR)/lib/entry.o $(FSOFILES) \
		-L$(OBJDIR)/lib -llos $(GCC_LIB)
	$(OBJDUMP) -S $@ >$@.asm

# How to build the file system image
$(OBJDIR)/fs/fsformat: fs/fsformat.c
	@echo + mk $(OBJDIR)/fs/fsformat
	@mkdir -p $(@D)
	$(NCC) $(NATIVE_CFLAGS) -o $(OBJDIR)/fs/fsformat fs/fsformat.c

$(OBJDIR)/fs/clean-fs.img: $(OBJDIR)/fs/fsformat $(FSINGFILES)
	@echo + mk $(OBJDIR)/fs/clean-fs.img
	@mkdir -p $(@D)
	$(OBJDIR)/fs/fsformat $(OBJDIR)/fs/clean-fs.img 1024 $(FSINGFILES)

$(OBJDIR)/fs/fs.img: $(OBJDIR)/fs/clean-fs.img
	@echo + cp $(OBJDIR)/fs/clean-fs.img $@
	@cp $(OBJDIR)/fs/clean-fs.img $@

all: $(OBJDIR)/fs/fs.img
