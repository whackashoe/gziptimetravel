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
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <gziptimetravel.h>

void displayHelp(char * name);
void displayVersion(void);
time_t getTime(const unsigned char vals[4]);
void printSecondsFromEpoch(const time_t t);
void prettyPrintTime(const time_t t);


int main(int argc, char ** argv) 
{
    int printtimeflag  = 0; /*do we pretty print?*/
    int settimeflag    = 0; /*do we alter time on file*/
    int grabfrominflag = 0; /*are we getting input from stdin?*/
    int fileflag       = 0; /*is a file set*/

    const char * filesrc = NULL; /*path to input file passed*/
    char newtime[11]; /*passed in time (since epoch)*/
    char * tnewtime; /*temporary newtime for checking if newtime->ntime conversion succeeded*/
    size_t newtimelength; /*length of newtime buffer*/ 
    unsigned long ntime; /*the value of newtime*/
    FILE *fp; /*input file*/
    unsigned char gheaderbuffer[8]; /*buffer to hold header of gzip input file*/
    size_t gheaderbytes; /*how many bytes read into gheaderbuffer*/
    
    const unsigned char ID1 = 0x1f; /*IDentifier 1 (for gzip detection of file)*/
    const unsigned char ID2 = 0x8b; /*IDentifier 2*/
    
    int i; /*for loop iterator*/
    int c; /*option iteration*/
    opterr = 0;
    while((c = getopt (argc, argv, "ps:S-:")) != -1) {
        switch(c) {
            case 'p':
                printtimeflag = 1;
                break;
            case 's':
                settimeflag = 1;
                strncpy(newtime, optarg, sizeof(newtime)-1);
                newtime[sizeof(newtime)-1] = '\0';
                break;
            case 'S':
                settimeflag = 1;
                grabfrominflag = 1;
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
    
    for (i = optind; i < argc; i++) {
        if(!fileflag) {
            fileflag = 1;
            filesrc = argv[i];
        }
    }
    
    if(!fileflag) {
        fprintf(stderr, "Missing file operand (\"%s --help\" for help)\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fp = fopen(filesrc, (settimeflag) ? "r+b" : "r");
    if(fp == NULL) {
        perror(filesrc);
        exit(EXIT_FAILURE);
    }
    
    gheaderbytes = fread(gheaderbuffer, sizeof(unsigned char), sizeof(gheaderbuffer), fp);
    if(gheaderbytes < sizeof(gheaderbuffer)) {
        fprintf(stderr, "Only read %lu bytes\n", gheaderbytes);
        exit(EXIT_FAILURE);
    }

    /* gzip's 8 byte header format:
     * bytes name desc
     * 1: ID1   -- (IDentification 1)
     * 1: ID2   -- (IDentification 2)
     * 1: CM    -- (Compression Method) 
     * 1: FLG   -- (FLaGs)
     * 4: MTIME -- (Modification TIME)
     */

    if(gheaderbuffer[0] != ID1 || gheaderbuffer[1] != ID2) {
        fprintf(stderr, "File: %s is not a gzip archive\n", filesrc);
        exit(EXIT_FAILURE);
    }

    if(!printtimeflag && !settimeflag)
        printSecondsFromEpoch(getTime(gheaderbuffer+4));

    if(printtimeflag)
        prettyPrintTime(getTime(gheaderbuffer+4));

    if(settimeflag) {
        if(grabfrominflag) {
            fflush(stdout);
            
            if(!fgets(newtime, sizeof(newtime), stdin)) {
                fprintf(stderr, "Error reading from stdin\n");
                exit(EXIT_FAILURE);
            }

            newtimelength = strlen(newtime);
            if(newtime[newtimelength-1] == '\n') newtime[--newtimelength] = '\0';
        }
    
        tnewtime = newtime;
        errno = 0;
        ntime = strtoul(newtime, &tnewtime, 0);
        if(errno != 0) {
            fprintf(stderr, "Conversion of time failed, EINVAL, ERANGE\n");
            exit(EXIT_FAILURE);
        }
        if(*tnewtime != 0) {
            fprintf(stderr, "Conversion of time failed, pass an unsigned integer\n");
            exit(EXIT_FAILURE);
        }

        fseek(fp, 4, SEEK_SET);
        fputc((ntime       & 0xFF), fp);
        fputc((ntime >> 8  & 0xFF), fp);
        fputc((ntime >> 16 & 0xFF), fp);
        fputc((ntime >> 24 & 0xFF), fp);
    }

    fclose(fp);
    return 0;
}

time_t getTime(const unsigned char vals[4])
{
    time_t mtime = (vals[0])       +
                   (vals[1] << 8)  +
                   (vals[2] << 16) +
                   (vals[3] << 24);
    return mtime; 
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

void displayHelp(char * name)
{
    printf("Usage: %s [OPTIONS...] SOURCE\n"
           "Set or view timestamp of gzip archives\n"
           "\n"
           "  -p                       print formatted timestamp to stdout\n"
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
