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

#include "Memory.h"
#include "Sstring.h"
#include "StringBuffer.h"
#include "NpxElement.h"

extern NpxElement *deserialize(char *);
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

static void attributeCb(NpxElement *s, int ix, char *name, char *val, void *prm)
{
    StringBuffer *sb = (StringBuffer *)prm;
    stringBufferAppend(sb, " ", 0);
    stringBufferAppend(sb, name, 0);
    stringBufferAppend(sb, "=\"", 0);
    stringBufferAppend(sb, val, 0);
    stringBufferAppend(sb, "\"", 0);
}
static void contentCb(NpxElement *s, int ix, char *name, void *obj, void *prm)
{
    StringBuffer *sb = (StringBuffer *)prm;
    NpxElement *elm;
    if (s->mtb->isElement(s, name))
    {
        stringBufferAppend(sb, "<", 0);
        stringBufferAppend(sb, name, 0);
        elm = (NpxElement *)obj;
        if (elm->mtb->hasAttributes(elm))
            elm->mtb->forEachAttribute(elm, prm, attributeCb);
        if (elm->mtb->hasContent(elm))
        {
            stringBufferAppend(sb, ">", 0);
            elm->mtb->forEachContent(elm, prm, contentCb);
            stringBufferAppend(sb, "</", 0);
            stringBufferAppend(sb, name, 0);
            stringBufferAppend(sb, ">", 0);
        }
        else
        {
            stringBufferAppend(sb, "/>", 0);
        }
    }
    else if (s->mtb->isText(s, name))
    {
        stringBufferAppend(sb, (char *)obj, 0);
    }
    else if (s->mtb->isCdata(s, name))
    {
        stringBufferAppend(sb, "<![CDATA[", 0);
        stringBufferAppend(sb, (char *)obj, 0);
        stringBufferAppend(sb, "]]>", 0);
    }
    else if (s->mtb->isComment(s, name))
    {
        stringBufferAppend(sb, "<!--", 0);
        stringBufferAppend(sb, (char *)obj, 0);
        stringBufferAppend(sb, "-->", 0);
    }
    else if (s->mtb->isProcInst(s, name))
    {
        stringBufferAppend(sb, "<?", 0);
        stringBufferAppend(sb, (char *)obj, 0);
        stringBufferAppend(sb, "?>", 0);
    }
    else if (s->mtb->isDoctype(s, name))
    {
        stringBufferAppend(sb, "<!DOCTYPE ", 0);
        stringBufferAppend(sb, (char *)obj, 0);
        stringBufferAppend(sb, ">", 0);
    }
    else
    {
        stringBufferAppend(sb, "<!--UNKNOWN:", 0);
        stringBufferAppend(sb, (char *)obj, 0);
        stringBufferAppend(sb, "-->", 0);
    }
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
static int exec(const char *fileName)
{
    FILE *file, *ofil;
    char *buf, *cbuf, *doc, *cp, *outf;
    long siz;
    struct stat statbuf;
    NpxElement *dsobj;
    void *exobj;
    StringBuffer *sb;
    clock_t sttime, entime;
    int rtn = 0, ix;
    if (0 == (file = fopen(fileName, "rb")))
    {
        fprintf(stderr, "unable to open %s\n", fileName);
        rtn = -1;
        goto ex1;
    }
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
    cbuf = mAlloc((int) siz + 1, "execbufc");
    rprintf("read %ld byte file %s\n", siz, fileName);
    for (ix = 1; ix <= opt_repeat; ++ix)
    {
        memcpy(cbuf, buf, siz + 1);
        if (0 != opt_expat)
        {
            sttime = clock();
            if (0 == (exobj = expatDeserialize(cbuf, siz)))
            {
                fprintf(stderr, "deserialize failed\n");
                rtn = -1;
                goto ex2;
            }
            entime = clock();
            rprintf("DOM build time %ld\n", getDOMTime());
            delDocumentTree(exobj);
        }
        else
        {
            sttime = clock();
            if (0 == (dsobj = deserialize(cbuf)))
            {
                fprintf(stderr, "deserialize failed\n");
                rtn = -1;
                goto ex2;
            }
            entime = clock();
            rprintf("TAM build time %ld\n", getTAMTime());
            if (ix != opt_repeat)
                delNpxElement(dsobj);
        }
        rprintf("deserialize time %ld (clocks per sec %d)\n", entime - sttime, CLOCKS_PER_SEC);
    }
    if (opt_writexmlc != 0)
    {
        sb = newStringBuffer((StringBuffer *) 0, 0);
        dsobj->mtb->forEachContent(dsobj, sb, contentCb);
        *stringBufferPostInc(sb) = '\0';
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
        //fprintf(stdout, "%s\n", doc);
        delString(doc);
        delStringBuffer(sb);
    }
    if (1 == opt_reserialize)
    {
        doc = serialize(dsobj);
        fprintf(stdout, "%s\n", doc);
        delString(doc);
    }
ex2:
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
    fprintf(stderr, "%s [-ofile] [-rn] [-v] [-w] [-x]\n", pgm);
    fprintf(stderr, "  -ofile  duplicate stdout to this file\n");
    fprintf(stderr, "  -rn  repreat the deserialization n times\n");
    fprintf(stderr, "  -v  display the version and exit\n");
    fprintf(stderr, "  -w  write deserialized output to .xml.outc file\n");
    fprintf(stderr, "  -x  write XML version to .xml.outc file\n");
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
    if (0 == ss)
    {
        sl = strlen(&argv[n][2]);
        opt_suffixes[ss] = mAlloc(5, "optSuf");
        memcpy(opt_suffixes[ss], ".txt", 5);
        ++ss;
    }
    opt_suffixes[ss] = 0;
    return 0;
}

