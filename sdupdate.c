/*
 *    Copyright (C) 2011 Codethink Ltd.
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License Version 2 as
 *    published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#define _POSIX_C_SOURCE 2
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h> 
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>
#include <sys/ioctl.h>


#if defined(__linux__)   
#include <linux/fs.h>
#endif


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

char* big (size_t buffsize) {
    char sizes[] = {'K', 'M', 'G', 'T', 'E', 'P', 'Z', 'Y'};
    char* size = malloc(6);
    int count = -1; 
    unsigned long long buff = buffsize;

    while (buff >= 1024) {
        buff /= 1024;
        count++;
    }  

    if (buff > 9999)
        return NULL;

    if (count == -1) {
        sprintf(size, "%d ", (int) buff);
        return size;
    }   
    else {
        sprintf(size, "%d%c", (int) buff, sizes[count]);
        return size;
    }   
}

int far (long where, long end) {
    struct winsize w;
    int i;
    int count = 0;

    int perc = where*100/end;

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    for (i = 0; i < w.ws_col; i++)
        printf("\b");
    
    char* currsize;
    if ((currsize = big(where)) == NULL)
        return 1;

    printf("%5sB synced [>", currsize);
    for (i = 0; i < (w.ws_col-24)*perc/100; i++) {
        printf("\b");
        printf("=>");
        count++;            
    }
    for (i = 0; i < (w.ws_col-24)-count; i++)
        printf(" ");
    printf("] %3d%%  ", perc);

    fflush(stdout);

    return 0;
}


int all_zero(const char* data, size_t size)
{
    int chunks = size / sizeof(uint64_t);
    
    while(chunks--)
    {
        if (*(uint64_t*)data != 0)
            return 0;
            
        data += sizeof(uint64_t);
    }
    
    int bytes = size % sizeof(uint64_t);
    
    for(int n = 0; n < bytes; ++n)
    {
        if (data[n] != 0)
            return 0;
    }
    
    return 1;
}


int copy (const char* srce, const char* dest, size_t buffsize, int progress) {
    
    FILE *srcfile = fopen (srce,"rb");
    if (srcfile == NULL) {
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

    fseek(srcfile, 0, SEEK_END);
    long end = ftell(srcfile);
    rewind(srcfile);
    long where = 0;
    

#if defined(__linux__)   
    int has_trim = 0;
    uint64_t trimstart = 0;
    uint64_t trimlen = 0;
    
    int dst_is_reg = 0;
    
    struct stat dststat;
    
    if (!fstat(fileno(destfile), &dststat)) {
        dst_is_reg = S_ISREG(dststat.st_mode);
        
        has_trim = dst_is_reg || S_ISBLK(dststat.st_mode);
    }
    
    
#endif

    do {

        lastw = fread (tmpch, blocksize, buffsize, srcfile);

#if defined(__linux__)
        if (has_trim && lastw!=0 && all_zero(tmpch, lastw)) {

            if (!trimlen)
                trimstart = where;
            
            trimlen += lastw;
        }
        else if (trimlen) {
            struct fstrim_range range;
            
            range.start = trimstart;
            range.len = trimlen;
            range.minlen = trimlen;
            
            fprintf(stderr, "zero range %llu - %llu\n", (unsigned long long)trimstart, (unsigned long long)trimlen);
            
            if (ioctl(fileno(destfile), FITRIM, &range) != 0) {              
                if (dst_is_reg) {
                    if (fseek(destfile, trimlen, SEEK_CUR)){
                        fprintf(stderr,"%s: file seek error: %d: %s\n", dest, 
                                errno, strerror(errno));
                        return 1;
                    }   
                }
                else while(trimlen--) {
                    fputc(0, destfile);
                    if (ferror(destfile)) {
                        fprintf(stderr,"%s: first file put error: %d: %s\n", dest, 
                                errno, strerror(errno));
                        return 1;
                    }
                }
            }
            
            trimstart = 0;
            trimlen = 0;
        }
        
        if (!trimlen) {
#endif

        //if cmpch contains something then compare and write
        if (fread (cmpch, blocksize, lastw, destfile) == lastw) {
                        
            //compare corresponding blocks in src and dest and if equal skip
            if (memcmp(cmpch,tmpch,lastw) != 0) {
                //rewind destfile to where it was before comparison
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
        {
            fwrite(tmpch, blocksize, lastw, destfile); 
            if (ferror(destfile)) {
                fprintf(stderr,"%s: first file write error: %d: %s\n", dest, 
                        errno, strerror(errno));
                return 1;
            }
        }
            
#if defined(__linux__)
        }
#endif

        if (progress) {
            where += lastw;
            if (far(where, end)) {
                fprintf(stderr,"progress bar failure: %d: %s\n", errno,
                        strerror(errno));
                return 1;
            }
        }

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

    int progress = 0;

    while ((opt = getopt(argc,argv,"pb:")) != -1) {
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
            
            case 'p':
                progress = 1;
                break;

            case '?':
                /*  if -b is not given a value then getopt returns '?' and stores
                    b in optopt */
                if (optopt == 'b') {
                    fprintf(stderr,"%s: invalid argument to -%c\n", fname,
                            optopt);
                    return EXIT_FAILURE;
                }
                fprintf(stderr, "Usage: %s [OPTIONS] SOURCE DEST\n",
                        fname);
                break;

        }
    }

    if (argv[optind] == NULL) {
        fprintf(stderr,"%s: no source file specified\n", fname);
        fprintf(stderr, "Usage: %s [OPTIONS] SOURCE DEST\n",
                fname);
        return EXIT_FAILURE;
    }
    if (argv[optind+1] == NULL) {
        fprintf(stderr,"%s: no destination file specified\n", fname);
        fprintf(stderr, "Usage: %s [OPTIONS] SOURCE DEST\n",
                fname);
        return EXIT_FAILURE;
    }
    if (argv[optind+2] != NULL) {
        fprintf(stderr,"%s: unexpected extra argument\n", argv[optind+2]);
        fprintf(stderr, "Usage: %s [OPTIONS] SOURCE DEST\n",
                fname);
        return EXIT_FAILURE;
    }

    const char* srce = argv[optind];
    const char* dest = argv[optind+1];

    int failure = copy(srce, dest, buffsize, progress); 
    if (failure)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;    
}
