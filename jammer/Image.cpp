#include "Rect.h"
#include "Image.h"
#include "GuiMaster.h"

Image::Image() : texture(nullptr), pos{}, rect(nullptr), stretch(false)
{
}

Image::Image(const Texture& tex) : texture(&tex), pos{}, rect(nullptr), stretch(false)
{
}

Image::Image(const char* file) : texture(&gui.engine.graphics.textures.Get(file)), pos{}, rect(nullptr), stretch(false)
{
}

void Image::ChangeTexture(const Texture& tex)
{
	texture = &tex;
}

void Image::ChangeTexture(const char* file)
{
	texture = &gui.engine.graphics.textures.Get(file);
}

void Image::RemoveTexture()
{
	texture = nullptr;
}

void Image::Draw()
{
	if (texture) {
		Rect r;
		const float w = texture->GetWidth();
		const float h = texture->GetHeight();
		if (rect) {
			r = *rect;
			if (!stretch) {
				const float texaspect = h / w;
				const float H = (float)r.h();
				const float W = (float)r.w();
				if (texaspect > H / W) {
					const int adjw = 0.5f * (W - H / texaspect);
					r.left += adjw;
					r.right -= adjw;
				}
				else {
					const int adjh = 0.5f * (H - W * texaspect);
					r.top += adjh;
					r.bottom -= adjh;
				}
			}
		}
		else r.resize(w, h);
		r += pos;
		al_draw_scaled_bitmap(texture->GetAlValue(), 0, 0, w, h, r.left, r.top, r.w(), r.h(), 0);
	}
}
