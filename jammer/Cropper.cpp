#include "Cropper.h"
#include "DefaultColors.h"
#include "GuiMaster.h"

Cropper::Cropper(const Rect& cropref) : cropping(cropref)
{
}

void Cropper::Draw()
{
	glm::ivec2 cropsize;
	al_get_clipping_rectangle(&previousCropping.tl.x, &previousCropping.tl.y, &cropsize.x, &cropsize.y);
	previousCropping.resize(cropsize);
	Rect newCrop(cropping);
	newCrop.transform(gui.CurrentTransform());
	newCrop.cropFrom(previousCropping);

	al_set_clipping_rectangle(newCrop.tl.x, newCrop.tl.y, newCrop.w(), newCrop.h());
}

void Cropper::PostDraw()
{
	al_set_clipping_rectangle(previousCropping.tl.x, previousCropping.tl.y, previousCropping.w(), previousCropping.h());
}

