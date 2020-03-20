#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Memory.h"
/**
 * Import the API declarations.
*/
#define API_EXPORT
#include "NpxElement.h"
#undef API_EXPORT
#include "NpxRoot.h"
#define arraySize(a) ((sizeof a)/(sizeof a[0]))

typedef char String;

static void add(NpxElement *self, const char *str, int nfm, int fm, int to, char typ);
static void addElem(NpxElement *self, char *nm, NpxElement *elm, char typ);
static void forEachContent(NpxElement *s, void *, void (*callback)(NpxElement *, int, char *name, void *obj, char typ, void *prm));
static int  isSLU(NpxElement *s, char *tp);
static int  isPLU(NpxElement *s, char *tp);
static char *getObjName(NpxElement *s, void *obj);
static void *getObjByName(NpxElement *s, char *nm);
static int  hasContent(NpxElement *s);
static int  getElementCount(NpxElement *s);
static char getElementType(NpxElement *s, int sub);
static char *getElementName(NpxElement *s, int sub);
static void *getElementValue(NpxElement *s, int sub);
static NpxElement *getParent(NpxElement *s);
static char *getTransaction(NpxElement *s);

/* method table */
static NpxElementMtb npxElementMtb = {add, addElem, forEachContent,
    isSLU, isPLU, getObjName, getObjByName,
    hasContent, getElementCount, getElementType, getElementName, getElementValue, getParent, getTransaction};

typedef struct NpxElementI
{
    NpxElementMtb  *mtb;
    char           *xact;
    NpxElement     *parent;
    char           **elemNames;
    void           **elemValues;
    unsigned short elemCount;
    unsigned short elemSize;
    char           elemTypes[8];
} NpxElementI;
/**
 * Construct a new NpxElement object.  The elemType array is sized in units of 8 chars.  If the passed esiz is
 * greater than 8, it requires additional storage allocated at the beginning of the variable section of the
 * object in units of 8 bytes. E.g. if esiz is 23, it requires 24 bytes or 2 additional units the size of elemTypes.
 * ((23 - 1) / 8) * 8) = 16 additional bytes
*/
NpxElement *newNpxElement(String *str, int esiz, NpxElement *par)
{
    NpxElementI *self;
    unsigned siz, szt;
    int ix;
    siz = sizeof(NpxElementI);
    siz += esiz * (sizeof(*self->elemNames) + sizeof(*self->elemValues));
    szt = sizeof(self->elemTypes);
    if (esiz > szt)
        szt = ((esiz - 1) / szt) * szt;
    else
        szt = 0;
    self = mAlloc(siz + szt, "newNpxElement");
    self->mtb = &npxElementMtb;
    self->parent = par;
    self->xact = str;
    self->elemSize = esiz;
    self->elemCount = 0;
    self->elemNames = (char **)((char *)(self + 1) + szt);
    for (ix = 0; ix < (int)(sizeof self->elemTypes) + szt; ++ix)
        self->elemTypes[ix] = '\0';
    self->elemValues = (void **)&self->elemNames[esiz];
    return (NpxElement *)self;
}
void delNpxElement(NpxElement *npxElement)
{
    int ix, ct;
    NpxElementI *self = (NpxElementI *)npxElement;
    ct = self->elemCount;
    for (ix = 0; ix < ct; ++ix)
    {
        if (0 != (ctype[self->elemTypes[ix]] & SL) && 0 != self->elemValues[ix])
            delNpxElement((NpxElement *)self->elemValues[ix]);
    }
  mFree((NpxElementI *)npxElement, "delNpxElement");
}
static void forEachContent(NpxElement *s, void *prm, void (*callback)(NpxElement *, int, char *name, void *obj, char typ, void *prm))
{
    int ix;
    NpxElementI *self = (NpxElementI *)s;
    for (ix = 0; ix < self->elemCount; ++ix)
    {
        callback(s, ix, self->elemNames[ix], self->elemValues[ix], self->elemTypes[ix], prm);
    }
}
static int isSLU(NpxElement *s, char *tp)
{
    char typ = *tp;
    if (typ < arraySize(ctype) && 0 != (typ & SL))
        return 1;
    return 0;
}
static int isPLU(NpxElement *s, char *tp)
{
    char typ = *tp;
    if (typ < arraySize(ctype) && 0 != (typ & PL))
        return 1;
    return 0;
}
static char *getObjName(NpxElement *s, void *obj)
{
    int ix;
    NpxElementI *self = (NpxElementI *)s;
    for (ix = 0; ix <self->elemCount; ++ix)
    {
        if (self->elemValues[ix] == obj)
            return self->elemNames[ix];
    }
    return 0;
}
static void *getObjByName(NpxElement *s, char *nm)
{
    int ix;
    NpxElementI *self = (NpxElementI *)s;
    for (ix = 0; ix <self->elemCount; ++ix)
    {
        if (0 == strcmp(self->elemNames[ix], nm))
            return self->elemValues[ix];
    }
    return 0;
}
static int getElementCount(NpxElement *s)
{
    NpxElementI *self = (NpxElementI *)s;
    return self->elemCount;
}
static char getElementType(NpxElement *s, int sub)
{
    NpxElementI *self = (NpxElementI *)s;
    return self->elemTypes[sub];
}
static char *getElementName(NpxElement *s, int sub)
{
    NpxElementI *self = (NpxElementI *)s;
    return self->elemNames[sub];
}
static void *getElementValue(NpxElement *s, int sub)
{
    NpxElementI *self = (NpxElementI *)s;
    return self->elemValues[sub];
}
static NpxElement *getParent(NpxElement *s)
{
    NpxElementI *self = (NpxElementI *)s;
    return self->parent;
}
static char *getTransaction(NpxElement *s)
{
    NpxElementI *self = (NpxElementI *)s;
    return self->xact;
}
static int hasContent(NpxElement *s)
{
    NpxElementI *self = (NpxElementI *)s;
    return 0 != self->elemCount;
}
static void add(NpxElement *s, const char *str, int nfm, int fm, int to, char typ)
{
    int sub;
    NpxElementI *self = (NpxElementI *)s;
    sub = self->elemCount++;
    self->elemNames[sub] = (nfm == fm) ? (char *)0 : (char *)&str[nfm];
    self->elemValues[sub] = (char *)&str[fm + 1];
    self->elemTypes[sub] = typ;
}
static void addElem(NpxElement *s, char *nm, NpxElement *cntnt, char typ)
{
    int sub;
    NpxElementI *self = (NpxElementI *)s;
    sub = self->elemCount++;
    self->elemNames[sub] = nm;
    self->elemValues[sub] = (void *)cntnt;
    self->elemTypes[sub] = typ;
}
