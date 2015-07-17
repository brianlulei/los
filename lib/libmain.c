// Called from entry.S to get us going.
// entry.S already took care of defining uenvs, upages, uvpd, uvpt

#include <include/lib.h>

extern void umain(int argc, char **argv);

const volatile Env *thisenv;
const char *binaryname = "<unkown>";

void
libmain(int argc, char **argv)
{
	// set thisenv to point at your Env stucture in envs[].
	thisenv = 0;

	// save the name of the program so that panic can use it.
	if (argc > 0)
		binaryname = argv[0];

	// call user main routine
	umain(argc, argv);

	// exit gracefully
	exit();
}
