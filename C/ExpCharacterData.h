//
// Created by Dan Evans on 2019-01-07.
//

#ifndef NPX_EXPCHARACTERDATA_H
#define NPX_EXPCHARACTERDATA_H
typedef struct ExpCharacterData ExpCharacterData;
typedef struct ExpCharacterDataMtb
{
    void (*vdeExpCharacterData)(ExpCharacterData *self);
} ExpCharacterDataMtb;
struct ExpCharacterData
{
    ExpCharacterDataMtb *mtb;
};
extern ExpCharacterData *newExpCharacterData(ExpCharacterData *self, char *charData, int len);
extern void delExpCharacterData(ExpCharacterData *self);

#endif //NPX_EXPCHARACTERDATA_H
