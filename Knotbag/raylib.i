%module raylib

#define bool bool
%{
#include <raylib.h>
%}

//%include "carrays.i"
//%array_functions(Mesh, MeshArray)
//%array_functions(Material, MaterialArray)
//%array_functions(MaterialMap, MaterialMapArray)
//%array_functions(Color, ColorArray)

%include <raylib.h>

%inline %{
#define ARRAYAT(type) type* arrayat(type* ar, int at) { return &ar[at]; }

ARRAYAT(Mesh)
ARRAYAT(Material)
ARRAYAT(MaterialMap)
ARRAYAT(Color)
%}

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
