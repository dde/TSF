//
// Created by Dan Evans on 2019-01-07.
//
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include "Memory.h"
#include "Sstring.h"
#include "StringBuffer.h"

#include <expat.h>

#ifdef XML_LARGE_SIZE
# if defined(XML_USE_MSC_EXTENSIONS) && _MSC_VER < 1400
#  define XML_FMT_INT_MOD "I64"
# else
#  define XML_FMT_INT_MOD "ll"
# endif
#else
# define XML_FMT_INT_MOD "l"
#endif

#ifdef XML_UNICODE_WCHAR_T
# define XML_FMT_STR "ls"
#else
# define XML_FMT_STR "s"
#endif

//#define trace(n) fprintf(stderr, "trace %d", n)
#define trace(n)
static const int ELEMENT_NODE = 1;
static const int ATTRIBUTE_NODE = 2;
static const int TEXT_NODE = 3;
static const int CDATA_SECTION_NODE = 4;
static const int ENTITY_REFERENCE_NODE = 5;
static const int DOCTYPE_NODE = 6;
static const int PROCESSING_INSTRUCTION_NODE = 7;
static const int COMMENT_NODE = 8;
static const int DOCUMENT_NODE = 9;
struct ExpElement;
static const unsigned char SEP = '\xff';
static const char CLN = ':';
static const char *PCDATA_NAME = "#text";
static const char *CDATA_NAME = "#cdata";
static const char *PI_NAME = "#pi";
static const char *COMMENT_NAME = "#comment";
static const char *DOCTYPE_NAME = "#doctype";
static const char *DOC_NAME = "#document";
static char *ENC = "UTF-8";
static struct ExpElement *document;
static struct ExpElement *current;
static int Depth;
static int textType;
static int ms_elementBuild = 0;

typedef struct node
{
    struct ExpElement *parentNode;
    struct ExpElement *firstChild;
    struct ExpElement *lastChild;
    struct ExpElement *nextSibling;
    struct ExpElement *prevSibling;
} Node;

Node *newNode(Node *self)
{
    self->parentNode = 0;
    self->firstChild = 0;
    self->lastChild = 0;
    self->nextSibling = 0;
    self->prevSibling = 0;
    return self;
}

typedef struct Attr
{
    struct Attr *next;
    int type;
    char *name;
    char *value;
    int vallen;
} Attr;
static Attr *newAttr(const char *name, const char *value)
{
    Attr *self;
    int ln;
    self = (Attr *)mAlloc(sizeof(Attr), "Attr");
    self->type = ATTRIBUTE_NODE;
    self->vallen = strlen(value);
    ln = strlen(name) + self->vallen + 2;
    self->value = (char *)mAlloc(ln, "Attr1");
    memcpy(self->value, value, self->vallen + 1);
    self->name = &self->value[self->vallen + 1];
    memcpy(self->name, name, ln - self->vallen - 1);
    self->next = 0;
    return self;
}
static void delAttr(Attr *self)
{
    mFree(self->value, "delAttr1");
    mFree(self, "delAttr");
}

typedef struct NamedNodeMap
{
    Attr *head;
    Attr *tail;
} NamedNodeMap;
static NamedNodeMap *newNamedNodeMap(NamedNodeMap *self)
{
    if (0 == self)
        self = (NamedNodeMap *)mAlloc(sizeof(NamedNodeMap), "NamedNodeMap");
    self->head = 0;
    self->tail = 0;
    return self;
}
static NamedNodeMap *namedNodeMapAddAttr(NamedNodeMap *self, Attr *attr)
{
    attr->next = 0;
    if (0 == self->head)
    {
        self->head = self->tail = attr;
    }
    else
    {
        (*self->tail).next = attr;
        self->tail = attr;
    }
    return self;
}
static void delNamedNodeMap(NamedNodeMap *self)
{
    Attr *attr, *nxt;
    attr = self->head;
    while (0 != attr)
    {
        nxt = attr->next;
        delAttr(attr);
        attr = nxt;
    }
    mFree(self, "delNamedNodeMap");
}

