#ifndef __RECT_H__
#define __RECT_H__

#include <glm/glm.hpp>
#include <allegro5/allegro_primitives.h>
#include "MathUtils.h"
#include "Draggable.h"

class Rect {
public:
	union {
		glm::ivec4 ltrb;
		struct {
			union {
				glm::ivec2 topleft;
				glm::ivec2 tl;
			};
			union {
				glm::ivec2 bottomright;
				glm::ivec2 br;
			};
		};
		struct {
			int left;
			int top;
			int right;
			int bottom;
		};
		struct {
			int l;
			int t;
			int r;
			int b;
		};
	};

	Rect() : ltrb{} {}
	Rect(const glm::ivec2& itl, const glm::ivec2& ibr) : tl(itl), br(ibr) {}
	Rect(const Rect& rhs) : ltrb(rhs.ltrb) {}
	Rect(const glm::ivec4& rhs) : ltrb(rhs) {}

	inline glm::ivec2 tr() const {
		return glm::ivec2(r, t);
	}

	inline glm::ivec2 bl() const {
		return glm::ivec2(l, b);
	}

	inline int w() const {
		return right - left;
	}

	inline int h() const {
		return bottom - top;
	}

	inline void resize(const glm::ivec2& newsize) {
		br = tl + newsize;
	}

	inline void resize(int newWidth, int newHeight) {
		resize(glm::ivec2(newWidth, newHeight));
	}

	inline void expand(int addition) {
		l -= addition;
		t -= addition;
		r += addition;
		b += addition;
	}

	inline void shrink(int substraction) {
		expand(-substraction);
	}

	inline void operator=(const Rect& rhs) { ltrb = rhs.ltrb; }
	inline void operator=(const glm::ivec4& rhs) { ltrb = rhs; }
	inline bool operator == (const Rect& rhs) const { return ltrb == rhs.ltrb; }
	inline bool operator != (const Rect& rhs) const { return ltrb != rhs.ltrb; }

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
		return valueInside(rhs.x, left, right) && valueInside(rhs.y, top, bottom);
	}

	inline void draw(ALLEGRO_COLOR color, float thickness) {
		al_draw_rectangle(l, t, r, b, color, thickness);
	}

	inline void draw_filled(ALLEGRO_COLOR color) {
		al_draw_filled_rectangle(l, t, r, b, color);
	}

	inline void draw_rounded(float rx, float ry, ALLEGRO_COLOR color, float thickness) {
		al_draw_rounded_rectangle(l, t, r, b, rx, ry, color, thickness);
	}

	inline void draw_filled_rounded(float rx, float ry, ALLEGRO_COLOR color) {
		al_draw_filled_rounded_rectangle(l, t, r, b, rx, ry, color);
	}

	inline void draw_outlined(ALLEGRO_COLOR filling, ALLEGRO_COLOR outline, float thickness) {
		// slower but prettier this way than using a draw above a filled
		al_draw_filled_rectangle(l, t, r, b, outline);
		al_draw_filled_rectangle(l + thickness, t + thickness, r - thickness, b - thickness, filling);
	}

	inline void draw_outlined_rounded(float rx, float ry, ALLEGRO_COLOR filling, ALLEGRO_COLOR outline, float thickness) {
		al_draw_filled_rounded_rectangle(l, t, r, b, rx, ry, outline);
		al_draw_filled_rounded_rectangle(l + thickness, t + thickness, r - thickness, b - thickness, rx, ry, filling);
	}

	void cropFrom(const Rect& o) {
		l = max(l, o.l);
		t = max(t, o.t);
		r = min(r, o.r);
		b = min(b, o.b);
	}

	void mergeWith(const Rect& o) {
		l = min(l, o.l);
		t = min(t, o.t);
		r = max(r, o.r);
		b = max(b, o.b);
	}

	void transform(const ALLEGRO_TRANSFORM& transform) {
		float fl = l;
		float ft = t;
		float fr = r;
		float fb = b;
		al_transform_coordinates(&transform, &fl, &ft);
		al_transform_coordinates(&transform, &fr, &fb);
		l = fl;
		t = ft;
		r = fr;
		b = fb;
	}
};



#endif