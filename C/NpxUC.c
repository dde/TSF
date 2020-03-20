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
#include <wchar.h>
#include <setjmp.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/resource.h>
/* #define MEMORY_TRAK */
#include "Memory.h"
#include "Sstring.h"
#include "StringBuffer.h"
#include "UCStringBuffer.h"
#include "NpxElement.h"
#include "NpxUCElement.h"

#define self(s) (*s->mtb)

extern int npx2XML(NpxElement *elm, StringBuffer *sb, int nopts, ...);
extern int npx2XMLUC(NpxUCElement *elm, UCStringBuffer *sb, int nopts, ...);

/* extern NpxElement *deserialize(unsigned char *, int len); */
extern NpxUCElement *deserializeUC(unsigned char *, int len);
extern void *expatDeserialize(unsigned char *buf, int len);
extern char *serialize(NpxElement *);
extern void delDocumentTree(void *root);
extern clock_t getDOMTime();
extern clock_t getTAMTime();


typedef char String;
static char *version = "2.0";
static char *separator = "/";
static jmp_buf env;
static int opt_writexmlc = 0;
static int opt_reserialize = 0;
static int opt_repeat = 1;
static FILE *opt_dupfile = 0;
static int opt_expat = 0;
static char *opt_suffixes[10];

static UCStringBuffer *utf8toUCS(const unsigned char *str, int ln)
{
    int ix;
    unsigned char ch;
    UCStringBuffer *wbuf;
    wbuf = newUCStringBuffer((UCStringBuffer *) 0, 0);
    ix = 0;
    while (ix < ln)
    {
        ch = str[ix];
        if (ch < 0x80)
        {
            *ucstringBufferPostInc(wbuf) = (wchar_t) ch;
            ix += 1;
        }
        else if (ch >= 0xc0)
        {
            if (ch < 0xe0)
            {
                *ucstringBufferPostInc(wbuf) = (wchar_t) ((ch & 0x1fu) << 6 | (str[ix + 1] & 0x3fu));
                ix += 2;
            }
            else if (ch < 0xf0)
            {
                *ucstringBufferPostInc(wbuf) = (wchar_t) (((ch & 0x0fu) << 12) | ((str[ix + 1] & 0x3fu) << 6) |
                                                          (str[ix + 2] & 0x3f));
                ix += 3;
            }
            else if (ch < 0xf8)
            {
                *ucstringBufferPostInc(wbuf) = (wchar_t) (((ch & 0x07u) << 18) | ((str[ix + 1] & 0x3fu) << 12) |
                                                          ((str[ix + 2] & 0x3fu) << 6) | (str[ix + 3] & 0x3fu));
                ix += 4;
            }
            else if (ch < 0xfc)
            {
                *ucstringBufferPostInc(wbuf) = (wchar_t) (((ch & 0x03u) << 24) | ((str[ix + 1] & 0x3fu) << 18) |
                                                          ((str[ix + 2] & 0x3fu) << 12) | ((str[ix + 3] & 0x3fu) << 6) |
                                                          (str[ix + 4] & 0x3fu));
                ix += 5;
            }
            else if (ch < 0xfe)
            {
                *ucstringBufferPostInc(wbuf) = (wchar_t) (((ch & 0x01u) << 30) | ((str[ix + 1] & 0x3fu) << 24) |
                                                          ((str[ix + 2] & 0x3fu) << 18) | ((str[ix + 3] & 0x3fu) << 12) |
                                                          ((str[ix + 4] & 0x3fu) <<  6) |  (str[ix + 4] & 0x3fu));
                ix += 6;
            }
            else
            {
                /* encoding error */
                delUCStringBuffer(wbuf);
                return 0;
            }
        }
        else
        {
            /* encoding error */
            delUCStringBuffer(wbuf);
            return 0;
        }
    }
    return wbuf;
}
static int wchar2Utf8(wchar_t ch, StringBuffer *sb)
{
    int ix = 0;
    if (ch < 0x0080)
    {
        *stringBufferPostInc(sb) = (char)ch;
        return 1;
    }
    if (ch < 0x0800)
    {
        *stringBufferPostInc(sb) = (char)(((ch & 0x07c0u) >> 6) |  0xc0u);
        ix = 2;
        goto l0;
    }
    else if (ch < 0x10000)
    {
        *stringBufferPostInc(sb) = (char)(((ch & 0xf000) >> 12) |  0xe0);
        ix = 3;
        goto l6;
    }
    else if (ch < 0x200000)
    {
        *stringBufferPostInc(sb) = (char)(((ch & 0x1c0000) >> 18) | 0xf0);
        ix = 4;
        goto l12;
    }
    else if (ch < 0x4000000)
    {
        *stringBufferPostInc(sb) = (char)(((ch & 0x3000000) >> 24) | 0xf80);
        ix = 5;
        goto l18;
    }
    else
    {
        *stringBufferPostInc(sb) = (char)(((ch & 0x40000000) >> 30) | 0xfc0);
        ix = 6;
        goto l24;
    }
  l24:
    *stringBufferPostInc(sb) = (char)(((ch & 0x3f000000) >> 24) | 0x80);
  l18:
    *stringBufferPostInc(sb) = (char)(((ch & 0x00fc0000) >> 18) | 0x80);
  l12:
    *stringBufferPostInc(sb) = (char)(((ch & 0x0003f000) >> 12) | 0x80);
  l6:
    *stringBufferPostInc(sb) = (char)(((ch & 0x00000fc0) >>  6) | 0x80);
  l0:
    *stringBufferPostInc(sb) = (char)(( ch & 0x0000003f) | 0x80);
    return ix;
}
static StringBuffer *ucsToUtf8(wchar_t *wbuf, StringBuffer *sb)
{
    int ix;
    for (ix = 0; L'\0' != wbuf[ix]; ++ix)
    {
        wchar2Utf8(wbuf[ix], sb);
    }
    return sb;
}
static StringBuffer *ucsToUtf8L(wchar_t *wbuf, int len, StringBuffer *sb)
{
    int ix;
    for (ix = 0; ix < len; ++ix)
    {
        wchar2Utf8(wbuf[ix], sb);
    }
    return sb;
}
static int rprintf(const char *format, ...)
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
static int rwprintf(const wchar_t *format, ...)
{
    int rtn;
    va_list args;
    va_start(args, format);
    rtn = vfwprintf(stdout, format, args);
    va_end(args);
    if (0 != opt_dupfile)
    {
        va_start(args, format);
        vfwprintf(opt_dupfile, format, args);
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
    unsigned char *buf;
    char *cp, *outf;
    wchar_t *cbuf, *wdoc;
    long siz;
    struct stat statbuf;
    NpxUCElement *dsobj;
    void *exobj;
    UCStringBuffer *ucsb;
    clock_t sttime = 0, entime = 0;
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
    if (siz != fread(buf, sizeof(char), siz, file))
    {
        fprintf(stderr, "error reading file %s (size %ld)\n", fileName, siz);
        rtn = -1;
        goto ex2;
    }
    buf[siz] = '\0';
    rprintf("read %ld byte file %s\n", siz, fileName);

    for (ix = 1; ix <= opt_repeat; ++ix)
    {
        if (0 != opt_expat)
        {
            getrusage(0, &rsc);
            sttv = rsc.ru_utime;
            //sttime = clock();
            if (0 == (exobj = expatDeserialize(buf, (int)siz)))
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
            getrusage(0, &rsc);
            sttv = rsc.ru_utime;
            //sttime = clock();
            if (0 == (dsobj = deserializeUC(buf, (int)siz)))
            {
                fprintf(stderr, "deserializeUC failed\n");
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
                cbuf = self(dsobj).getTransaction(dsobj);
                mFree(cbuf, "wcxact");
                delNpxUCElement(dsobj);
                dsobj = 0;
            }
        }
        rprintf("deserializeUC time %ld (clocks per sec %d)\n", entime - sttime, CLOCKS_PER_SEC);
    }
    if (opt_writexmlc != 0)
    {
        ucsb = newUCStringBuffer((UCStringBuffer *)0, 0);
        rtn = npx2XMLUC(dsobj, ucsb, 1, (int)1);
        wdoc = ucstringBufferToString(ucsb, 0);
        if (opt_writexmlc != 0 && 0 != (cp = strstr(fileName, ".txt")) && 4 == strlen(cp))
        {
            siz = ucstringBufferGetOffset(ucsb) - 1;
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
            if (siz != fwrite(wdoc, sizeof(char), siz, ofil))
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
            fwprintf(stdout, L"%ls\n", wdoc);
        mFree(wdoc, "ucsBuf2string");
        delUCStringBuffer(ucsb);
    }
/*    if (1 == opt_reserialize && 0 == opt_writexmlc)
    {
        doc = serialize(dsobj);
        fprintf(stdout, "%s\n", doc);
        delString(doc);
    }*/
ex2:
    if (0 != dsobj)
    {
        cbuf = self(dsobj).getTransaction(dsobj);
        mFree(cbuf, "wcxact2");
        delNpxUCElement(dsobj);
    }
    mFree(buf, "execbufF");
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
                        sl = (int)strlen(&argv[n][2]);
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
        opt_suffixes[ss] = mAlloc(5, "optSuf");
        strcpy(opt_suffixes[ss], ".txt");
        ++ss;
    }
    opt_suffixes[ss] = 0;
    for (n = 1; n < argc; n += ni)
    {
        if (argv[n][0] != '-')
        {
            if (0 <= lstat(argv[n], &statbuf))
            {
                if (0 != ((unsigned)S_IFDIR & statbuf.st_mode))
                {
                    execDir(argv[n]);
                }
                else if (0 != ((unsigned)S_IFREG & statbuf.st_mode))
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