typedef struct ExpElement
{
    Node node;
    int type;
    char *localName;
    char *qName;
    char *nsURI;
    NamedNodeMap *attrs;
} ExpElement;

static void doTriple(const char *triple, char** uri, char **localName, char **prefix)
{
    unsigned char *cp;
    unsigned char *ptrs[3];
    int ix, ln, ln2;
    ix = 0;
    ptrs[ix] = cp = (unsigned char *)triple;
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
        ln2 = ptrs[1] - ptrs[0] - 1;
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
        ln2 = ptrs[2] - ptrs[1] - 1;
        memcpy(*localName, ptrs[1], ln2);  // also copies trailing 0
        (*localName)[ln2] = '\0';
        ln2 = ptrs[1] - ptrs[0] - 1;
        *uri = (char *)mAlloc(ln2 + 1, "nsURI");
        memcpy(*uri, ptrs[0], ln2);
        (*uri)[ln2] = '\0';
    }
    else
    {
        fprintf(stderr, "logic error doTriple: bad pointer array count"); // error
    }
}
static ExpElement *newExpElementNS(ExpElement *self, const char *nsTriple)
{
    int ln;
    if (0 == self)
        self = (ExpElement *)mAlloc(sizeof(ExpElement), "ExpElement");
    self->type = ELEMENT_NODE;
    doTriple(nsTriple, &self->nsURI, &self->localName, &self->qName);
    newNode(&self->node);
    self->attrs = 0;
    return self;
}
static ExpElement *newExpElement(ExpElement *self, const char *localName)
{
    int ln;
    if (0 == self)
        self = (ExpElement *)mAlloc(sizeof(ExpElement), "ExpElement");
    self->type = ELEMENT_NODE;
    newNode(&self->node);
    ln = strlen(localName) + 1;
    self->localName = (char *)mAlloc(ln, "ExpElement");
    memcpy(self->localName, localName, ln);
    self->nsURI = 0;
    self->qName = 0;
    self->attrs = 0;
    return self;
}

static ExpElement *elementAppendChild(ExpElement *self, ExpElement *child)
{
    if (0 == self->node.firstChild)
    {
        self->node.firstChild = self->node.lastChild = child;
    }
    else
    {
        self->node.lastChild->node.nextSibling = child;
        child->node.prevSibling = self->node.lastChild;
        self->node.lastChild = child;
    }
    child->node.parentNode = self;
    return self;
}
static ExpElement *elementAddAttribute(ExpElement *self, Attr *attr)
{
    if (0 == self->attrs)
        self->attrs = newNamedNodeMap((NamedNodeMap *)0);
    namedNodeMapAddAttr(self->attrs, attr);
    return self;
}
static ExpElement *elementSetNamedNodeMap(ExpElement *self, NamedNodeMap *map)
{
    self->attrs = map;
    return self;
}
static void vdeExpElement(ExpElement *self)
{
    if (0 != self->qName)
        mFree(self->qName, "delElemqNm");
    else
        mFree(self->localName, "delElemlNm");
    if (0 != self->nsURI)
        mFree(self->nsURI, "delElemnsU");
    if (0 != self->attrs)
        delNamedNodeMap(self->attrs);
}
static void delExpElement(ExpElement *self)
{
    vdeExpElement(self);
    mFree(self, "delExpElement");
}

typedef struct ProcessingInstruction
{
    ExpElement elem;
    int type;
    char *key;
    char *data;
} ProcessingInstruction;
static void delProcessingInstruction(ProcessingInstruction *);

