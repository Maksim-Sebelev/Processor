#ifndef ONEGIN_H
#define ONEGIN_H


#include <stdio.h>


const char** ReadFile    (const char*  file, size_t* bufSize);
int          strtoi      (const char*  str);
void         BufferDtor  (const char** buffer);

size_t       CalcFileLen (const char* fileName);


#endif
