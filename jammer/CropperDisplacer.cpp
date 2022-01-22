#include "CropperDisplacer.h"
#include "DefaultColors.h"

CropperDisplacer::CropperDisplacer() : cropping(nullptr)
{
}

Engine::InputStatus CropperDisplacer::Event(ALLEGRO_EVENT& event)
{
	// catch every mouse event
	if (event.type == ALLEGRO_EVENT_MOUSE_AXES
		|| event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN
		|| event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
		|| event.type == ALLEGRO_EVENT_MOUSE_WARPED
		|| event.type == ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY
		|| event.type == ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY) {

		// deny input for children outside cropping
		if (cropping && !cropping->isInside(glm::ivec2(event.mouse.x, event.mouse.y))) {
			return Engine::InputStatus::notforchildren;
		}

		// trick next inputs into a different mouse position
		glm::ivec2 d = GetDisplaceOffset();
		event.mouse.x -= d.x;
		event.mouse.y -= d.y;

		// resume the recursivity by taking responsibility
		GuiMaster::RecursiveEvent(this, event, false);

		// go back to correct input position
		event.mouse.x += d.x;
		event.mouse.y += d.y;

		// stop the original recursivity
		return Engine::InputStatus::grabbed;
	}

	return Engine::InputStatus::ignored;
}

void CropperDisplacer::Draw()
{
	if (cropping) {
		glm::ivec2 cropsize;
		al_get_clipping_rectangle(&previousCropping.tl.x, &previousCropping.tl.y, &cropsize.x, &cropsize.y);
		previousCropping.resize(cropsize);
		Rect newCrop(*cropping);
		newCrop.transform(gui.CurrentTransform());
		newCrop.cropFrom(previousCropping);

		//al_reset_clipping_rectangle();
		//engine.graphics.PushOverlayTransform();
		//engine.graphics.IdentityOverlayTransform();
		//newCrop.draw(green, 1);
		//engine.graphics.PopOverlayTransform();

		al_set_clipping_rectangle(newCrop.tl.x, newCrop.tl.y, newCrop.w(), newCrop.h());
	}

	gui.PushTransform();
	gui.TranslateTransform(GetDisplaceOffset());
}

void CropperDisplacer::PostDraw()
{
	gui.PopTransform();

	if (cropping) {
		al_set_clipping_rectangle(previousCropping.tl.x, previousCropping.tl.y, previousCropping.w(), previousCropping.h());
	}
}
