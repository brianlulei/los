#ifndef _MONITOR_H_
#define _MONITOR_H_

struct Trapframe;

// Functions implementing moniror commands.
int		mon_backtrace(int argc, char **argv, struct Trapframe *tf);
void	monitor(struct Trapframe *tf);
#endif
