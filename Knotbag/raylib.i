%module raylib

#define bool bool
%{
#include <raylib.h>
%}

%include <raylib.h>

%rename("%(strip:[SWIG_])s") "";

%inline %{
#define PARSE_COLORS(X) \
X(LIGHTGRAY) \
X(GRAY) \
X(DARKGRAY) \
X(YELLOW) \
X(GOLD) \
X(ORANGE) \
X(PINK) \
X(RED) \
X(MAROON) \
X(GREEN) \
X(LIME) \
X(DARKGREEN) \
X(SKYBLUE) \
X(BLUE) \
X(DARKBLUE) \
X(PURPLE) \
X(VIOLET) \
X(DARKPURPLE) \
X(BEIGE) \
X(BROWN) \
X(DARKBROWN) \
X(WHITE) \
X(BLACK) \
X(BLANK) \
X(MAGENTA) \
X(RAYWHITE)

#define CONST_COLOR(val) const Color SWIG_##val = val;

PARSE_COLORS(CONST_COLOR)
%}
