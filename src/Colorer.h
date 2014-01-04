#pragma once

#include <Windows.h>

#define RED    FOREGROUND_RED   | FOREGROUND_INTENSITY
#define BLUE   FOREGROUND_BLUE  | FOREGROUND_INTENSITY
#define GREEN  FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define DARK_GREEN FOREGROUND_GREEN
#define YELLOW 0x0E
#define WHITE  0x0F
#define BACKGROUND_WHITE 0x70
#define PURPLE 0x0D
#define DEFAULT_COLOR DARK_GREEN

namespace Colorer
{
    void setColor(int color);
    void restoreColor();

    template <int color>
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
