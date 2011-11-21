#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

int copy (const char* srce, const char* dest) {
	
	FILE *srcfile = fopen (srce,"rb");
	if (srcfile==NULL) {
		fprintf(stderr, "%s: file could not be read: %d: %s\n", srce, errno, strerror(errno));
		return 1;
	}

	FILE *destfile = fopen (dest,"wb");
	if (destfile == NULL) {
		fprintf(stderr, "%s: file write error: %d: %s\n", dest, errno, strerror(errno));
		return 1;
	}

	int tmpch;
	while ((tmpch = fgetc(srcfile)) !=EOF)
		putc(tmpch, destfile);

	if (fflush(destfile)) {
		fprintf(stderr,"error while emptying buffer: %d: %s\n", errno, strerror(errno));
		return 1;
	}

	if (ferror(destfile)) {
		fprintf(stderr,"%s: file write error: %d: %s\n", dest, errno, strerror(errno));
		return 1;
	}

	if (fclose(destfile)) {
		fprintf(stderr,"%s: error closing file: %d: %s\n", dest, errno, strerror(errno));
		return 1;
	}
	return 0;
}

int main (char argc, const char **argv) {
	
	if (argc != 3) {
		fprintf(stderr,"invalid number of arguments: %d: %s\n", errno, strerror(errno));
		return EXIT_FAILURE;
	}
	
	const char* srce = argv[1];
	const char* dest = argv[2];

	int failure = copy(srce, dest);
	if (failure) {
		fprintf(stderr,"copy failure: %d: %s\n", errno, strerror(errno));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
