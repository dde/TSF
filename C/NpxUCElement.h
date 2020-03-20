#ifndef NpxUCElement_INCLUDE_GUARD
#define NpxUCElement_INCLUDE_GUARD
#ifdef NpxUCElement_EXPORT
#define NpxUCElement_API
#else
#define NpxUCElement_API extern
#endif
#define UCS_CHAR
#ifdef UCS_CHAR
typedef wchar_t Char;
#else
typedef char Char;
#endif
typedef char String;
typedef struct NpxUCElement NpxUCElement;
typedef struct NpxUCElementMtb
{
    void (*add)(NpxUCElement *, Char *str, int nfm, int ndx, int idx, char typ);
    void (*addElem)(NpxUCElement *, Char *nm, NpxUCElement *elm, char typ);
    void (*forEachContent)  (NpxUCElement *s, void *prm, void (*cb)(NpxUCElement *, int, Char *nm, void *obj, char typ, void *prm));
    int  (*isSLU)(NpxUCElement *, Char *tp);
    int  (*isPLU)(NpxUCElement *, Char *tp);
    Char *(*getObjName)(NpxUCElement *, void *obj);
    void *(*getObjByName)(NpxUCElement *, Char *nm);
    int  (*hasContent)(NpxUCElement *);
    int  (*getElementCount)(NpxUCElement *s);
    char (*getElementType)(NpxUCElement *s, int sub);
    Char *(*getElementName)(NpxUCElement *s, int sub);
    void *(*getElementValue)(NpxUCElement *s, int sub);
    NpxUCElement *(*getParent)(NpxUCElement *s);
    Char *(*getTransaction)(NpxUCElement *s);
} NpxUCElementMtb;
struct NpxUCElement
{
    NpxUCElementMtb *mtb;
};
NpxUCElement_API NpxUCElement *newNpxUCElement(Char *str, int esiz, NpxUCElement *par);
NpxUCElement_API void delNpxUCElement(NpxUCElement *);
#undef NpxUCElement_API
#endif
