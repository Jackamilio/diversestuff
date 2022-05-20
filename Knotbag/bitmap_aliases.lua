if bitmaps == nil then
	bitmaps = {
		errortexture = "errortexture.png",
		failtex = "nofileherelol.png",
		banana = "banana.png",
		monkey = "cutemonkey.png"}
end

local show, cont = imgui.Begin("Bitmap aliases", true)

if show then
	local change = {}
	for bmp, f in pairs(bitmaps) do
		local newval = imgui.InputText(f, bmp)
		if bmp ~= newval then
			change[bmp] = newval
			break
		end
	end
	for old, new in pairs(change) do
		bitmaps[new] = bitmaps[old]
		bitmaps[old] = nil
	end

	if (imgui.Button("New alias")) then
		bitmaps["New alias"] = "no address"
	end
end

imgui.End()

return cont