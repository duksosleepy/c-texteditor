#include <stdio.h>

#ifdef _WIN32
#include <io.h>
#define STDIN_FILENO 0
#else
#include <unistd.h>
#endif

int main()
{
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q')
        ;
    return 0;
}
