//
// Created by Dan Evans on 9/23/19.
//
#include <stdio.h>
#include <stddef.h>
int main(int argc, char **argv)
{
    fprintf(stdout, "sizeof(wchar_t) %ld\n", sizeof(wchar_t));
    fprintf(stdout, "sizeof(char *) %ld\n", sizeof(char *));
    return 0;
}
