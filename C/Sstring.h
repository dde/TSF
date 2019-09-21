#ifndef STRING_INCLUDE_GUARD
#define STRING_INCLUDE_GUARD
#include <stdio.h>
#ifdef STRING_EXPORT
#define STRING_API
#else
#define STRING_API extern
#endif
typedef char String;
STRING_API String *newString(unsigned int);
STRING_API void delString(String *);
STRING_API int stringgetLine(FILE *fp, char **iobuf);
STRING_API String *stringCopy(const String *str);
STRING_API String *stringCut(const char *start, const char *end);
STRING_API String *stringConcat(const String *str1, const String *str2);
STRING_API String *stringConcat2(const char *chr1, unsigned ln1, const char *chr2, unsigned ln2);
STRING_API String *stringConcatV(const String *str1, ...);
STRING_API void stringAppend(String **str1, const String *str2);
STRING_API void stringAppendV(String **str1, ...);
STRING_API void stringAssign(String **target, const String *source);
STRING_API void stringAssignV(String **target, const String *source, ...);
STRING_API int stringIsDigits(const String *str);
STRING_API int stringIsBlank(const String *str);
STRING_API char *stringToupper(const char *str);
STRING_API char *stringTrim(const char *str);
#undef STRING_API
#endif
