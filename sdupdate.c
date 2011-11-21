#include <stdio.h>
#include <stdlib.h>

int copy (const char* srce, const char* dest) {
	
	FILE *srcfile = fopen (srce,"rb");
	if (srcfile==NULL) {
		fprintf(stderr,"File could not be read.\n");
		return 1;
	}

	FILE *destfile = fopen (dest,"wb");
	if (destfile == NULL) {
		fprintf(stderr,"File write error.\n");
		return 1;
	}

	int tmpch;
	while ((tmpch = fgetc(srcfile)) !=EOF)
		putc(tmpch, destfile);

	fflush(destfile);

	if (ferror(destfile)) {
		fprintf(stderr,"File write error.\n");
		return 1;
	}

	fclose(destfile);
	return 0;
}

int main (char argc, const char **argv) {
	
	const char* srce = argv[1];
	const char* dest = argv[2];

	if (argc != 3) {
		fprintf(stderr,"Invalid number of arguments.\n");
		return EXIT_FAILURE;
	}

	int failure = copy(srce, dest);
	if (failure) {
		fprintf(stderr,"Copy Failure"); 
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
