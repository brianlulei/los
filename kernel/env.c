#include <kernel/env.h>
#include <include/mmu.h>
#include <include/x86.h>

struct Env *envs	= NULL;			// All environments
struct Env *curenv	= NULL;			// Current env
static struct Env *	env_free_list;	// Free environment list
									// (linked by Env->env_link)

/********************************************************************
 * Global descriptor table.
 *
 * Set up global descriptor table (GDT) with separate segments for 
 * kernel mode and user mode. Segments serve many purposes on the x86.
 * We don't use any of their memory-mapping capabilities, but we need
 * them to switch privilege levels.
 *
 * The kenel and user segments are identical except for the DPL.
 * To load the SS register, the CPL must equal the DPL. Thus, 
 * we must duplicate the segments for the user and the kernel.
 *
 * In particular, the last argument to the SEG macro used in the
 * definition of gdt specifies the Descriptor Privilege Level (DPL)
 * of that descriptor: 0 for kernel and 3 for user.
 ********************************************************************/

struct Segdesc gdt[] = 
{
	// 0x0 - unused (always faluts -- for trapping NULL far pointers)
	SEG_NULL,

	// 0x01 - kernel code segment
	[GD_KT >> 3] = SEG(STA_X | STA_R, 0x0, 0xffffffff, 0),

	// 0x10 - kernel data segment
	[GD_KD >> 3] = SEG(STA_W, 0x0, 0xffffffff, 0),

	// 0x11 - user code segment
	[GD_UT >> 3] = SEG(STA_X | STA_R, 0x0, 0xffffffff, 3),

	// 0x100 - user data segment
	[GD_UD >> 3] = SEG(STA_W, 0x0, 0xffffffff, 3),

	// 0x101 - tss, initialized in trap_init_percpu()
	[GD_TSS0 >> 3] = SEG_NULL
};

struct Pseudodesc gdt_pd = {
	sizeof(gdt) - 1, (unsigned long) gdt
};



/********************************************************************
 * Mark all environements in 'envs' as free, set their env_ids to 0,
 * and insert them into the env_free_list.
 * Make sure the environments are in the free list in the same order
 * they are in the envs array (i.e. so that the first call to
 * env_alloc() returns envs[0] 
 ********************************************************************/
void
env_init(void)
{
	// Set up envs array
	
	env_init_percpu();	
}

void
env_init_percpu()
{
	lgdt(&gdt_pd);

	// The kernel never uses GS or FS, so we leave those set to
	// the user data segment.
	asm volatile("movw %%ax, %%gs" :: "a" (GD_UD|3));	
	asm volatile("movw %%ax, %%fs" :: "a" (GD_UD|3));

	asm volatile("movw %%ax, %%es" :: "a" (GD_KD));
	asm volatile("movw %%ax, %%ds" :: "a" (GD_KD));
	asm volatile("movw %%ax, %%ss" :: "a" (GD_KD));

	// Load the kernel text segment into CS.
	asm volatile("ljmp %0, $1f\n 1: \n" :: "i" (GD_KT));

	// For good measure, clear the local descriptor table (LDT),
	// since we don't use it.
	lldt(0);
}
