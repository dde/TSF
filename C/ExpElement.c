//
// Created by Dan Evans on 2019-01-07.
//
#include <string.h>
#include "Memory.h"
#include "ExpElement.h"
static ExpElement *elementAppendChild(ExpElement *self, ExpElement *child);
static ExpElement *elementAddAttribute(ExpElement *self, Attr *attr);
static ExpElement *elementSetNamedNodeMap(ExpElement *self, NamedNodeMap *map);
static void vdeExpElement(ExpElement *self);

static ExpElementMtb methodTable = {elementAppendChild, elementAddAttribute, elementSetNamedNodeMap, vdeExpElement};

static const char SEP = '\xff';
static const char CLN = ':';

typedef struct ExpElementI
{
    ExpElementMtb *mtb;
    void (*destroy)(void *);
    char *localName;
    char *qName;
    char *nsURI;
    struct ExpElement *parentNode;
    struct ExpElement *firstChild;
    struct ExpElement *lastChild;
    struct ExpElement *nextSibling;
    struct ExpElement *prevSibling;
    NamedNodeMap *attrs;
} ExpElementI;
static void doTriple(char *triple, char** uri, char **localName, char **prefix)
{
    char *cp;
    char *ptrs[3];
    int ix, ln, ln2;
    ix = 0;
    ptrs[ix] = triple;
    while ('\0' != *cp)
    {
        if (*cp == SEP)
        {
            ptrs[++ix] = cp + 1;
        }
        ++cp;
    }
    if (0 == ix)  // no separator, only local name
    {
        ln = cp - ptrs[0] + 1;
        *localName = (char *)mAlloc(ln, "lclName");
        memcpy(*localName, ptrs[0], ln);  // also copies trailing 0
        *uri = 0;
        *prefix = 0;
    }
    else if (1 == ix)  // uri separator local name
    {
        ln2 = cp - ptrs[1] + 1;
        *localName = (char *)mAlloc(ln2, "lclName");
        memcpy(*localName, ptrs[0], ln2);  // also copies trailing 0
        ln2 = ptrs[1] - ptrs[0];
        *uri = (char *)mAlloc(ln2 + 1, "nsURI");
        memcpy(*uri, ptrs[0], ln2);
        (*uri)[ln2] = '\0';
        *prefix = 0;
    }
    else if (2 == ix) // uri separator local name separator prefix
    {
        ln = cp - ptrs[1] + 1;
        *prefix = (char *)mAlloc(ln, "qName");
        ln2 = cp - ptrs[2];
        memcpy(*prefix, ptrs[2], ln2);
        (*prefix)[ln2] = CLN;
        *localName = &(*prefix)[ln2 + 1];
        ln2 = ptrs[2] - ptrs[1] + 1;
        memcpy(*localName, ptrs[1], ln2);  // also copies trailing 0
        (*localName)[ln2] = '\0';
        ln2 = ptrs[1] - ptrs[0];
        *uri = (char *)mAlloc(ln2 + 1, "nsURI");
        memcpy(*uri, ptrs[0], ln2);
        (*uri)[ln2] = '\0';
    }
    else
    {
        ; // error
    }

}
ExpElement *newExpElementNS(ExpElement *selfx, char *nsTriple)
{
    int ln;
    ExpElementI *self = (ExpElementI *)selfx;
    if (0 == self)
        self = (ExpElement *)mAlloc(sizeof(ExpElement), "ExpElement");
    self->mtb = &methodTable;
    doTriple(nsTriple, &self->nsURI, &self->localName, &self->qName);
    self->parentNode = 0;
    self->firstChild = 0;
    self->lastChild = 0;
    self->nextSibling = 0;
    self->prevSibling = 0;
    self->attrs = 0;
    return self;
}
ExpElement *newExpElement(ExpElement *selfx, char *localName)
{
    int ln;
    ExpElementI *self = (ExpElementI *)selfx;
    if (0 == self)
        self = (ExpElement *)mAlloc(sizeof(ExpElement), "ExpElement");
    ln = strlen(localName) + 1;
    self->localName = (char *)mAlloc(ln, "ExpElement");
    memcpy(self->localName, localName, ln);
    self->parentNode = 0;
    self->firstChild = 0;
    self->lastChild = 0;
    self->nextSibling = 0;
    self->prevSibling = 0;
    self->attrs = 0;
    self->nsURI = 0;
    self->qName = 0;
    return self;
}

static ExpElement *elementAppendChild(ExpElement *selfx, ExpElement *child)
{
    ExpElementI *self = (ExpElementI *)selfx;
    if (0 == self->firstChild)
    {
        self->firstChild = self->lastChild = child;
    } else
    {

    }
    return self;
}
static ExpElement *elementAddAttribute(ExpElement *selfx, Attr *attr)
{
    ExpElementI *self = (ExpElementI *)selfx;
    if (0 == self->attrs)
        self->attrs = newNamedNodeMap();
    namedNodeMapAddAttr(self->attrs, attr);
    return self;
}
static ExpElement *elementSetNamedNodeMap(ExpElement *self, NamedNodeMap *map)
{
    self->attrs = map;
    return self;
}
static void vdeExpElement(ExpElement *selfx)
{
    ExpElementI *self = (ExpElementI *)selfx;
    if (0 != self->qName)
        mFree(self->qName, "delElemqNm");
    else
        mFree(self->localName, "delElemlNm");
    if (0 != self->nsURI)
        mFree(self->nsURI, "delElemnsU");
    if (0 != self->attrs)
        delNamedNodeMap(self->attrs);
}
void delExpElement(ExpElement *selfx)
{
    ExpElementI *self = (ExpElementI *)selfx;
    vdeExpElement(self);
    mFree(self, "delExpElement");
}
void delExpElementTree(ExpElement *selfx)
{
    ExpElementI *self = (ExpElementI *)selfx;
    ExpElementI *seq, *nxt;
    seq = self->firstChild;
    while (0 != seq)
    {
        nxt = seq->nextSibling;
        seq->destroy(seq);
        seq = nxt;
    }
    delExpElement(self);
}