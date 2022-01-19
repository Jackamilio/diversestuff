#include "CropperDisplacer.h"
#include "Rect.h"

CropperDisplacer::CropperDisplacer() : cropping(nullptr)
{
}

void CropperDisplacer::AddForDisplacement(Engine::Input* inputobj, Engine::Graphic* graphicobj)
{
	Engine::Input::AddChild(inputobj);
	Engine::Graphic::AddChild(graphicobj);
}

void CropperDisplacer::RemoveFromDisplacement(Engine::Input* inputobj, Engine::Graphic* graphicobj)
{
	Engine::Input::RemoveChild(inputobj);
	Engine::Graphic::RemoveChild(graphicobj);
}

bool CropperDisplacer::Event(ALLEGRO_EVENT& event)
{
	static bool ignoreThisTime = false;
	if (ignoreThisTime) {
		return false;
	}

	// catch every mouse event
	if (event.type == ALLEGRO_EVENT_MOUSE_AXES
		|| event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN
		|| event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
		|| event.type == ALLEGRO_EVENT_MOUSE_WARPED
		|| event.type == ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY
		|| event.type == ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY) {

		// deny input for children outside cropping
		if (cropping && !cropping->isInside(glm::ivec2(event.mouse.x, event.mouse.y))) {
			return false;
		}

		// trick next inputs into a different mouse position
		glm::ivec2 d = GetDisplaceOffset();
		event.mouse.x -= d.x;
		event.mouse.y -= d.y;

		// resume the recursivity by taking responsibility
		ignoreThisTime = true;
		Engine::RecursiveInput(this, event);
		ignoreThisTime = false;

		// stop the original recursivity
		return true;
	}

	return false;
}

void CropperDisplacer::Draw()
{
	engine.graphics.PushOverlayTransform();
	engine.graphics.TranslateOverlayTransform(GetDisplaceOffset());
	
	if (cropping) {
		al_set_clipping_rectangle(cropping->tl.x, cropping->tl.y, cropping->w(), cropping->h());
	}
}

void CropperDisplacer::PostDraw()
{
	engine.graphics.PopOverlayTransform();

	if (cropping) {
		al_reset_clipping_rectangle();
	}
}
