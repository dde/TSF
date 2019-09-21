#ifndef NPXELEMENT_INCLUDE_GUARD
#define NPXELEMENT_INCLUDE_GUARD
#ifdef NPXELEMENT_EXPORT
#define NPXELEMENT_API
#else
#define NPXELEMENT_API extern
#endif
typedef char String;
typedef struct NpxElement NpxElement;
typedef struct NpxElementMtb
{
    void (*addAttr)(NpxElement *self, char *nm, char *val);
    void (*add)(NpxElement *, const char *ch, int ndx, int idx);
    void (*addElem)(NpxElement *, char *nm, NpxElement *elm);
    void (*forEachContent)  (NpxElement *s, void *prm, void (*cb)(NpxElement *, int, char *nm, void *obj, void *prm));
    void (*forEachAttribute)(NpxElement *s, void *prm, void (*cb)(NpxElement *, int, char *nm, char *val, void *prm));
    int  (*isElement)(NpxElement *, char *obj);
    int  (*isText)(NpxElement *, char *obj);
    int  (*isCdata)(NpxElement *, char *obj);
    int  (*isComment)(NpxElement *, char *obj);
    int  (*isProcInst)(NpxElement *, char *obj);
    int  (*isDoctype)(NpxElement *, char *obj);
    char *(*getObjName)(NpxElement *, void *obj);
    void *(*getObjByName)(NpxElement *, char *nm);
    int  (*hasContent)(NpxElement *);
    int  (*hasAttributes)(NpxElement *);
    int  (*getElementCount)(NpxElement *s);
    int  (*getAttributeCount)(NpxElement *s);
} NpxElementMtb;
struct NpxElement
{
    NpxElementMtb *mtb;
};
NPXELEMENT_API NpxElement *newNpxElement(String *str, int esiz, int asiz);
NPXELEMENT_API void delNpxElement(NpxElement *);
#undef NPXELEMENT_API
#endif
