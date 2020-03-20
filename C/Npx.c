//
//  main.c
//  Test driver for Npx
//
//  Created by Dan Evans on 8/31/17.
//  Copyright © 2017 Dan Evans. All rights reserved.
//
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/resource.h>

#include "Memory.h"
#include "Sstring.h"
#include "StringBuffer.h"
#include "NpxElement.h"

#define self(s) (*s->mtb)

extern int npx2XML(NpxElement *elm, StringBuffer *sb, int nopts, ...);

extern NpxElement *deserialize(char *, int len);
extern void *expatDeserialize(char *buf, int len);
extern char *serialize(NpxElement *);
extern void delDocumentTree(void *root);
extern clock_t getDOMTime();
extern clock_t getTAMTime();


typedef char String;
static char *version = "1.0";
static char *separator = "/";
static jmp_buf env;
static int opt_writexmlc = 0;
static int opt_reserialize = 0;
static int opt_repeat = 1;
static FILE *opt_dupfile = 0;
static int opt_expat = 0;
static char *opt_suffixes[10];

static StringBuffer *wcharToUtf8(wchar_t *wbuf, int ln)
{
    int ix;
    StringBuffer *sb;
    wchar_t ch;
    sb = newStringBuffer((StringBuffer *) 0, 0);
    ix = 0;
    while (ix < ln)
    {
        ch = wbuf[ix];
        if (ch < 0x0080)
        {
            *stringBufferPostInc(sb) = (char)ch;
            ix += 1;
        }
        else if (ch < 0x0800)
        {
            *stringBufferPostInc(sb) = (char)(((ch & 0x07c0) >> 6) |  0xc0);
            *stringBufferPostInc(sb) = (char)(((ch & 0x003f) | 0x80));
            ix += 2;
        }
        else if (ch < 0x10000)
        {
            *stringBufferPostInc(sb) = (char)(((ch & 0xf000) >> 12) |  0xe0);
            *stringBufferPostInc(sb) = (char)(((ch & 0x0fc0) >> 6)  | 0x80);
            *stringBufferPostInc(sb) = (char)((ch & 0x003f) | 0x80);
            ix += 3;
        }
        else if (ch < 0x200000)
        {
            *stringBufferPostInc(sb) = (char)(((ch & 0x1e0000) >> 18) | 0xf0);
            *stringBufferPostInc(sb) = (char)(((ch & 0x03f000) >> 12) | 0x80);
            *stringBufferPostInc(sb) = (char)(((ch & 0x000fc0) >>  6) | 0x80);
            *stringBufferPostInc(sb) = (char)(( ch & 0x00003f) | 0x80);
            ix += 4;
        }
        else if (ch < 0x4000000)
        {
            *stringBufferPostInc(sb) = (char)(((ch & 0x3000000) >> 24) | 0xf80);
            *stringBufferPostInc(sb) = (char)(((ch & 0x0fe0000) >> 18) | 0x80);
            *stringBufferPostInc(sb) = (char)(((ch & 0x003f000) >> 12) | 0x80);
            *stringBufferPostInc(sb) = (char)(((ch & 0x0000fc0) >>  6) | 0x80);
            *stringBufferPostInc(sb) = (char)(( ch & 0x000003f) | 0x80);
            ix += 5;
        }
        else
        {
            *stringBufferPostInc(sb) = (char)(((ch & 0x40000000) >> 30) | 0xfc0);
            *stringBufferPostInc(sb) = (char)(((ch & 0x3f000000) >> 24) | 0x80);
            *stringBufferPostInc(sb) = (char)(((ch & 0x00fe0000) >> 18) | 0x80);
            *stringBufferPostInc(sb) = (char)(((ch & 0x0003f000) >> 12) | 0x80);
            *stringBufferPostInc(sb) = (char)(((ch & 0x00000fc0) >>  6) | 0x80);
            *stringBufferPostInc(sb) = (char)(( ch & 0x0000003f) | 0x80);
            ix += 6;
        }
    }
    return sb;
}

