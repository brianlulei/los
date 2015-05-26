OBJDIR := obj

include boot/Makefile

all: $(OBJDIR)/boot/main.o $(OBJDIR)/boot/boot.o
