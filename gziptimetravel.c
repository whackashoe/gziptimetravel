/* Copyright (C) 2013 Jett LaRue
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <utime.h>

#include "gziptimetravel.h"


int main(int argc, char ** argv) 
{
    struct Flags flags;
    int i; /*for loop iterator*/
    int c; /*option iteration*/

    flags.prettyPrintTime     = 0;
    flags.printSrc            = 0;
    flags.setTime             = 0;
    flags.setModificationTime = 0;
    flags.newTime             = 0;

    opterr = 0;
    while((c = getopt (argc, argv, "pnms:S-:")) != -1) {
        switch(c) {
            case 'p':
                flags.prettyPrintTime = 1;
                break;
            case 'n':
                flags.printSrc = 1;
                break;
            case 'm':
                flags.setModificationTime = 1;
                break;
            case 's':
                flags.setTime = 1;
                strncpy(flags.newTimeStr, optarg, sizeof(flags.newTimeStr)-1);
                flags.newTimeStr[sizeof(flags.newTimeStr)-1] = '\0';
                flags.newTime = convertStrToTime(flags.newTimeStr);
                break;
            case 'S':
                flags.setTime = 1;
                strncpy(flags.newTimeStr, grabTimeFromStdin(), sizeof(flags.newTimeStr)-1);
                flags.newTime = convertStrToTime(flags.newTimeStr);
                break;
            case '-':
                if(strcmp(optarg, "help") == 0) {
                    displayHelp(argv[0]);
                    exit(EXIT_SUCCESS);
                }
                if(strcmp(optarg, "version") == 0) {
                    displayVersion();
                    exit(EXIT_SUCCESS);
                }
                break;
            case '?':
                if(optopt == 's')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if(isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);

                fprintf(stderr, "Try %s --help\n", argv[0]);
                exit(EXIT_FAILURE);
            default:
                exit(EXIT_FAILURE);
        }
    }
    
    if(optind >= argc) {
        fprintf(stderr, "Missing file operand (\"%s --help\" for help)\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    argc -= optind;
    argv += optind;

    for (i = 0; i < argc; ++i) {
        gziptimetravel(&flags, argv[i]);
    }

    exit(EXIT_SUCCESS);
    return 0;
}

int gziptimetravel(const struct Flags * flags, const char * filesrc)
{
    int ret = 1;
    FILE * fp;
    unsigned char header[8];
    size_t header_l;
    time_t file_mtime;

    fp = fopen(filesrc, (flags->setTime) ? "r+b" : "r");
    if(fp == NULL) {
        perror(filesrc);
        return 0;
    }

    header_l = fread(header, sizeof(unsigned char), sizeof(header), fp);
    if(header_l < sizeof(header)) {
        fprintf(stderr, "Only read %lu bytes\n", header_l);
        ret = 0;
        goto closeFile;
    }

    if(!verifyGzipHeader(header)) {
        fprintf(stderr, "File: %s is not a gzip archive\n", filesrc);
        ret = 0;
        goto closeFile;
    }

    file_mtime = getTime(header+4);

    if(flags->printSrc) printFileSrc(filesrc);

    if(!flags->prettyPrintTime && !flags->setTime)
        printSecondsFromEpoch(file_mtime);

    if(flags->prettyPrintTime)
        prettyPrintTime(file_mtime);

    if(flags->setTime)
        if(!setFileTime(fp, flags->newTime)) {
            perror(filesrc);
            ret = 0;
            printf("dlasdlasdas");
            goto closeFile;
        }

closeFile:
    fclose(fp);

    if(flags->setModificationTime) {
        if(!setFileModificationTime(filesrc, file_mtime)) {
            perror(filesrc);
            ret = 0;
        }
    }

    return ret;
}

int setFileModificationTime(const char * src, time_t mtime)
{
    struct utimbuf ubuf;
    ubuf.modtime = mtime;
    time(&ubuf.actime);
    if(utime(src, &ubuf) != 0) 
        return 0;
    else
        return 1;
}

unsigned long convertStrToTime(const char * str)
{
    unsigned long t;

    errno = 0;
    t = strtoul(str, '\0', 0);
    if(errno != 0) {
        fprintf(stderr, "Conversion of time failed, EINVAL, ERANGE\n");
        exit(EXIT_FAILURE);
    }

    return t;
}

int verifyGzipHeader(const unsigned char header[2])
{
    /* gzip's 8 byte header format:
     * bytes name desc
     * 1: ID1   -- (IDentification 1)
     * 1: ID2   -- (IDentification 2)
     * 1: CM    -- (Compression Method) 
     * 1: FLG   -- (FLaGs)
     * 4: MTIME -- (Modification TIME)
     */
    const unsigned char ID1 = 0x1f;
    const unsigned char ID2 = 0x8b;

    if(header[0] != ID1 || header[1] != ID2)
        return 0;

    return 1;
}

char * grabTimeFromStdin()
{
    char * newtime = NULL;
    size_t newtime_l;

    fflush(stdout);
    if(!fgets(newtime, sizeof(newtime), stdin)) {
        fprintf(stderr, "Error reading from stdin\n");
        exit(EXIT_FAILURE);
    }

    newtime_l = strlen(newtime);
    if(newtime[newtime_l-1] == '\n') newtime[--newtime_l] = '\0';

    return newtime;
}

time_t getTime(const unsigned char vals[4])
{
    time_t mtime = (vals[0])       +
                   (vals[1] << 8)  +
                   (vals[2] << 16) +
                   (vals[3] << 24);
    return mtime; 
}

int setFileTime(FILE * fp, const unsigned long ntime)
{
    if(    (fseek(fp, 4, SEEK_SET) != 0)
        || (fputc((ntime       & 0xFF), fp) == EOF)
        || (fputc((ntime >> 8  & 0xFF), fp) == EOF)
        || (fputc((ntime >> 16 & 0xFF), fp) == EOF)
        || (fputc((ntime >> 24 & 0xFF), fp) == EOF)) return 0;

    return 1;
}

void printFileSrc(const char * src)
{
    printf("%s\t", src);
}

void printSecondsFromEpoch(const time_t t)
{
    printf("%lu\n", t);
}

void prettyPrintTime(const time_t t)
{
    char gheaderbuffer[100];
    strftime(gheaderbuffer, 100, "%Y-%m-%d %H:%M:%S", localtime(&t));
    printf("%s\n", gheaderbuffer);
}

void displayHelp(const char * name)
{
    printf("Usage: %s [OPTIONS...] [FILES...]\n"
           "Set or view timestamp of gzip archives\n"
           "\n"
           "  -p                       print formatted timestamp to stdout\n"
           "  -n                       print name of file to stdout\n"
           "  -m                       set files modification timestamp to mtime value\n"
           "  -s [seconds from epoch]  set timestamp of file\n"
           "  -S                       set timestamp of file from stdin\n"
           "\n"
           "If no flags other than file are given %s will output inputs timestamp as seconds since epoch\n"
           "\n", name, name);
    printf("EXAMPLES:\n"
           "  %s input.tar.gz\n"
           "                      Print input.tar.gz's timestamp\n"
           "\n"
           "  %s -s0 input.tar.gz\n"
           "                      Set input.tar.gz's timestamp to January 1st 1970\n"
           "\n"
           "  echo \"123456789\" | %s -S input.tar.gz\n"
           "                      Set input.tar.gz's timestamp to 1973-11-29 November 29th 1973\n"
           "\n"
           "\n", name, name, name);
    printf("Report gziptimetravel bugs to whackashoe@gmail.com\n"
           "gziptimetravel homepage: <https://github.com/whackashoe/gziptimetravel/>\n");
}

void displayVersion(void)
{
    printf("%s%d%c%d\n", "gziptimetravel ", GZIPTIMETRAVEL_VERSION_MAJOR, '.', GZIPTIMETRAVEL_VERSION_MINOR);
}