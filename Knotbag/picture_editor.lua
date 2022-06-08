picture_editor = setmetatable(
{
	w = 640,
	h = 480,
	ew = 640,
	eh = 480,
	lmpos = imgui.ImVec2(0,0),
	thick = 5,
	r=0, g=0, b=0
},
{
	__gc = function (pe)
		if pe.picture then
			raylib.UnloadRenderTexture(pe.picture)
			pe.picture = nil
		end
	end
}
)
knotbag.set_window("Picture editor", function()
	local rl = raylib
	local pe = picture_editor
	_, pe.ew = imgui.InputInt("Width", pe.ew)
	_, pe.eh = imgui.InputInt("Height", pe.eh)
	local pic = pe.picture
	if imgui.Button("Set") then
		pe.w = pe.ew
		pe.h = pe.eh
		if pic == nil then
			pic = rl.LoadRenderTexture(pe.w, pe.h)
			rl.BeginTextureMode(pic)
			rl.ClearBackground(rl.WHITE)
			rl.EndTextureMode()
			pe.picture = pic
		else
			local lw = pic.texture.width
			local lh = pic.texture.height
			if lw ~= pe.w or lh ~= pe.h then
				pic = rl.LoadRenderTexture(pe.w, pe.h)
				rl.BeginTextureMode(pic)
				rl.ClearBackground(rl.WHITE)
				rl.DrawTexture(pe.picture.texture, 0, 0, rl.WHITE)
				rl.EndTextureMode()
				rl.UnloadRenderTexture(pe.picture)
				pe.picture = pic
			end
		end
	end

	local iio = imgui.GetIO()
	local mpos = imgui.ImVec2(imgui.GetCursorScreenPos())
	mpos = iio.MousePos - mpos

	if pic then
		if iio:MouseDown(0) then
			if mpos.x >= 0 and mpos.x < pic.texture.width
			and mpos.y >=0 and mpos.y < pic.texture.height then
				pe.clicstart = mpos
			end
		else
			pe.clicstart = nil
		end
		if pe.clicstart then
			rl.BeginTextureMode(pic)
			local col = rl.Color()
			col.r = pe.r * 255
			col.g = pe.g * 255
			col.b = pe.b * 255
			col.a = 255
			local from = rl.Vector2()
			from.x = pe.lmpos.x
			from.y = pic.texture.height - pe.lmpos.y
			local to = rl.Vector2()
			to.x = mpos.x
			to.y = pic.texture.height - mpos.y
			rl.DrawLineEx(from, to, pe.thick, col)
			rl.DrawCircle(to.x, to.y, pe.thick / 2, col)
			rl.DrawCircle(from.x, from.y, pe.thick / 2, col)
			rl.EndTextureMode()
		end
	end
	
	pe.lmpos = mpos
	
	if pe.picture then
		imgui.Image(pe.picture.texture, pe.w, pe.h)
	else
		imgui.Text("Set or load a new image")
	end
	
	_,pe.r,pe.g,pe.b = imgui.ColorEdit3("Color",pe.r,pe.g,pe.b)
	_,pe.thick = imgui.DragInt("Thickness", pe.thick, 1, 1, 100)
	
	imgui.Text("Mouse: "..mpos.x..", "..mpos.y)
	if pe.clicstart then
		imgui.Text("To: "..pe.clicstart.x..", "..pe.clicstart.y)
	end
	
	return true
end)