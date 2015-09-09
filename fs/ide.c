/*
 * Minimal PIO-based (non-interrupt-driven) IDE driven code.
 * For information about what all this IDE/ATA magic means,
 * see the materials avialable online.
 */

#include <include/x86.h>

#define IDE_BSY		0x80
#define IDE_RDY		0x40
#define	IDE_DF		0x20
#define IDE_ERR		0x01

static int diskno = 1;

static int
ide_wait_ready(bool check_error)
{
	int r;

	while (((r = inb(0x1F7)) & (IDE_BSY | IDE_RDY)) != IDE_RDY)
		/* do nothing */ ;

	if (check_error && (r & (IDE_DF | IDE_ERR)) != 0)
		return -1;

	return 0;
}

bool
ide_probe_disk1(void)
{
	int r, x;

	// wait for Device 0 to be ready
	ide_wait_ready(0);

	// check for Device 1 to be ready for a while
	outb(0x1F6, 0xE0 | (1 << 4));

	// check for Device 1 to be ready for a while
	for (x = 0;
		 x < 1000 && ((r = inb(0x1F7)) & (IDE_BSY | IDE_DF | IDE_ERR)) != 0;
		 x ++)
		/* do nothing */;

	// switch back to Device 0
	outb(0x1F6, 0xE0 | (0 << 4));
	cprintf("Device 1 presence: %d\n", (x < 1000));
	return (x < 1000);
}

void
ide_set_disk(int d)
{
	if (d != 0 && d != 1)
		panic("bad disk number");
	diskno = d;
}