static ProcessingInstruction *newProcessingInstruction(ProcessingInstruction *self, const char *key, const char *data)
{
    int ln;
    if (0 == self)
        self = (ProcessingInstruction *)mAlloc(sizeof(ProcessingInstruction), "PI");
    newExpElement(&self->elem, PI_NAME);
    self->type = PROCESSING_INSTRUCTION_NODE;
    ln = strlen(key) + 1;
    self->key = mAlloc(ln, "piKey");
    memcpy(self->key, key, ln);
    ln = strlen(data) + 1;
    self->key = mAlloc(ln, "piData");
    memcpy(self->data, data, ln);
    return self;
}
static void vdeProcessingInstruction(ProcessingInstruction *self)
{
    vdeExpElement(&self->elem);  // super class cleanup
    mFree(self->key, "delpiKey");
    mFree(self->data, "delpiData");
}
static void delProcessingInstruction(ProcessingInstruction *self)
{
    vdeProcessingInstruction(self);
    mFree(self, "delPI");
}
typedef struct Comment
{
    ExpElement elem;
    int type;
    char *comment;
} Comment;
static void delComment(Comment *);

static Comment *newComment(Comment *self, const char *cmt)
{
    int ln;
    if (0 == self)
        self = (Comment *)mAlloc(sizeof(Comment), "PI");
    newExpElement(&self->elem, COMMENT_NAME);
    self->type = COMMENT_NODE;
    ln = strlen(cmt) + 1;
    self->comment = mAlloc(ln, "cmmData");
    memcpy(self->comment, cmt, ln);
    return self;
}
static void vdeComment(Comment *self)
{
    vdeExpElement(&self->elem);  // super class cleanup
    mFree(self->comment, "delcmmData");
}
static void delComment(Comment *self)
{
    vdeComment(self);
    mFree(self, "delComment");
}
typedef struct CharacterData
{
    ExpElement elem;
    int type;
    char *data;
    int len;
} CharacterData;

static CharacterData *newCharacterData(CharacterData *self, const char *charData, int len)
{
    int ln;
    if (0 == self)
        self = (CharacterData *)mAlloc(sizeof(CharacterData), "CharData");
    if (textType == TEXT_NODE)
    {
        newExpElement((ExpElement *) &self->elem, PCDATA_NAME);
        self->type = TEXT_NODE;
    }
    else
    {
        newExpElement((ExpElement *) &self->elem, CDATA_NAME);
        self->type = CDATA_SECTION_NODE;
    }
    self->data = mAlloc(len, "pcData");
    memcpy(self->data, charData, len);
    self->len = len;
    return self;
}
static void vdeCharacterData(CharacterData *self)
{
    vdeExpElement(&self->elem);  // super class cleanup
    mFree(self->data, "delpcData");
}
static void delCharacterData(CharacterData *self)
{
    vdeCharacterData(self);
    mFree(self, "delCharData");
}

typedef struct DocType
{
    ExpElement elem;
    int type;
    char *name;
    char *sysid;
    char *pubid;
} DocType;

