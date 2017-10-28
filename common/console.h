#ifndef _MIYUKI_CONSOLE_H
#define _MIYUKI_CONSOLE_H

#include "include/rang.hpp"

namespace Miyuki::Console {

    using namespace rang;

#define CONSOLE_ADD_COLOR_STYLE(color, _style, _bg, _fg, _default_msg) \
    ConsoleStyle& color ( const char * msg ) {\
        static ConsoleStyle _color ( _style, _fg, _bg );\
        _color.setMessage( msg ); \
        return _color;\
    }\
    ConsoleStyle& color ( ) {  return color ( _default_msg );  }

    class ConsoleStyle {
        style Style;
        fg    Foreground;
        bg    Background;
        const char * msg;

    public:
        ConsoleStyle(style s = style::reset, fg f = fg::reset, bg b = bg::reset) { setColor(s, f, b); }
        void setColor(style s = style::reset, fg f = fg::reset, bg b = bg::reset) {
            Style = s;  Foreground = f;  Background = b;
        };
        friend std::ostream& operator << (std::ostream &os, ConsoleStyle& s) {
            return os << s.Style << s.Foreground << s.Background << s.msg
                << style::reset << fg::reset << bg::reset << "  ";
        }
        void setMessage(const char * _msg) { msg = _msg; }

    };

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

#undef CONSOLE_ADD_COLOR_STYLE

}

#endif