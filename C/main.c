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
#include <sys/stat.h>

#include "Memory.h"
#include "Sstring.h"
#include "StringBuffer.h"
#include "NpxElement.h"

extern NpxElement *deserialize(char *);
extern char *serialize(NpxElement *);


typedef char String;
static char *version = "1.0";
static char *separator = "/";
static jmp_buf env;

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
static int exec(const char *fileName)
{
    FILE *file, *ofil;
    char *buf, *doc, *cp, *outf;
    long siz;
    struct stat statbuf;
    NpxElement *dsobj;
    StringBuffer *sb;
    int rtn = 0;
    if (0 == (file = fopen(fileName, "rb")))
    {
        fprintf(stderr, "unable to open %s\n", fileName);
        rtn = -1;
        goto ex1;
    }
    fstat(fileno(file), &statbuf);
    siz = statbuf.st_size;
    buf = mAlloc((int)siz + 1, "execbuf");
    if (siz != fread(buf, sizeof(char), siz, file))
    {
        fprintf(stderr, "error reading file %s (size %ld)\n", fileName, siz);
        rtn = -1;
        goto ex2;
    }
    buf[siz] = '\0';
    fprintf(stderr, "read %ld byte file %s\n", siz, fileName);
    if (0 == (dsobj = deserialize(buf)))
    {
        fprintf(stderr, "deserialize failed\n");
        rtn = -1;
        goto ex2;
    }
    sb = newStringBuffer((StringBuffer *)0, 0);
    dsobj->mtb->forEachContent(dsobj, sb, contentCb);
    *stringBufferPostInc(sb) = '\0';
    doc = stringBufferToString(sb, 0);
    if (0 != (cp = strstr(fileName, ".txt")) && 4 == strlen(cp))
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
        fprintf(stdout, "writing %s\n", outf);
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
    doc = serialize(dsobj);
    fprintf(stdout, "%s\n", doc);
    delString(doc);
    delStringBuffer(sb);
ex2:
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
    int ln;
    d = opendir(dirName);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            ln = (int)strlen(dir->d_name);
            if (0 != (cp = strstr(dir->d_name, ".txt")) && 4 == strlen(cp))
            {
                fprintf(stdout, "processing %s\n", dir->d_name);
                path = stringConcatV(dirName, separator, dir->d_name, (char *)0);
                exec(path);
                delString(path);
            }
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
    fprintf(stderr, "%s\n", pgm);
    exit(1);
}
int main(int argc, const char *argv[])
{
    struct stat statbuf;
    int n, ni;
    if (setjmp(env))
    {
        exit(254);
    }
    ni = 1;
    for (n = 1; n < argc; n += ni)
    {
        if (argv[n][0] == '-')
        {
            switch (argv[n][1])
            {
                case 'h':
                    usage(argv[0]);
                    break;
                case 'v':
                    fprintf(stderr, "Version: %s\n", version);
                    exit(1);
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
    return 0;
}

