#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "Memory.h"
/**
 * Import the API declarations.
*/
#define API_EXPORT
#include "NpxElement.h"
#undef API_EXPORT

typedef char String;
/*static const char ELEMENT[2] = "<";*/
static const char CDATA[2]   = "]";
static const char PI[2]      = "?";
static const char COMMENT[2] = "-";
static const char TEXT[2]    = "[";
/*static const char ATTRS[2]   = ">";*/
/*static const char ATTR[2]    = "=";*/
static const char DOCTYPE[2] = "!";
/*static const char *etypes[]  = {TEXT, CDATA, COMMENT, PI, DOCTYPE};*/
/*NpxDataType dt = new NpxDataType();*/

typedef struct NpxElement NpxElement;
static void addAttr(NpxElement *self, char *nm, char *val);
static void add(NpxElement *self, const char *ch, int ndx, int idx);
static void addElem(NpxElement *self, char *nm, NpxElement *elm);
static void forEachContent(NpxElement *s, void *, void (*cb)(NpxElement *, int, char *name, void *obj, void *prm));
static void forEachAttribute(NpxElement *s, void *, void (*cb)(NpxElement *, int, char *name, char *val, void *prm));
static int  isElement(NpxElement *s, char *obj);
static int  isText(NpxElement *s, char *obj);
static int  isCdata(NpxElement *s, char *obj);
static int  isComment(NpxElement *s, char *obj);
static int  isProcInst(NpxElement *s, char *obj);
static int  isDoctype(NpxElement *s, char *obj);
static char *getObjName(NpxElement *s, void *obj);
static void *getObjByName(NpxElement *s, char *nm);
static int  hasContent(NpxElement *s);
static int  hasAttributes(NpxElement *s);
static int  getElementCount(NpxElement *s);
static int  getAttributeCount(NpxElement *s);

/* method table */
static NpxElementMtb npxElementMtb = {addAttr, add, addElem, forEachContent, forEachAttribute,
    isElement, isText, isCdata, isComment, isProcInst, isDoctype, getObjName, getObjByName,
    hasContent, hasAttributes, getElementCount, getAttributeCount};

typedef struct NpxElementI
{
    NpxElementMtb  *mtb;
    char           *xact;
    char           **tagNames;
    void           **content;
    char           **attrNames;
    char           **attrVals;
    unsigned short elemCount;
    unsigned short elemSize;
    unsigned short attrCount;
    unsigned short attrSize;
} NpxElementI;
/**
 * Construct a new NpxElement object.
*/
NpxElement *newNpxElement(String *str, int esiz, int asiz)
{
    NpxElementI *self;
    unsigned siz;
    siz = sizeof(NpxElementI);
    siz += esiz * (sizeof(*self->tagNames) + sizeof(*self->content));
    siz += asiz * (sizeof(*self->attrNames) + sizeof(*self->attrVals));
    self = mAlloc(siz, "newNpxElement");
    self->mtb = &npxElementMtb;
    self->xact = str;
    self->elemSize = esiz;
    self->attrSize = asiz;
    self->elemCount = 0;
    self->attrCount = 0;
    self->tagNames = (char **)(self + 1);
    self->content = (void **)&self->tagNames[esiz];
    self->attrNames = (char **)&self->content[esiz];
    self->attrVals = (char **)&self->attrNames[asiz];
    return (NpxElement *)self;
}
void delNpxElement(NpxElement *npxElement)
{
  mFree((NpxElementI *)npxElement, "delNpxElement");
}
static void forEachContent(NpxElement *s, void *prm, void (*callback)(NpxElement *, int, char *name, void *obj, void *prm))
{
    int ix;
    NpxElementI *self = (NpxElementI *)s;
    for (ix = 0; ix < self->elemCount; ++ix)
    {
        callback(s, ix, self->tagNames[ix], self->content[ix], prm);
    }
}
static void forEachAttribute(NpxElement *s, void *prm, void (*callback)(NpxElement *, int, char *name, char *val, void *prm))
{
    int ix;
    NpxElementI *self = (NpxElementI *)s;
    for (ix = 0; ix < self->attrCount; ++ix)
    {
        callback(s, ix, self->attrNames[ix], self->attrVals[ix], prm);
    }
}
static int isElement(NpxElement *s, char *nm)
{
    return isalpha(*nm);
}
static int isText(NpxElement *s, char *nm)
{
    return TEXT[0] == *nm;
}
static int isCdata(NpxElement *s, char *nm)
{
    return CDATA[0] == *nm;
}
static int isComment(NpxElement *s, char *nm)
{
    return COMMENT[0] == *nm;
}
static int isProcInst(NpxElement *s, char *nm)
{
    return PI[0] == *nm;
}
static int isDoctype(NpxElement *s, char *nm)
{
    return DOCTYPE[0] == *nm;
}
static char *getObjName(NpxElement *s, void *obj)
{
    int ix;
    NpxElementI *self = (NpxElementI *)s;
    for (ix = 0; ix <self->elemCount; ++ix)
    {
        if (self->content[ix] == obj)
            return self->tagNames[ix];
    }
    return 0;
}
static void *getObjByName(NpxElement *s, char *nm)
{
    int ix;
    NpxElementI *self = (NpxElementI *)s;
    for (ix = 0; ix <self->elemCount; ++ix)
    {
        if (0 == strcmp(self->tagNames[ix], nm))
            return self->content[ix];
    }
    return 0;
}
static int getElementCount(NpxElement *s)
{
    NpxElementI *self = (NpxElementI *)s;
    return self->elemCount;
}
static int getAttributeCount(NpxElement *s)
{
    NpxElementI *self = (NpxElementI *)s;
    return self->attrCount;
}
static int hasContent(NpxElement *s)
{
    NpxElementI *self = (NpxElementI *)s;
    return 0 != self->elemCount;
}
static int hasAttributes(NpxElement *s)
{
    NpxElementI *self = (NpxElementI *)s;
    return 0 != self->attrCount;
}
static void add(NpxElement *s, const char *ch, int fm, int to)
{
    int sub;
    NpxElementI *self = (NpxElementI *)s;
    sub = self->elemCount++;
    self->tagNames[sub] = (char *)ch;
    self->content[sub] = (void *)&self->xact[fm];
}
static void addElem(NpxElement *s, char *nm, NpxElement *cntnt)
{
    int sub;
    NpxElementI *self = (NpxElementI *)s;
    sub = self->elemCount++;
    self->tagNames[sub] = nm;
    self->content[sub] = (void *)cntnt;
}
static void addAttr(NpxElement *s, char *nm, char *val)
{
    int sub;
    NpxElementI *self = (NpxElementI *)s;
    sub = self->attrCount++;
    self->attrNames[sub] = nm;
    self->attrVals[sub] = val;
}