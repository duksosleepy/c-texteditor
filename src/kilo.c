#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <io.h>
#define STDIN_FILENO 0
#include <windows.h>
static DWORD originalConsoleMode;
#else
#include <termios.h>
static struct termios orig_termios;
#include <unistd.h>
#endif

void die(const char* s)
{
    perror(s);
    exit(1);
}

void restoreConsoleMode(HANDLE hConsole, DWORD originalMode)
{
    if (!SetConsoleMode(hConsole, originalMode))
    {
        die("SetConsoleMode");
    }
}

void disableRawMode()
{
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(hStdin, originalConsoleMode);
#else
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
#endif
}

void enableRawMode()
{
#ifdef _WIN32
    HANDLE       hStdin = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE       hSerial;
    COMMTIMEOUTS timeouts;
    DWORD        mode;
    GetConsoleMode(hStdin, &mode);
    timeouts.ReadIntervalTimeout        = 100;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant   = 100;
    SetCommTimeouts(hSerial, &timeouts);
    mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
    SetConsoleMode(hStdin, mode);
#else
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        die("tcgetattr");
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | ICRNL | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
#endif
}

int main()
{
    enableRawMode();
    while (1)
    {
        char c = '\0';
        _read(STDIN_FILENO, &c, 1);
        if (_read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");
        if (iscntrl(c))
        {
            printf("%d\r\n", c);
        }
        else
        {
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q')
            break;
    }
    return 0;
}
