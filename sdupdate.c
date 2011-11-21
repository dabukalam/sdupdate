#include <stdio.h>
#include <stdlib.h>

int main (char argc, const char **argv) {
	const char* srce = argv[1];
	const char* dest = argv[2];

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
		return EXIT_FAILURE;
	}

	if (a==1) 
		printf("File Copied Successfully.\n");
	
	return 0;
}

int copy (const char* srce, char* dest) {
	if (file_exists(dest)) {
		printf("Error: Destination File Exists\n");
		return 0;
	}
	FILE *srcfile = fopen (srce,"r");
	if (srcfile==NULL) {
		printf("Error: File Could not be read\n");
		return 0;
	}
	FILE *destfile = fopen (dest,"w");
	char tmpch;
	while (!feof(srcfile)) {
		tmpch = fgetc(srcfile);
		putc(tmpch, destfile);
	}
	return 1;
}

int file_exists(char* path) {
	FILE* file;
    if (file = fopen(path,"r"))
		return 1;
	else
		return 0;
}