static DocType *newDocType(DocType *self, const char *doctypeName, const char *sysid, const char *pubid)
{
    int nln, sln, pln;
    if (0 == self)
        self = (DocType *)mAlloc(sizeof(DocType), "Doctype");
    newExpElement((ExpElement *) &self->elem, DOCTYPE_NAME);
    self->type = DOCTYPE_NODE;
    nln = sln = pln = 0;
    nln = strlen(doctypeName) + 1;
    if (0 != sysid)
        sln = strlen(sysid) + 1;
    if (0 != pubid)
        pln = strlen(pubid) + 1;
    self->name = mAlloc(nln + sln + pln, "dtName");
    memcpy(self->name, doctypeName, nln);
    if (0 != sln)
    {
        memcpy(self->name + nln, sysid, sln);
        self->sysid = self->name + nln;
    }
    else
    {
        self->sysid = 0;
    }
    if (0 != pln)
    {
        memcpy(self->name + nln + sln, pubid, pln);
        self->pubid = self->name + nln + sln;
    }
    else
    {
        self->pubid = 0;
    }
    return self;
}
static void vdeDocType(DocType *self)
{
    vdeExpElement(&self->elem);  // super class cleanup
    mFree(self->name, "deldtName");
}
static void delDocType(DocType *self)
{
    vdeDocType(self);
    mFree(self, "delDT");
}
void delDocumentTree(ExpElement *self)
{
    ExpElement *seq, *nxt;
    seq = self->node.firstChild;
    while (0 != seq)
    {
        nxt = seq->node.nextSibling;
        delDocumentTree(seq);
        seq = nxt;
    }
    switch (self->type)
    {
    case ELEMENT_NODE:
    case DOCUMENT_NODE:
        delExpElement(self);
        break;
    case TEXT_NODE:
    case CDATA_SECTION_NODE:
        delCharacterData((CharacterData *)self);
        break;
    case PROCESSING_INSTRUCTION_NODE:
        delProcessingInstruction((ProcessingInstruction *)self);
        break;
    case COMMENT_NODE:
        delComment((Comment *)self);
        break;
    case DOCTYPE_NODE:
        delDocType((DocType *)self);
        break;
    default:
        fprintf(stderr, "delExpElementTree unknown node type %d\n", self->type);
        break;
    }
}
#if 0
static void XMLCALL
start(void *data, const XML_Char *el, const XML_Char **attr)
{
    int i;

    for (i = 0; i < Depth; i++)
        printf("  ");

    printf("%" XML_FMT_STR, el);

    for (i = 0; attr[i]; i += 2) {
        printf(" %" XML_FMT_STR "='%" XML_FMT_STR "'", attr[i], attr[i + 1]);
    }

    printf("\n");
    Depth++;
}

static void XMLCALL
end(void *data, const XML_Char *el)
{
    (void)data;
    (void)el;

    Depth--;
}
#endif

/**
 * @inheritDoc
 * Create a new ExpElement and append it to the current ExpElement.  Create an Attr node for each attribute
 * and add it to the current ExpElement.
 */
static void
elementStart(void *userData, const XML_Char *qName, const XML_Char **attrs)
{
    ExpElement *elm;
    Attr *attr;
    int ix;
    NamedNodeMap *nmp;
    trace(1);
    int tm;
    //tm = clock();
    //logger.debug(String.format("start element tag %s", qName));
    //elm = new ExpElement(qName, localName, uri);
    //elm = newExpElementNS((ExpElement *)0, qName);
    elm = newExpElement((ExpElement *)0, qName);
    elementAppendChild(current, elm);
    current = elm;
    // for (ix = 0; attr[ix]; ix += 2) {
        // printf(" %" XML_FMT_STR "='%" XML_FMT_STR "'", attr[i], attr[i + 1]);
    // }
    if (0 != attrs)
    {
        for (ix = 0; 0 != attrs[ix]; ix += 2)
        {
            // attr = newAttr(attrs.getQName(ix), attrs.getLocalName(ix), attrs.getURI(ix), attrs.getValue(ix));
            attr = newAttr(attrs[ix], attrs[ix + 1]);
            //logger.debug(String.format("  attribute %s", attrs.getQName(ix)));
            elementAddAttribute(elm, attr);
        }
    }
    //ms_elementBuild += clock() - tm;
}
/**
 * @inheritDoc
 * The ending element is closed by making its parent the current object.
 */
