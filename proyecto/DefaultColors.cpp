#include "DefaultColors.h"

ALLEGRO_COLOR black;
ALLEGRO_COLOR white;
ALLEGRO_COLOR grey;
ALLEGRO_COLOR lightgrey;
ALLEGRO_COLOR darkgrey;
ALLEGRO_COLOR red;
ALLEGRO_COLOR green;
ALLEGRO_COLOR blue;
ALLEGRO_COLOR yellow;
ALLEGRO_COLOR magenta;
ALLEGRO_COLOR cyan;

void InitDefaultColors()
{
	black = al_map_rgb(0, 0, 0);
	white = al_map_rgb(255, 255, 255);
	grey = al_map_rgb(127, 127, 127);
	lightgrey = al_map_rgb(191, 191, 191);
	darkgrey = al_map_rgb(63, 63, 63);
	red = al_map_rgb(255, 0, 0);
	green = al_map_rgb(0, 255, 0);
	blue = al_map_rgb(0, 0, 255);
	yellow = al_map_rgb(255, 255, 0);
	magenta = al_map_rgb(255, 0, 255);
	cyan = al_map_rgb(0, 255, 255);
}