int rprintf(const char *format, ...)
{
    int rtn;
    va_list args;
    va_start(args, format);
    rtn = vfprintf(stdout, format, args);
    va_end(args);
    if (0 != opt_dupfile)
    {
        va_start(args, format);
        vfprintf(opt_dupfile, format, args);
        va_end(args);
    }
    return rtn;
}
static long timediff(struct timeval *tfm, struct timeval *tto)
{
    return (tto->tv_sec * 1000000 + tto->tv_usec) - (tfm->tv_sec * 1000000 + tfm->tv_usec);
}
static int exec(const char *fileName)
{
    FILE *file, *ofil;
    char *buf, *cbuf, *doc, *cp, *outf;
    wchar_t *u8buf;
    long siz;
    struct stat statbuf;
    NpxElement *dsobj;
    void *exobj;
    StringBuffer *sb;
    clock_t sttime, entime;
    struct timeval sttv, entv;
    int rtn = 0, ix;
    unsigned len;
    struct rusage rsc;
    if (0 == (file = fopen(fileName, "rb")))
    {
        fprintf(stderr, "unable to open %s\n", fileName);
        rtn = -1;
        goto ex1;
    }
    dsobj = 0;
    fstat(fileno(file), &statbuf);
    siz = statbuf.st_size;
    buf = mAlloc((int) siz + 1, "execbuf");
    cbuf = mAlloc((int) siz + 1, "execbufc");
    if (siz != fread(buf, sizeof(char), siz, file))
    {
        fprintf(stderr, "error reading file %s (size %ld)\n", fileName, siz);
        rtn = -1;
        goto ex2;
    }
    buf[siz] = '\0';
    rprintf("read %ld byte file %s\n", siz, fileName);
    //rprintf("%s\n", buf);
    for (ix = 1; ix <= opt_repeat; ++ix)
    {
        memcpy(cbuf, buf, siz + 1);
        if (0 != opt_expat)
        {
            //sttime = clock();
            getrusage(0, &rsc);
            sttv = rsc.ru_utime;
            if (0 == (exobj = expatDeserialize(cbuf, siz)))
            {
                fprintf(stderr, "expat deserialize failed\n");
                rtn = -1;
                goto ex2;
            }
            //entime = clock();
            //rprintf("DOM build time %ld\n", getDOMTime());
            getrusage(0, &rsc);
            entv = rsc.ru_utime;
            rprintf("rusage CPU %ld\n", timediff(&sttv, &entv));
            delDocumentTree(exobj);
        }
        else
        {
            //sttime = clock();
            getrusage(0, &rsc);
            sttv = rsc.ru_utime;
            if (0 == (dsobj = deserialize(cbuf, siz)))
            {
                fprintf(stderr, "deserialize failed\n");
                rtn = -1;
                goto ex2;
            }
            //entime = clock();
            //rprintf("TAM build time %ld\n", getTAMTime());
            getrusage(0, &rsc);
            entv = rsc.ru_utime;
            rprintf("rusage CPU %ld\n", timediff(&sttv, &entv));
            if (ix != opt_repeat)
            {
                delNpxElement(dsobj);
                dsobj = 0;
            }
        }
        //rprintf("deserialize time %ld (clocks per sec %d)\n", entime - sttime, CLOCKS_PER_SEC);
    }
    if (opt_writexmlc != 0)
    {
        sb = newStringBuffer((StringBuffer *)0, 0);
        rtn = npx2XML(dsobj, sb, 1, (int)1);
        doc = stringBufferToString(sb, 0);
        if (opt_writexmlc != 0 && 0 != (cp = strstr(fileName, ".txt")) && 4 == strlen(cp))
        {
            siz = stringBufferGetOffset(sb) - 1;
            cp = stringCut(fileName, cp + 1);
            outf = stringConcat(cp, "xml.outc");
            delString(cp);
            if (0 == (ofil = fopen(outf, "wb")))
            {
                fprintf(stderr, "unable to open %s\n", outf);
                rtn = -1;
                goto ex3;
            }
            rprintf("writing %s\n", outf);
            if (siz != fwrite(doc, sizeof(char), siz, ofil))
            {
                fprintf(stderr, "error writing file %s (size %ld)\n", outf, siz);
                rtn = -1;
                goto ex4;
            }
            ex4:
            fclose(ofil);
            ex3:
            delString(outf);
        }
        if (1 == opt_reserialize)
            fprintf(stdout, "%s\n", doc);
        delString(doc);
        delStringBuffer(sb);
    }
    if (1 == opt_reserialize && 0 == opt_writexmlc)
    {
        doc = serialize(dsobj);
        fprintf(stdout, "%s\n", doc);
        delString(doc);
    }
ex2:
    if (0 != dsobj)
        delNpxElement(dsobj);
    mFree(buf, "execbufF");
    mFree(cbuf, "execbufc");
    fclose(file);
ex1:
    return rtn;
}
static int execDir(const char *dirName)
{
    DIR           *d;
    struct dirent *dir;
    char *cp, *path;
    int ix, found;
    d = opendir(dirName);
    if (0 != d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            found = 0;
            for (ix = 0; 0 != opt_suffixes[ix]; ++ix)
            {
                if (0 != (cp = strstr(dir->d_name, opt_suffixes[ix])) && strlen(opt_suffixes[ix]) == strlen(cp))
                {
                    found = 1;
                    break;
                }
            }
            if (0 == found)
                continue;
            rprintf("processing %s\n", dir->d_name);
            path = stringConcatV(dirName, separator, dir->d_name, (char *)0);
            exec(path);
            delString(path);
        }
        closedir(d);
    }
    else
    {
        fprintf(stderr, "cannot open directory %s\n", dirName);
        return -1;
    }
    return 0;
}
static void usage(const char *pgm)
{
    fprintf(stderr, "%s [-e] [-h] [-ofile] [-rn] [-v] [-w] [-x] path\n", pgm);
    fprintf(stderr, "  -e  deserialize input XML files using Expat\n");
    fprintf(stderr, "  -h  display this help information\n");
    fprintf(stderr, "  -ofile  duplicate stdout to this file\n");
    fprintf(stderr, "  -rn  repeat the deserialization n times\n");
    fprintf(stderr, "  -ssuffix  if path is a directory, input files have this suffix (default .txt)\n");
    fprintf(stderr, "            (may be repeated for a maximum of 9 suffixes)\n");
    fprintf(stderr, "  -v  display the version and exit\n");
    fprintf(stderr, "  -w  write deserialized output to .xml.outc file\n");
    fprintf(stderr, "  -x  write reserialized XML version to .xml.outc file\n");
    exit(1);
}
int main(int argc, const char *argv[])
{
    struct stat statbuf;
    int n, ni, ss, sl;
    if (setjmp(env))
    {
        exit(254);
    }
    ss = 0;
    ni = 1;
    for (n = 1; n < argc; n += ni)
    {
        if (argv[n][0] == '-')
        {
            switch (argv[n][1])
            {
                case 'e':
                    opt_expat = 1;
                    break;
                case 'h':
                    usage(argv[0]);
                    break;
                case 'o':
                    opt_dupfile = fopen(&argv[n][2], "wb");
                    break;
                case 'r':
                    opt_repeat = (int) strtol(&argv[n][2], (char **) 0, 10);
                    break;
                case 's':
                    if (ss < 9)
                    {
                        sl = strlen(&argv[n][2]);
                        opt_suffixes[ss] = mAlloc(sl + 1, "optSuf");
                        memcpy(opt_suffixes[ss], &argv[n][2], sl);
                        opt_suffixes[ss][sl] = '\0';
                        ++ss;
                    }
                    break;
                case 'v':
                    fprintf(stderr, "Version: %s\n", version);
                    exit(1);
                case 'w':
                    opt_writexmlc = 1;
                    break;
                case 'x':
                    opt_reserialize = 1;
                    break;
                default:
                    usage(argv[0]);
                    break;
            }
        }
    }
    if (0 == ss)
    {
        sl = strlen(&argv[n][2]);
        opt_suffixes[ss] = mAlloc(5, "optSuf");
        memcpy(opt_suffixes[ss], ".txt", 5);
        ++ss;
    }
    opt_suffixes[ss] = 0;
    for (n = 1; n < argc; n += ni)
    {
        if (argv[n][0] != '-')
        {
            if (0 <= lstat(argv[n], &statbuf))
            {
                if (0 != (S_IFDIR & statbuf.st_mode))
                {
                    execDir(argv[n]);
                }
                else if (0 != (S_IFREG & statbuf.st_mode))
                {
                    exec(argv[n]);
                }
            }
            else
            {
                fprintf(stderr, "cannot stat %s\n", argv[n]);
            }
        }
    }
    for (n = 0; 0 != opt_suffixes[n]; n +=1)
    {
        mFree(opt_suffixes[n], "optSuf");
    }
#ifdef MEMORY_TRAK
    trakReport(__FILE__);
#endif
    return 0;
}

