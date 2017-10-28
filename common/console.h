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

#define CONSOLE_ADD_COLOR_STYLE1(color, _style, _bg, _fg, _default_msg) \
    ConsoleStyle& color ( const char * msg );\
    ConsoleStyle& color ( );

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

    CONSOLE_ADD_COLOR_STYLE1(Warning, style::reset, bg::reset, fg::yellow, " Warning ")
    CONSOLE_ADD_COLOR_STYLE1(Info   , style::reset, bg::reset, fg::gray,   " Info    ")
    CONSOLE_ADD_COLOR_STYLE1(Debug  , style::reset, bg::gray , fg::black,  "  Debug  ")
    CONSOLE_ADD_COLOR_STYLE1(Error  , style::reset, bg::reset, fg::red  ,  "  Error  ")

    CONSOLE_ADD_COLOR_STYLE1(Red   , style::reset, bg::reset, fg::red,   " Info    ")
    CONSOLE_ADD_COLOR_STYLE1(Green , style::reset, bg::reset, fg::green,   " Info    ")
    CONSOLE_ADD_COLOR_STYLE1(Yellow, style::reset, bg::reset, fg::yellow,   " Info    ")
    CONSOLE_ADD_COLOR_STYLE1(Blue  , style::reset, bg::reset, fg::blue,   " Info    ")
    CONSOLE_ADD_COLOR_STYLE1(Magenta, style::reset, bg::reset, fg::magenta,   " Info    ")
    CONSOLE_ADD_COLOR_STYLE1(Cyan  , style::reset, bg::reset, fg::cyan,   " Info    ")
    CONSOLE_ADD_COLOR_STYLE1(Gray , style::reset, bg::reset, fg::gray,   " Info    ")

}

#endif