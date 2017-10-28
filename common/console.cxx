#include "console.h"

namespace Miyuki::Console {

    CONSOLE_ADD_COLOR_STYLE(Warning, style::reset, bg::reset, fg::yellow, " Warning ")
    CONSOLE_ADD_COLOR_STYLE(Info   , style::reset, bg::reset, fg::gray,   " Info    ")
    CONSOLE_ADD_COLOR_STYLE(Debug  , style::reset, bg::gray , fg::black,  "  Debug  ")
    CONSOLE_ADD_COLOR_STYLE(Error  , style::reset, bg::reset, fg::red  ,  "  Error  ")

    CONSOLE_ADD_COLOR_STYLE(Red   , style::reset, bg::reset, fg::red,   " Info    ")
    CONSOLE_ADD_COLOR_STYLE(Green , style::reset, bg::reset, fg::green,   " Info    ")
    CONSOLE_ADD_COLOR_STYLE(Yellow, style::reset, bg::reset, fg::yellow,   " Info    ")
    CONSOLE_ADD_COLOR_STYLE(Blue  , style::reset, bg::reset, fg::blue,   " Info    ")
    CONSOLE_ADD_COLOR_STYLE(Magenta, style::reset, bg::reset, fg::magenta,   " Info    ")
    CONSOLE_ADD_COLOR_STYLE(Cyan  , style::reset, bg::reset, fg::cyan,   " Info    ")
    CONSOLE_ADD_COLOR_STYLE(Gray , style::reset, bg::reset, fg::gray,   " Info    ")

}