static void
elementEnd(void *userData, const XML_Char *data)
{
    trace(2);
    //logger.debug(String.format("end element tag %s", current.nodeName));
    current = current->node.parentNode;
    //logger.debug(String.format("parent element tag %s", current.nodeName));
}
static void startDoctypeDeclHandler(void *userData,
        const XML_Char *doctypeName,
        const XML_Char *sysid,
        const XML_Char *pubid, int has_internal_subset)
{
    DocType *self;
    self = newDocType((DocType *)0, doctypeName, sysid, pubid);
    elementAppendChild(current, &self->elem);
}
static void endDoctypeDeclHandler(void *userData)
{

}
static void characterDataHandler(void *userData, const XML_Char *data, int len)
{
    CharacterData *self;
    self = newCharacterData((CharacterData *)0, data, len);
    elementAppendChild(current, &self->elem);
}
static void processingInstructionHandler(void *userData, const XML_Char *target, const XML_Char *data)
{
    ProcessingInstruction *self;
    self = newProcessingInstruction((ProcessingInstruction *)0, target, data);
    elementAppendChild(current, &self->elem);
}
static void commentHandler(void *userData, const XML_Char *data)
{
    Comment *self;
    self = newComment((Comment *)0, data);
    elementAppendChild(current, &self->elem);
}
static void startCdataSectionHandler(void *userData)
{
    textType = CDATA_SECTION_NODE;
}
static void endCdataSectionHandler(void *userData)
{
    textType = TEXT_NODE;
}
#if 0
#define BUFFSIZE        8192

char Buff[BUFFSIZE];
int
main(int argc, char *argv[])
{
    char *fileName = "/Users/danevans/Pace/research/dissertation/test-files/British_Royal_Family_Tree.xhtml";

    XML_Parser p = XML_ParserCreate((void *)0);
    XML_SetElementHandler(p, start, end);

    (void)argc;
    (void)argv;

    if (! p) {
        fprintf(stderr, "Couldn't allocate memory for parser\n");
        exit(-1);
    }

    freopen(fileName,"r",stdin);

    for (;;) {
        int done;
        int len;

        len = (int)fread(Buff, 1, BUFFSIZE, stdin);
        if (ferror(stdin)) {
            fprintf(stderr, "Read error\n");
            exit(-1);
        }
        done = feof(stdin);

        if (XML_Parse(p, Buff, len, done) == XML_STATUS_ERROR) {
            fprintf(stderr,
                    "Parse error at line %" XML_FMT_INT_MOD "u:\n%" XML_FMT_STR "\n",
                    XML_GetCurrentLineNumber(p),
                    XML_ErrorString(XML_GetErrorCode(p)));
            exit(-1);
        }

        if (done)
            break;
    }
    XML_ParserFree(p);
    return 0;
}
#endif

clock_t getDOMTime()
{
    return ms_elementBuild;
}

void *expatDeserialize(char *buf, int len)
{
    textType = TEXT_NODE;
    document = newExpElement((ExpElement *) 0, DOC_NAME);
    document->type = DOCUMENT_NODE;
    ms_elementBuild = 0;
    current = document;
    //fprintf(stderr, "%d:%.64s\n", len, buf);
    //XML_Parser p = XML_ParserCreate(ENC, SEP);
    XML_Parser p = XML_ParserCreate((void *)0);
    if (! p) {
        fprintf(stderr, "Couldn't allocate memory for parser\n");
        exit(-1);
    }
    //XML_SetReturnNSTriplet(p, 1);
    XML_SetElementHandler(p, elementStart, elementEnd);
    XML_SetProcessingInstructionHandler(p, processingInstructionHandler);
    XML_SetCharacterDataHandler(p, characterDataHandler);
    XML_SetCommentHandler(p, commentHandler);
    XML_SetCdataSectionHandler(p, startCdataSectionHandler, endCdataSectionHandler);
    XML_SetDoctypeDeclHandler(p, startDoctypeDeclHandler, endDoctypeDeclHandler);
    trace(3);
    if (XML_Parse(p, buf, len, 1) == XML_STATUS_ERROR) {
        trace(7);
        fprintf(stderr,
                "Parse error at line %" XML_FMT_INT_MOD "u:\n%" XML_FMT_STR "\n",
                XML_GetCurrentLineNumber(p),
                XML_ErrorString(XML_GetErrorCode(p)));
        exit(-1);
    }
    trace(4);
    XML_ParserFree(p);
    if (current != document)
        fprintf(stderr, "inconsistent document and root element parent pointers");
    trace(6);
    return document;
}