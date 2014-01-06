#pragma once

#include <Windows.h>

// Custom colors
#define BLUE             FOREGROUND_INTENSITY | FOREGROUND_BLUE
#define GREEN            FOREGROUND_INTENSITY | FOREGROUND_GREEN
#define RED              FOREGROUND_INTENSITY | FOREGROUND_RED
#define YELLOW           FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED
#define WHITE            FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED
#define GRAY             FOREGROUND_INTENSITY
#define DARK_GREEN       FOREGROUND_GREEN

#define BACKGROUND_WHITE BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE

#define FOREGROUND_ONE FOREGROUND_BLUE
#define BACKGROUND_ONE BACKGROUND_BLUE

typedef int ColorType;

namespace Colorer
{
    void setColor(ColorType color);
    void restoreColor();

    template <ColorType color>
    void print(const char* format, ...)
    {
        va_list arg;
        va_start(arg, format);

        Colorer::setColor(color);
        vprintf(format, arg);
        Colorer::restoreColor();

        va_end(arg);
    }
}
