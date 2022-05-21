if bitmaps == nil then
	bitmaps = {
		{name = "errortexture", file = "errortexture.png"},
		{name = "failtex", file = "nofileherelol.png"},
		{name = "banana", file = "banana.png"},
		{name = "monkey", file = "cutemonkey.png"}
	}
end

local show, cont = imgui.Begin("Bitmap aliases", true)

if show then
	local fd = imgui.GetFileDialog()
	for t, v in pairs(bitmaps) do
		imgui.PushID(t)
		v.name = imgui.InputText("", v.name)
		imgui.SameLine()
		if imgui.Button("...") then
			fd:Open("LoadTextureForAlias", "Open a file", "Image (*.bmp,*.png){.bmp,.png},.*")
		end
		imgui.SameLine()
		imgui.Text(v.file)
		imgui.PopID()
	end

	if (imgui.Button("New alias")) then
		table.insert(bitmaps, {name = "new texture", file = "no file yet"})
	end

	if fd:IsDone("LoadTextureForAlias") then
		print(fd:GetResult())
		fd:Close()
	end
end

imgui.End()

return cont