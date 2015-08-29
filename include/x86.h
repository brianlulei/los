#ifndef _X86_H_
#define _X86_H_
#include <include/types.h>

// 
static __inline uint8_t inb(int port) __attribute__((always_inline));
static __inline void insl(int port, void *addr, int cnt) __attribute__((always_inline));

static __inline void outb(int port, uint8_t data) __attribute__((always_inline));
static __inline void outw(int port, uint16_t data) __attribute__((always_inline));
static __inline uint32_t read_ebp(void) __attribute__((always_inline));
static __inline uint32_t read_esp(void) __attribute__((always_inline));


static __inline uint32_t rcr0(void) __attribute__((always_inline));
static __inline void lcr0(uint32_t val) __attribute__((always_inline));

static __inline uint32_t rcr2(void) __attribute__((always_inline));

static __inline void lcr3(uint32_t val) __attribute__((always_inline));

static __inline void lldt(uint16_t sel) __attribute__((always_inline));
static __inline void lgdt(void *p) __attribute__((always_inline));
static __inline void ltr(uint16_t sel) __attribute__((always_inline));
static __inline void lidt(void *p) __attribute__((always_inline));

static __inline void invlpg(void *addr) __attribute__((always_inline));

static __inline uint32_t read_eflags(void) __attribute__((always_inline));

static __inline uint8_t
inb(int port)
{
	uint8_t data;
	__asm __volatile("inb %w1, %0" : "=a" (data) : "d" (port));
	return data;
}

static __inline void
insl(int port, void *addr, int cnt)
{
	__asm __volatile("cld\n\trepne\n\tinsl"				:
					 "=D" (addr), "=c" (cnt)			:
					 "d" (port), "0" (addr), "1" (cnt)	:
					 "memory", "cc");
}

static __inline void
outb(int port, uint8_t data)
{
	__asm __volatile("outb %0, %w1" : : "a" (data), "d" (port));
}

static __inline void
outw(int port, uint16_t data)
{
	__asm __volatile("outw %0, %w1" : : "a" (data), "d" (port));
}

static __inline uint32_t
read_ebp(void)
{
	uint32_t ebp;
	__asm __volatile("movl %%ebp, %0" : "=r" (ebp));
	return ebp;
}

static __inline uint32_t
read_esp(void)
{
	uint32_t esp;
	__asm __volatile("movl %%esp, %0" : "=r" (esp));
	return esp;
}

static __inline void 
lcr0(uint32_t val) 
{
	__asm __volatile("movl %0, %%cr0" : : "r" (val));
}

static __inline uint32_t
rcr0(void)
{
	uint32_t val;
	__asm __volatile("movl %%cr0, %0" : "=r" (val));
	return val;
}

static __inline void
lcr3(uint32_t val)
{
	__asm __volatile("movl %0, %%cr3": : "r" (val));
}

static __inline uint32_t
rcr2(void)
{
	uint32_t val;
	__asm __volatile("movl %%cr2, %0" : "=r" (val));
	return val;
}

static __inline void
invlpg(void *addr)
{
	__asm __volatile("invlpg (%0)" : : "r" (addr): "memory");
}

static __inline void
lgdt(void *p)
{
	__asm __volatile("lgdt (%0)" : : "r" (p));
}

static __inline void
lldt(uint16_t sel)
{
	__asm __volatile("lldt %0" : : "r" (sel));
}

static __inline void
ltr(uint16_t sel)
{
	__asm __volatile("ltr %0" : : "r" (sel));
}

static __inline void
lidt(void *p)
{
	__asm __volatile("lidt (%0)" : : "r" (p));
}

static __inline uint32_t
read_eflags(void)
{
	uint32_t eflags;
	__asm __volatile("pushfl; popl %0" : "=r" (eflags));
	return eflags;
}

static inline uint32_t
xchg(volatile uint32_t *addr, uint32_t newval)
{
	uint32_t	result;

	// The + in "+m" denotes a read-modify-write operand.
	asm volatile("lock; xchgl %0, %1" :
				 "+m" (*addr), "=a" (result) :
				 "1" (newval) :
				 "cc");
	return result;
}

#endif
