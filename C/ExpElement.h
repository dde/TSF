//
// Created by Dan Evans on 2019-01-07.
//

#ifndef NPX_EXPELEMENT_H
#define NPX_EXPELEMENT_H

typedef struct Attr Attr;
typedef struct NamedNodeMap NamedNodeMap;
typedef struct ExpElement ExpElement;
typedef struct ExpElementMtb
{
    ExpElement *(*elementAppendChild)(ExpElement *self, ExpElement *child);
    ExpElement *(*elementAddAttribute)(ExpElement *self, Attr *attr);
    ExpElement *(*elementSetNamedNodeMap)(ExpElement *self, NamedNodeMap *map);
    void (*vdeExpElement)(ExpElement *self);
} ExpElementMtb;
struct ExpElement
{
    ExpElementMtb *mtb;
};
extern ExpElement *newExpElement(ExpElement *, char *);
extern void delExpElement(ExpElement *);

#endif //NPX_EXPELEMENT_H
