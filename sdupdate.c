#include <stdio.h>
#include <stdlib.h>

int main (char argc, char **argv) {
	char* srce = argv[1];
	char* dest = argv[2];

	if (srce == NULL) {
		printf("Error: No Source File Specified\n");
		return EXIT_FAILURE;
	}
	if (dest == NULL) {
		printf("Error: No Destination File Specified\n");
		return EXIT_FAILURE;
	}

	printf("Source: %s\n", srce);
	printf("Destination: %s\n", dest);

	int a = copy(srce, dest);
	if (a!=1) {
		printf("Error: File Could not be read\n");
		return EXIT_FAILURE;
	}
	if (a==1) {
		printf("File Copied Successfully.\n");
	

	return 0;
}

int copy (char* srce, char* dest) {
	FILE *srcfile = fopen (srce,"r");
	FILE *destfile = fopen (dest,"r");
	char tmpch;
	if (srcfile==NULL)
		return 0;
	else {
		while (!feof(srcfile)) {
			tmpch = fgetc(srcfile);
			putc(tmpch, destfile);
		}
		return 1;
	}
}
