#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

unsigned long buffparser (char* buff) {
    int size = strlen(buff);
    char lastch = buff[size-1];
    if (lastch >= 48 && lastch <= 57)
        return atof(buff);
    else {
        int i;
        char* newbuff = malloc(size-1);
        for (i = 0; i < size-1; i++)
            newbuff[i] = buff[i];
        float mltplr = atof(newbuff);
        switch (lastch) {
            case 'K':
                return mltplr*1024;
            case 'M':
                return mltplr*1024*1024;
            case 'G':
                return mltplr*1024*1024*1024;
            default:
                return 0;
        }
    }
}

int copy (const char* srce, const char* dest, size_t buff) {
    
    FILE *srcfile = fopen (srce,"rb");
    if (srcfile==NULL) {
        fprintf(stderr, "%s: file could not be read: %d: %s\n", srce, errno,
                strerror(errno));
        
        return 1;
    }

    FILE *destfile = fopen (dest,"wb");
    if (destfile == NULL) {
        fprintf(stderr, "%s: file write error: %d: %s\n", dest, errno,
                strerror(errno));
        
        return 1;
    }

    //buffer size of array
    char* tmpch = malloc(buff);
    int count, lastw;
    lastw = count = sizeof(tmpch);

    do {
        //read one character at a time into tmpch
        lastw = fread (tmpch, 1, count, srcfile);
        fwrite(tmpch, blocksize, lastw, destfile);
    } while (lastw > 0);

    if (fflush(destfile)) {
        fprintf(stderr,"error while emptying buffer: %d: %s\n", errno,
                strerror(errno));
        
        return 1;
    }

    if (ferror(srcfile)) {
        fprintf(stderr,"%s: file read error: %d: %s\n", srce, errno,
                strerror(errno));
        
        return 1;
    }

    if (ferror(destfile)) {
        fprintf(stderr,"%s: file write error: %d: %s\n", dest, errno,
                strerror(errno));
        
        return 1;
    }

    if (fclose(destfile)) {
        fprintf(stderr,"%s: error closing file: %d: %s\n", dest, errno,
                strerror(errno));
        
        return 1;
    }
    
    return 0;
}

int main (int argc, char **argv) {
    
    char* fname = argv[0];
    if (argc < 3) {
        fprintf(stderr,"%s: invalid number of arguments\n", fname);
        
        return EXIT_FAILURE;
    }

    unsigned long buff;
    int opt;
    int verbose = 0;

    while ((opt=getopt(argc,argv,"vb:"))!=-1) {
        switch (opt) {
            case 'v':
                verbose = 1;
                break;
            case 'b':
                if ((buff = buffparser(optarg)) == 0) {
                    fprintf(stderr,"%s: invalid buffer multiplier\n", fname);
                    return EXIT_FAILURE;
                }                
                break;
            case '?':
                if (optopt=='b') {
                    fprintf(stderr,"%s: invalid argument to -%c\n", fname,
                            optopt);
                    return EXIT_FAILURE;
                }
                break;
        }
    }

    if (buff<=0) {
        fprintf(stderr,"%s: invalid Buffer Size: %lu\n", fname, buff);
        return EXIT_FAILURE;
    }
        
    
    const char* srce = argv[optind];
    const char* dest = argv[optind+1];
    if (argv[optind+2]!=NULL) {
        fprintf(stderr,"%s: unexpected extra argument\n", argv[optind+2]);
        return EXIT_FAILURE;
    }

    int failure = copy(srce, dest, buff); 
    /*if (failure) {
        fprintf(stderr,"copy failure: %d: %s\n", errno, strerror(errno));
        
        return EXIT_FAILURE;
    }*/

    return EXIT_SUCCESS;    
}
