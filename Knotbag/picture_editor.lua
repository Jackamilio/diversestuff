picture_editor = {w = 640, h = 480, ew = 640, eh = 480, lmpos = imgui.ImVec2(0,0), r=1, g=1, b=1 }
knotbag.set_window("Picture editor", function()
	local pe = picture_editor
	_, pe.ew = imgui.InputInt("Width", pe.ew)
	_, pe.eh = imgui.InputInt("Height", pe.eh)
	local pic = pe.picture
	if imgui.Button("Set") then
		pe.w = pe.ew
		pe.h = pe.eh
		if not pic then
			pic = al.create_bitmap(pe.w, pe.h)
			local ot = al.get_target_bitmap()
			al.set_target_bitmap(pic)
			al.clear_to_color(al.map_rgb(255,255,255))
			al.set_target_bitmap(ot)
			pe.picture = pic
		else
			local lw = al.get_bitmap_width(pic)
			local lh = al.get_bitmap_height(pic)
			if lw ~= pe.w or lh ~ pe.h then
				pic = al.create_bitmap(pe.w, pe.h)
				local ot = al.get_target_bitmap()
				al.set_target_bitmap(pic)
				al.clear_to_color(al.map_rgb(255,255,255))
				al.draw_bitmap(pe.picture,0,0,0)
				al.set_target_bitmap(ot)
				al.destroy_bitmap(pe.picture)
				pe.picture = pic
			end
		end
	end

	local iio = imgui.GetIO()
	local mpos = imgui.ImVec2(imgui.GetCursorScreenPos())
	mpos = iio.MousePos - mpos

	if pic then
		if iio:MouseDown(0) then
			if not pe.clicstart then
				pe.clicstart = mpos
			end
			local ot = al.get_target_bitmap()
			al.set_target_bitmap(pic)
			al.draw_line(pe.lmpos.x, pe.lmpos.y, mpos.x, mpos.y, al.map_rgb(0,0,0), 3)
			al.set_target_bitmap(ot)
		elseif pe.clicstart then
			pe.clicstart = nil
		end
	end
	
	pe.lmpos = mpos
	
	if pe.picture then
		imgui.Image(pe.picture, pe.w, pe.h)
	else
		imgui.Text("Set or load a new image")
	end
	
	_,pe.r,pe.g,pe.b = imgui.ColorEdit("Color",pe.r,pe.g,pe.b)
	
	imgui.Text("Mouse: "..mpos.x..", "..mpos.y)
	if pe.clicstart then
		imgui.Text("To: "..pe.clicstart.x..", "..pe.clicstart.y)
	end
	
	return true
end)