#ifndef __RECT_H__
#define __RECT_H__

#include <glm/glm.hpp>
#include <allegro5/allegro_primitives.h>
#include "MathUtils.h"
#include "Draggable.h"

class Rect {
public:
	union {
		glm::ivec2 topleft;
		glm::ivec2 tl;
	};
	
	union {
		glm::ivec2 bottomright;
		glm::ivec2 br;
	};

	Rect() : tl{}, br{} {}
	Rect(const glm::ivec2& itl, const glm::ivec2& ibr) : tl(itl), br(ibr) {}
	Rect(const Rect& rhs) : tl(rhs.tl), br(rhs.br) {}

	inline glm::ivec2 tr() const {
		return glm::ivec2(br.x, tl.y);
	}

	inline glm::ivec2 bl() const {
		return glm::ivec2(tl.x, br.y);
	}

	inline int w() const {
		return br.x - tl.x;
	}

	inline int h() const {
		return br.y - tl.y;
	}

	inline void resize(const glm::ivec2& newsize) {
		br = tl + newsize;
	}

	inline void resize(int newWidth, int newHeight) {
		resize(glm::ivec2(newWidth, newHeight));
	}

	void operator=(const Rect& rhs) {
		tl = rhs.tl;
		br = rhs.br;
	}

	void operator+=(const glm::ivec2& rhs) {
		tl += rhs;
		br += rhs;
	}

	void operator-=(const glm::ivec2& rhs) {
		tl -= rhs;
		br -= rhs;
	}

	Rect operator+(const glm::ivec2& rhs) const {
		return Rect(tl + rhs, br + rhs);
	}

	Rect operator-(const glm::ivec2& rhs) const {
		return Rect(tl - rhs, br - rhs);
	}

	bool isInside(const glm::ivec2& rhs) const {
		return valueInside(rhs.x, tl.x, br.x) && valueInside(rhs.y, tl.y, br.y);
	}

	inline void draw(ALLEGRO_COLOR color, float thickness) {
		al_draw_rectangle(tl.x, tl.y, br.x, br.y, color, thickness);
	}

	inline void draw_filled(ALLEGRO_COLOR color) {
		al_draw_filled_rectangle(tl.x, tl.y, br.x, br.y, color);
	}

	inline void draw_rounded(float rx, float ry, ALLEGRO_COLOR color, float thickness) {
		al_draw_rounded_rectangle(tl.x, tl.y, br.x, br.y, rx, ry, color, thickness);
	}

	inline void draw_filled_rounded(float rx, float ry, ALLEGRO_COLOR color) {
		al_draw_filled_rounded_rectangle(tl.x, tl.y, br.x, br.y, rx, ry, color);
	}

	void cropFrom(const Rect& other) {
		tl.x = max(tl.x, other.tl.x);
		tl.y = max(tl.y, other.tl.y);
		br.x = min(br.x, other.br.x);
		br.y = min(br.y, other.br.y);
	}

	void transform(const ALLEGRO_TRANSFORM& transform) {
		float ftlx = tl.x;
		float ftly = tl.y;
		float fbrx = br.x;
		float fbry = br.y;
		al_transform_coordinates(&transform, &ftlx, &ftly);
		al_transform_coordinates(&transform, &fbrx, &fbry);
		tl.x = ftlx;
		tl.y = ftly;
		br.x = fbrx;
		br.y = fbry;
	}
};



#endif