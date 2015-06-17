#ifndef _KCLOCK_H_
#define _KCLOCK_H_

/* RTC ports */
#define OUT_RTC			0x70	// out port
#define IN_RTC			0x71	// in port

/* CMOS BASE MEMORY IN KB */
#define NVRAM_BASELO	0x15	// low byte
#define NVRAM_BASEHI	0x16	// high byte

/* CMOS EXTENDED MEMORY IN KB */
#define NVRAM_EXTLO		0x17	// low byte
#define NVRAM_EXTHI		0x18	// high byte

/* CMOS EXTENDED MEMORY POSTED SIZE IN KB */
#define NVRAM_PEXTLO	0x30
#define NVRAM_PEXTHI	0x31

unsigned	mc146818_read(unsigned reg);
void		mc146818_write(unsigned reg, unsigned datum);

#endif
