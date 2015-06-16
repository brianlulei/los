#ifndef _MONITOR_H_
#define _MONITOR_H_

struct Trapframe;

void	monitor(struct Trapframe *tf);

// Functions implementing moniror commands.
int		mon_backtrace(int argc, char **argv, struct Trapframe *tf);
int		mon_help(int argc, char **argv, struct Trapframe *tf);
int		mon_kerninfo(int argc, char **argv, struct Trapframe *tf);
#endif
