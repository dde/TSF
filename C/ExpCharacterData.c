//
// Created by Dan Evans on 2019-01-07.
//
#include <string.h>
#include <stdlib.h>
#include "Memory.h"
#include "ExpElement.h"
#include "ExpCharacterData.h"
static void vdeExpCharacterData(ExpCharacterData *self);

static char PCDATA[] = "#text";
static ExpCharacterDataMtb methodTable = {vdeExpCharacterData};
typedef struct ExpCharacterDataI
{
    ExpCharacterDataMtb *mtb;
    ExpElement elem;
    char *data;
    int len;
} ExpCharacterDataI;

ExpCharacterData *newExpCharacterData(ExpCharacterData *selfx, char *charData, int len)
{
    int ln;
    ExpCharacterDataI *self = (ExpCharacterDataI *)selfx;
    if (0 == self)
        self = (ExpCharacterData *)mAlloc(sizeof(ExpCharacterData), "CharData");
    self->mtb = &methodTable;
    newExpElement((ExpElement *)&self->elem, PCDATA);
    self->data = mAlloc(len, "pcData");
    memcpy(self->data, charData, len);
    self->len = len;
    return self;
}
static void vdeExpCharacterData(ExpCharacterData *selfx)
{
    ExpCharacterDataI *self = (ExpCharacterDataI *)selfx;
    ExpElement *par = (ExpElement *)&self->elem;
    (*par->mtb).vdeExpElement(par);  // super class cleanup
    mFree(self->data, "delpcData");
}
void delExpCharacterData(ExpCharacterData *self)
{
    vdeExpCharacterData(self);
    mFree(self, "delCharData");
}