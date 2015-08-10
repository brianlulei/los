#include <kernel/cpu.h>
#include <kernel/pmap.h>
#include <kernel/cpu.h>

#include <include/string.h>
#include <include/x86.h>

CpuInfo cpus[NCPU];
CpuInfo *bootcpu;
int ismp;
int ncpu;

// Per-CPU kernel stacks
unsigned char percpu_kstacks[NCPU][KSTKSIZE]
__attribute__ ((aligned(PGSIZE)));

/* See MultiProcessor Specification Verstion 1.14 */

struct mp {							// floating pointer
	uint8_t		signature[4];		// "_MP_"
	physaddr_t	physaddr;			// physical address of MP config table
	uint8_t		length;				// 1
	uint8_t		specrev;			// [14]
	uint8_t		checksum;			// all bytes must add up to 0
	uint8_t		type;				// MP system config type
	uint8_t		imcrp;
	uint8_t		reserved[3];
} __attribute__((__packed__));

struct mpconf {
	uint8_t		signature[4];		// "PCMP"
	uint16_t	length;				// total table length
	uint8_t		version;			// [14]
	uint8_t		checksum;			// all bytes must add up to 0
	uint8_t		product[20];		// product id
	physaddr_t	oemtable;			// OEM table pointer
	uint16_t	oemlength;			// OEM table length
	uint16_t	entry;				// entry count
	physaddr_t	lapicaddr;			// address of local APIC
	uint16_t	xlength;			// extended table length
	uint8_t		xchecksum;			// externded table checksum
	uint8_t		reserved;
	uint8_t		entries[0];			// table entries
} __attribute__((__packed__));

struct mpproc {						// processor table entry
	uint8_t		type;				// entry type
	uint8_t		apicid;				// local APIC id
	uint8_t		version;			// local APIC version
	uint8_t		flags;				// CPU flags
	uint8_t		signature[4];		// CPU signature
	uint32_t	feature;			// feature flags from CPUID instruction
	uint8_t		reserved[8];
} __attribute__((__packed__));

// mproc flags
#define MPPROC_BOOT		0x02		// This mpproc is the bootstrap processor

// Table entry types
#define MPPROC			0x00		// One per processor
#define MPBUS			0x01		// One per bus
#define MPIOAPIC		0x02		// One per I/O APIC
#define MPIOINTR		0x03		// One per bus interrupt source
#define MPLINTR			0x04		// One per system interrupt source

static uint8_t
sum(void *addr, int len)
{
	int i, sum = 0;

	for (i = 0; i < len; i++)
		sum += ((uint8_t *) addr)[i];

	return sum;
}


static struct mp *
mpsearch1(physaddr_t a, int len)
{
	struct mp *mp = KADDR(a), *end = KADDR(a + len);

	for (; mp < end; mp++)
		if (memcmp(mp->signature, "_MP_", 4) == 0 &&
			sum(mp, sizeof(*mp)) == 0)
			return mp;

	return NULL;
}

/**********************************************************************
 * Search for the MP Floating Pointer structure, which according to
 * [MP 4] is in one of the following three locations:
 * 1. in the first KB of the EBDA;
 * 2. if there is no EBDA, in the last KB of system base memory;
 * 3. in the BIOS ROM between 0xE0000 and 0xFFFFF.
 **********************************************************************/
static struct mp*
mpsearch(void)
{
	uint8_t *bda;
	uint32_t p;
	struct mp *mp;

	static_assert(sizeof(*mp) == 16);

	// The BIOS data area lives in 16-bit segment 0x40.
	bda = (uint8_t *) KADDR(0x40 << 4);

	// The starting address of the EBDA segment for 
	// EISA or MCA systems can be found in a two byte
	// location (40:0Eh) of the BIOS data area. 
	if ((p = *(uint16_t *) (bda + 0x0E))) {
		p <<= 4;	// Translate EBDA segment to PA
		if ((mp = mpsearch1(p, 1024)))
			return mp;
	} else {
		// The BIOS reports the base memory size in a two-byte 
		// location (40:13h) of the BIOS data area.
		// The base memory size is reported in kilobytes minus 1K, 
		// which is used by the EBDA segment or for other purposes.
		p = *(uint16_t *) (bda + 0x13) * 1024; // base memory end addr
		if ((mp = mpsearch1(p - 1024, 1024)))
			return mp;
	}
	return mpsearch1(0xE0000, 0x1FFFF);
}


// Search for an MP configuration table.
// Check for the correct signature, checksum, and version.
static struct mpconf *
mpconfig(struct mp **pmp)
{
	struct mpconf *conf;
	struct mp *mp;

	if ((mp = mpsearch()) == 0)
		return NULL;

	if (mp->physaddr == 0 || mp->type != 0) {
		cprintf("SMP: Default configuration not implemented\n");
		return NULL;
	}

	// change to virtual address
	conf = (struct mpconf *) KADDR(mp->physaddr);

	if (memcmp(conf, "PCMP", 4) != 0) {
		cprintf("SMP: Incorrect MP configuration table signature\n");
		return NULL;
	}

	if (sum(conf, conf->length) != 0) {
		cprintf("SMP: Bad MP configuration checksum\n");
		return NULL;
	}
	
	if (conf->version != 1 && conf->version != 4) {
		cprintf("SMP: Unsupported MP verstion %d\n", conf->version);
		return NULL;
	}

	if ((sum((uint8_t *)conf + conf->length, conf->xlength) + conf->xchecksum) & 0xff) {
		cprintf("SMP: Bad MP configuration extended checksum\n");
		return NULL;
	}
	*pmp = mp;
	return conf;
}

void
mp_init(void)
{
	struct mp *mp;
	struct mpconf *conf;
	struct mpproc *proc;
	uint8_t *p;
	unsigned int i;

	bootcpu = &cpus[0];
	if ((conf = mpconfig(&mp)) == 0)
		return;

	ismp = 1;
	lapicaddr = conf->lapicaddr;

	for (p = conf->entries, i = 0; i < conf->entry; i++) {
		switch (*p) {
			case MPPROC:
				proc = (struct mpproc *)p;
				if (proc->flags & MPPROC_BOOT)
					bootcpu = &cpus[ncpu];

				if (ncpu < NCPU) {
					cpus[ncpu].cpu_id = ncpu;
					ncpu++;
				} else {
					cprintf("SMP: too many CPUs, CPU %d disabled\n",
							proc->apicid);
				}
				p += sizeof(struct mpproc);
				continue;
			case MPBUS:
			case MPIOAPIC:
			case MPIOINTR:
			case MPLINTR:
				p += 8;
				continue;
			default:
				cprintf("mpinit: unknown config type %x\n", *p);
				ismp = 0;
				i = conf->entry;
		}
	}

	bootcpu->cpu_status = CPU_STARTED;
	if (!ismp) {
		// Fall back to no MP
		ncpu = 1;
		lapicaddr = 0;
		cprintf("SMP: configuration not found, SMP disabled");
		return;
	}

	cprintf("SMP: CPU %d found %d CPU(s)\n", bootcpu->cpu_id, ncpu);

	if (mp->imcrp) {
		// If the hardware implements PIC mode,
		// switch to getting interrupts from the LAPIC.
		cprintf("SMP: Setting ICMCR to switch from PIC mode to symmetric I/O mode\n");
		outb(0x22, 0x70);			// Select IMCR
		outb(0x23, inb(0x23) | 1);	// Mask external interrupts.
	}
}
