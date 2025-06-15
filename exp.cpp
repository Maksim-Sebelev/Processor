#include <stdio.h>

int main()
{
    const char* file = "tmp.txt";

    FILE* fp = fopen(file, "wbsd");

    int c = 0;

    while((c = getc(fp)) != EOF)
    {
        printf( "%c", c);
    }


    return 0;
}
