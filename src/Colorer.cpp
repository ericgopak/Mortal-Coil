#include <cstdio>
#include "Windows.h"
#include "Colorer.h"

namespace Colorer
{
    HANDLE hstdin  = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO csbi;

    BOOL ok = GetConsoleScreenBufferInfo(hstdout, &csbi);

    void setColor(ColorType color)
    {
        SetConsoleTextAttribute(hstdout, color);
    }

    void colorize(const char* text, ColorType color)
    {
        SetConsoleTextAttribute(hstdout, (WORD)color);
        printf("%s", text);
        restoreColor();
    }

    void restoreColor()
    {
        SetConsoleTextAttribute(hstdout, csbi.wAttributes);
    }
}