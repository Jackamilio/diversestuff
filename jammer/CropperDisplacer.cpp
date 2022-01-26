#include "CropperDisplacer.h"
#include "DefaultColors.h"
#include "GuiMaster.h"

CropperDisplacer::CropperDisplacer() : cropping(nullptr)
{
}

Engine::InputStatus CropperDisplacer::Event(ALLEGRO_EVENT& event)
{
	// catch every mouse event
	if (event.type == ALLEGRO_EVENT_MOUSE_AXES ||
		event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
		event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP ||
		event.type == ALLEGRO_EVENT_MOUSE_WARPED ||
		event.type == ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY ||
		event.type == ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY
		) {

		// deny input for children outside cropping
		bool insidecropping = InsideCropping(glm::ivec2(event.mouse.x, event.mouse.y));
		if (!insidecropping) {
			return Engine::InputStatus::notforchildren;
		}

		// trick next inputs into a different mouse position
		glm::ivec2 d = GetDisplaceOffset();
		event.mouse.x -= d.x;
		event.mouse.y -= d.y;

		// resume the recursivity by taking responsibility
		bool ret = GuiMaster::RecursiveEvent(this, event, false);

		// grab the button down in any case if it was inside the cropping
		if (insidecropping && event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
			ret = true;
		}

		// go back to correct input position
		event.mouse.x += d.x;
		event.mouse.y += d.y;

		// stop the original recursivity
		return ret ? Engine::InputStatus::grabbed : Engine::InputStatus::notforchildren;
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

glm::ivec2 CropperDisplacer::CalculateGlobalDisplaceOffset() const
{
	glm::ivec2 offset(GetDisplaceOffset());

	const GuiElement* parent = Parent();
	while (parent)
	{
		const CropperDisplacer* cdparent = dynamic_cast<const CropperDisplacer*>(parent);
		if (cdparent) {
			offset += cdparent->GetDisplaceOffset();
		}
		parent = parent->Parent();
	}

	return offset;
}
