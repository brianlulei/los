#include <kernel/cpu.h>
#include <kernel/pmap.h>

// Local APIC registers, divided by 4 for use as uint32_t[] indices.
#define ID      (0x0020/4)   // ID
#define VER     (0x0030/4)   // Version
#define TPR     (0x0080/4)   // Task Priority
#define EOI     (0x00B0/4)   // EOI
#define SVR     (0x00F0/4)   // Spurious Interrupt Vector
    #define ENABLE     0x00000100   // Unit Enable
#define ESR     (0x0280/4)   // Error Status
#define ICRLO   (0x0300/4)   // Interrupt Command
    #define INIT       0x00000500   // INIT/RESET
    #define STARTUP    0x00000600   // Startup IPI
    #define DELIVS     0x00001000   // Delivery status
    #define ASSERT     0x00004000   // Assert interrupt (vs deassert)
    #define DEASSERT   0x00000000
    #define LEVEL      0x00008000   // Level triggered
    #define BCAST      0x00080000   // Send to all APICs, including self.
    #define OTHERS     0x000C0000   // Send to all APICs, excluding self.
    #define BUSY       0x00001000
    #define FIXED      0x00000000
#define ICRHI   (0x0310/4)   // Interrupt Command [63:32]
#define TIMER   (0x0320/4)   // Local Vector Table 0 (TIMER)
    #define X1         0x0000000B   // divide counts by 1
    #define PERIODIC   0x00020000   // Periodic
#define PCINT   (0x0340/4)   // Performance Counter LVT
#define LINT0   (0x0350/4)   // Local Vector Table 1 (LINT0)
#define LINT1   (0x0360/4)   // Local Vector Table 2 (LINT1)
#define ERROR   (0x0370/4)   // Local Vector Table 3 (ERROR)
    #define MASKED     0x00010000   // Interrupt masked
#define TICR    (0x0380/4)   // Timer Initial Count
#define TCCR    (0x0390/4)   // Timer Current Count
#define TDCR    (0x03E0/4)   // Timer Divide Configuration

physaddr_t			lapicaddr;		// Initialized in mpconfig.c
volatile uint32_t	*lapic;

static void
lapicw(int index, int value)
{
	lapic[index] = value;
	lapic[ID];				// wait for write to finish, by reading.
}

void
lapic_init(void)
{
	if (!lapicaddr)
		return;

	// lapicaddr is the physical address of the LAPIC's 4K MMIO region.
	// Map it in to virtual memory so we can access it.
	lapic = mmio_map_region(lapicaddr, 4096);

	// Enable local APIC; set spurious interrupt vector.
	lapicw(SVR, ENABLE | (IRQ_OFFSET + IRQ_SPURIOUS));	

	// The timer repeatedly counts down at bus frequency from lapic[TICR]
	// and then issues an interrupt. If we cared more about precise
	// timekeeping, TICR would be calibrated using an external time source.
	lapicw(TDCR, X1);										// divide by 1
	lapicw(TIMER, PERIODIC | (IRQ_OFFSET + IRQ_TIMER));		// periodic mode
	lapicw(TICR, 10000000);									// initial count

	// Leave LINT0 of the BSP enabled so that it can get interrupts from
	// the 8259A chip.
	//
	// According to Intel MP Specification, the BIOS should initialize BSP's
	// local APIC in Virtual Wire Mode, in which 8259A's INTR is virtually
	// connected to BSP's LINTIN0. In this mode, do not need to program the IOAPIC.
	//if (thiscpu != bootcpu)
}
