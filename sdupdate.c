#define _POSIX_C_SOURCE 2
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

unsigned long buffparser (char* buff) {
    int size = strlen(buff);
    char lastch = buff[size-1];
    if (lastch >= '0' && lastch <= '9')
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

int copy (const char* srce, const char* dest, size_t buffsize) {
    
    FILE *srcfile = fopen (srce,"rb");
    if (srcfile==NULL) {
        fprintf(stderr, "%s: file could not be read: %d: %s\n", srce, errno,
                strerror(errno));
        return 1;
    }
    
    FILE *destfile;
    
    //if cannot open r+b then file doesn't exist so open w+b (create it)
    if ((destfile = fopen (dest,"r+b")) == NULL)
        if ((destfile = fopen (dest,"w+b")) == NULL) {
            fprintf(stderr, "%s: file could not be read: %d: %s\n", dest, errno,
                    strerror(errno));
            return 1;
        }

    char* tmpch;
    char* cmpch;
    if ( (tmpch = malloc(buffsize)) == NULL) {
        fprintf(stderr,"%lu: memory allocation error: %d: %s\n", buffsize, 
                errno, strerror(errno));
        return EXIT_FAILURE;
    }
    if ( (cmpch  = malloc(buffsize)) == NULL) {
        fprintf(stderr,"%lu: memory allocation error: %d: %s\n", buffsize, 
                errno, strerror(errno));
        return EXIT_FAILURE;
    }
    long offset = buffsize;
    int lastw;
    int blocksize = 1;

    do {

        lastw = fread (tmpch, blocksize, buffsize, srcfile);

        //if cmpch contains something then compare and write
        if (fread (cmpch, blocksize, lastw, destfile)) {
                        
            //compare corresponding blocks in src and dest and if equal skip
            if (memcmp(cmpch,tmpch,lastw)!=0) {
                fseek(destfile, -offset, SEEK_CUR);
                fwrite(tmpch, blocksize, lastw, destfile);
                if (ferror(destfile)) {
                    fprintf(stderr,"%s: first file write error: %d: %s\n", dest, 
                            errno, strerror(errno));
                    return 1;
                }   
            }   
        }
       
        //if cmpch is empty then don't bother, just  
        else
            fwrite(tmpch, blocksize, lastw, destfile);           

    //on the last instance fread will have nothing left to read so lastw = 0
    } while (lastw > 0);

    if (ferror(srcfile)) {
        fprintf(stderr,"%s: file read error: %d: %s\n", srce, errno,
                strerror(errno));
        
        return 1;
    }

    if (fflush(destfile)) {
        fprintf(stderr,"error while emptying buffer: %d: %s\n", errno,
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
    free(tmpch);
    free(cmpch);
    return 0;
}

int main (int argc, char **argv) {
    
    char* fname = argv[0];

    unsigned long buffsize = 1024*1024;
    int opt;

    while ((opt=getopt(argc,argv,"b:"))!=-1) {
        switch (opt) {
            case 'b':
                if ((buffsize = buffparser(optarg)) == 0) {
                    fprintf(stderr,"%s: invalid buffer multiplier\n", fname);
                    return EXIT_FAILURE;
                }
                
                if (buffsize <= 0) {
                    fprintf(stderr,"%s: invalid buffer size: %lu\n", fname,
                            buffsize);
                    return EXIT_FAILURE;
                }
                /* buffsize is used as a long later on so cannot be a value
                   between l_max and lu_max */
                if (buffsize >= LONG_MAX) {
                    fprintf(stderr,"%s: exorbitant buffer size: %lu\n", fname,
                            buffsize);
                    return EXIT_FAILURE;
                }
                                
                break;
            
            case '?':
                /*  if -b is not given a value then getopt returns '?' and stores
                    b in optopt */
                if (optopt=='b') {
                    fprintf(stderr,"%s: invalid argument to -%c\n", fname,
                            optopt);
                    return EXIT_FAILURE;
                }
                fprintf(stderr, "Usage: %s [-b <buffer>K/M/G] SOURCE DEST\n",
                        fname);
                break;

        }
    }

    if (argv[optind] == NULL) {
        fprintf(stderr,"%s: no source file specified\n", fname);
        fprintf(stderr, "Usage: %s [-b <buffer>K/M/G] SOURCE DEST\n",
                fname);
        return EXIT_FAILURE;
    }
    if (argv[optind+1] == NULL) {
        fprintf(stderr,"%s: no destination file specified\n", fname);
        fprintf(stderr, "Usage: %s [-b <buffer>K/M/G] SOURCE DEST\n",
                fname);
        return EXIT_FAILURE;
    }
    if (argv[optind+2]!=NULL) {
        fprintf(stderr,"%s: unexpected extra argument\n", argv[optind+2]);
        fprintf(stderr, "Usage: %s [-b <buffer>K/M/G] SOURCE DEST\n",
                fname);
        return EXIT_FAILURE;
    }

    const char* srce = argv[optind];
    const char* dest = argv[optind+1];

    int failure = copy(srce, dest, buffsize); 
    if (failure)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;    
}
