#include <include/x86.h>
#include <kernel/kclock.h>

unsigned
mc146818_read(unsigned reg)
{
	outb(OUT_RTC, reg);
	return inb(IN_RTC);
}

void
mc146818_write(unsigned reg, unsigned datum)
{
	outb(OUT_RTC, reg);
	outb(IN_RTC, datum);
}
