#include <stdio.h>
#define MAX 1000

int
main(int argc, char *argv[])
{
	
	char buffer[MAX];
	FILE * fd;
	fd = fopen(argv[1], "rb");

	if (!fd) {
		printf("Unable to open the file: %s.", argv[1]);
		return 1;
	}

	int length = fread(buffer, sizeof(char), MAX, fd);

	if (length > 510) {
		printf("Boot block too large: %d bytes (max 510)\n", length);
		return 1;
	}

	printf("Boot block is %d bytes (max 510)\n", length);
	fclose(fd);

	fd = fopen(argv[1], "ab");

	int length_to_write = 510 - length;

	char to_write = '\x00';
	int i;
	for (i = 0; i < length_to_write; i++) {
		fwrite(&to_write, sizeof(char), 1, fd);
	}
	to_write = '\x55';
	fwrite(&to_write, sizeof(char), 1, fd);
	to_write = '\xAA';
	fwrite(&to_write, sizeof(char), 1, fd);
	fclose(fd);
}
