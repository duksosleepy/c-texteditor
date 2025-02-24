/*** includes ***/
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#define CTRL_KEY(k) ((k) & 0x1f)

#ifdef _WIN32
#include <io.h>
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#include <windows.h>
static DWORD originalConsoleMode;
struct editorConfig {
    HANDLE hStdin;
    HANDLE hStdout;
    DWORD  originalConsoleMode;
};

struct editorConfig E;
#else
#include <termios.h>
static struct termios orig_termios;
#include <unistd.h>
struct editorConfig {
    struct termios orig_termios;
};
struct editorConfig E;
#endif
/*** data ***/

void die(const char* s)
{
    _write(STDOUT_FILENO, "\x1b[2J", 4);
    _write(STDOUT_FILENO, "\x1b[H", 3);
    perror(s);
    exit(1);
}
/*** terminal ***/
void restoreConsoleMode(HANDLE hConsole, DWORD originalMode)
{
    if (!SetConsoleMode(hConsole, originalMode))
    {
        die("SetConsoleMode");
    }
}
/*** terminal ***/
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
/*** terminal ***/
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

/*** output ***/
void editorDrawRows()
{
    int y;
    for (y = 0; y < 24; y++)
    {
        _write(STDOUT_FILENO, "~\r\n", 3);
    }
}

void editorRefreshScreen()
{
    _write(STDOUT_FILENO, "\x1b[2J", 4);
    _write(STDOUT_FILENO, "\x1b[H", 3);
    editorDrawRows();
    _write(STDOUT_FILENO, "\x1b[H", 3);
}
/*** input ***/
char editorReadKey()
{
    int  nread;
    char c;
    while (nread = _read(STDIN_FILENO, &c, 1) != 1)
    {
        if (nread == -1 && errno != EAGAIN)
            die("read");
    }
    return c;
}
/*** input ***/
void editorProcessKeypress()
{
    char c = editorReadKey();
    switch (c)
    {
    case CTRL_KEY('q'):
        _write(STDOUT_FILENO, "\x1b[2J", 4);
        _write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
        break;
    }
}

int main()
{
    enableRawMode();
    while (1)
    {
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}
