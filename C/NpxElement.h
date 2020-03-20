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
    void (*add)(NpxElement *, const char *str, int nfm, int ndx, int idx, char typ);
    void (*addElem)(NpxElement *, char *nm, NpxElement *elm, char typ);
    void (*forEachContent)  (NpxElement *s, void *prm, void (*cb)(NpxElement *, int, char *nm, void *obj, char typ, void *prm));
    int  (*isSLU)(NpxElement *, char *tp);
    int  (*isPLU)(NpxElement *, char *tp);
    char *(*getObjName)(NpxElement *, void *obj);
    void *(*getObjByName)(NpxElement *, char *nm);
    int  (*hasContent)(NpxElement *);
    int  (*getElementCount)(NpxElement *s);
    char (*getElementType)(NpxElement *s, int sub);
    char *(*getElementName)(NpxElement *s, int sub);
    void *(*getElementValue)(NpxElement *s, int sub);
    NpxElement *(*getParent)(NpxElement *s);
    char *(*getTransaction)(NpxElement *s);
} NpxElementMtb;
struct NpxElement
{
    NpxElementMtb *mtb;
};
NPXELEMENT_API NpxElement *newNpxElement(String *str, int esiz, NpxElement *par);
NPXELEMENT_API void delNpxElement(NpxElement *);
#undef NPXELEMENT_API
#endif
