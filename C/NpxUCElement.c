#include <wchar.h>

#include "Memory.h"
/**
 * Import the API declarations.
*/
#define API_EXPORT
#include "NpxUCElement.h"
#undef API_EXPORT
#include "NpxRoot.h"
#define arraySize(a) ((sizeof a)/(sizeof a[0]))

typedef char String;

static void add(NpxUCElement *self, Char *str, int nfm, int fm, int to, char typ);
static void addElem(NpxUCElement *self, Char *nm, NpxUCElement *cntnt, char typ);
static void forEachContent(NpxUCElement *s, void *, void (*callback)(NpxUCElement *, int, Char *name, void *obj, char typ, void *prm));
static int  isSLU(NpxUCElement *s, Char *tp);
static int  isPLU(NpxUCElement *s, Char *tp);
static Char *getObjName(NpxUCElement *s, void *obj);
static void *getObjByName(NpxUCElement *s, Char *nm);
static int  hasContent(NpxUCElement *s);
static int  getElementCount(NpxUCElement *s);
static char getElementType(NpxUCElement *s, int sub);
static Char *getElementName(NpxUCElement *s, int sub);
static void *getElementValue(NpxUCElement *s, int sub);
static NpxUCElement *getParent(NpxUCElement *s);
static Char *getTransaction(NpxUCElement *s);

/* method table */
static NpxUCElementMtb npxUCElementMtb = {add, addElem, forEachContent,
    isSLU, isPLU, getObjName, getObjByName,
    hasContent, getElementCount, getElementType, getElementName, getElementValue, getParent, getTransaction};

typedef struct NpxUCElementI
{
    NpxUCElementMtb *mtb;
    Char            *xact;
    NpxUCElement    *parent;
    Char            **elemNames;
    void            **elemValues;
    unsigned short  elemCount;
    unsigned short  elemSize;
    char            elemTypes[8];
} NpxUCElementI;
/**
 * Construct a new NpxUCElement object.  The elemType array is sized in units of 8 chars.  If the passed esiz is
 * greater than 8, it requires additional storage allocated at the beginning of the variable section of the
 * object in units of 8 bytes. E.g. if esiz is 23, it requires 24 bytes or 2 additional units the size of elemTypes.
 * ((23 - 1) / 8) * 8) = 16 additional bytes
*/
NpxUCElement *newNpxUCElement(Char *str, int esiz, NpxUCElement *par)
{
    NpxUCElementI *self;
    unsigned siz, szt;
    int ix;
    siz = sizeof(NpxUCElementI);
    siz += esiz * (sizeof(*self->elemNames) + sizeof(*self->elemValues));
    szt = sizeof(self->elemTypes);
    if (esiz > szt)
        szt = ((esiz - 1) / szt) * szt;
    else
        szt = 0;
    self = mAlloc(siz + szt, "newNpxUCElement");
    self->mtb = &npxUCElementMtb;
    self->parent = par;
    self->xact = str;
    self->elemSize = esiz;
    self->elemCount = 0;
    self->elemNames = (Char **)((char *)(self + 1) + szt);
    for (ix = 0; ix < (int)(sizeof self->elemTypes + szt); ++ix)
        self->elemTypes[ix] = '\0';
    self->elemValues = (void **)&self->elemNames[esiz];
    return (NpxUCElement *)self;
}
void delNpxUCElement(NpxUCElement *npxUCElement)
{
    int ix, ct;
    NpxUCElementI *self = (NpxUCElementI *)npxUCElement;
    ct = self->elemCount;
    for (ix = 0; ix < ct; ++ix)
    {
        if (0 != (ctype[self->elemTypes[ix]] & SL) && 0 != self->elemValues[ix])
            delNpxUCElement((NpxUCElement *)self->elemValues[ix]);
    }
    mFree((NpxUCElementI *)npxUCElement, "delNpxUCElementI");
}
static void forEachContent(NpxUCElement *s, void *prm, void (*callback)(NpxUCElement *, int, Char *name, void *obj, char typ, void *prm))
{
    int ix;
    NpxUCElementI *self = (NpxUCElementI *)s;
    for (ix = 0; ix < self->elemCount; ++ix)
    {
        callback(s, ix, self->elemNames[ix], self->elemValues[ix], self->elemTypes[ix], prm);
    }
}
static int isSLU(NpxUCElement *s, Char *tp)
{
    Char typ = *tp;
    if (typ < arraySize(ctype) && 0 != (typ & SL))
        return 1;
    return 0;
}
static int isPLU(NpxUCElement *s, Char *tp)
{
    unsigned char typ = *tp;
    if (typ < arraySize(ctype) && 0 != (typ & PL))
        return 1;
    return 0;
}
static Char *getObjName(NpxUCElement *s, void *obj)
{
    int ix;
    NpxUCElementI *self = (NpxUCElementI *)s;
    for (ix = 0; ix <self->elemCount; ++ix)
    {
        if (self->elemValues[ix] == obj)
            return self->elemNames[ix];
    }
    return 0;
}
static void *getObjByName(NpxUCElement *s, Char *nm)
{
    int ix;
    NpxUCElementI *self = (NpxUCElementI *)s;
    for (ix = 0; ix <self->elemCount; ++ix)
    {
        if (0 == wcscmp(self->elemNames[ix], nm))
            return self->elemValues[ix];
    }
    return 0;
}
static int getElementCount(NpxUCElement *s)
{
    NpxUCElementI *self = (NpxUCElementI *)s;
    return self->elemCount;
}
static char getElementType(NpxUCElement *s, int sub)
{
    NpxUCElementI *self = (NpxUCElementI *)s;
    return self->elemTypes[sub];
}
static Char *getElementName(NpxUCElement *s, int sub)
{
    NpxUCElementI *self = (NpxUCElementI *)s;
    return self->elemNames[sub];
}
static void *getElementValue(NpxUCElement *s, int sub)
{
    NpxUCElementI *self = (NpxUCElementI *)s;
    return self->elemValues[sub];
}
static NpxUCElement *getParent(NpxUCElement *s)
{
    NpxUCElementI *self = (NpxUCElementI *)s;
    return self->parent;
}
static Char *getTransaction(NpxUCElement *s)
{
    NpxUCElementI *self = (NpxUCElementI *)s;
    return self->xact;
}
static int hasContent(NpxUCElement *s)
{
    NpxUCElementI *self = (NpxUCElementI *)s;
    return 0 != self->elemCount;
}
static void add(NpxUCElement *s, Char *str, int nfm, int fm, int to, char typ)
{
    int sub;
    NpxUCElementI *self = (NpxUCElementI *)s;
    sub = self->elemCount++;
    self->elemNames[sub] = (nfm == fm) ? (Char *)0 : (Char *)&str[nfm];
    self->elemValues[sub] = (void *)&str[fm + 1];
    self->elemTypes[sub] = typ;
}
static void addElem(NpxUCElement *s, Char *nm, NpxUCElement *cntnt, char typ)
{
    int sub;
    NpxUCElementI *self = (NpxUCElementI *)s;
    sub = self->elemCount++;
    self->elemNames[sub] = nm;
    self->elemValues[sub] = (void *)cntnt;
    self->elemTypes[sub] = typ;
}
