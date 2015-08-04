#include <include/trap.h>

#include <kernel/picirq.h>

// Current IRQ mask.
// Initial IRQ mask has interrupt 2 enabled (for slave 8259A).
uint16_t irq_mask_5259A = 0xFFFF & ~(1 <<IRQ_SLAVE);
static bool didinit;

/* Initialize the 8259A interrupt controllers */
void
pic_init(void)
{
	
}
