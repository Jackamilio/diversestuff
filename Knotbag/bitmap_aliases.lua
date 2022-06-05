dofile("bitmap_manager.lua")

if bitmaps == nil then
	bitmaps = {
		{name = "errortexture", file = "errortexture.png"},
		{name = "failtex", file = "nofileherelol.png"},
		{name = "banana", file = "banana.png"},
		{name = "monkey", file = "cutemonkey.png"}
	}

	setmetatable(bitmaps, { __index = function(tb,key)
		if type(key) == "string" then
			for t, v in pairs(tb) do
				if type(t) == "number" and v.name == key then
					local bmp = raylib.Load.Texture(v.file)
					if bmp then
						rawset(tb, key, bmp)
						return bmp
					end
					break
				end
			end
			if key == "errortexture" then
				if not rawget(tb, "__generated_error_texture__") then
					local gen = raylib.GenImageChecked(256,256,32,32,raylib.GetColor(0xFFFF00FF),raylib.GetColor(0x111111FF))
					raylib.ImageDrawText(gen, "NOT", 60, 70, 50, raylib.GetColor(0xFFFFFFFF))
					raylib.ImageDrawText(gen, "FOUND", 2, 140, 50, raylib.GetColor(0xFFFFFFFF))
					local newtex = raylib.Load.Texture(gen)
					rawset(tb, "__generated_error_texture__", newtex)
					raylib.UnloadImage(gen)
				end
				return tb.__generated_error_texture__
			end
			return tb.errortexture
		end
		return nil
	end})

	bitmaps.__editor_values__ = { current = -1, elemwidth = 100 }
end

knotbag.set_window("Bitmap aliases", function()
	local fd = imgui.GetFileDialog()
	local ed = bitmaps.__editor_values__
	local fieldtodel = -1
	if imgui.BeginTable("Bitmaps table", math.max(imgui.GetWindowWidth() / ed.elemwidth, 1), imgui.constant.WindowFlags.NoSavedSettings) then
		for t, v in pairs(bitmaps) do
			if type(t) == "number" then
				imgui.TableNextColumn()
				if ed.current == t then --and v.name ~= "errortexture" then
					if imgui.SmallButton("X") then
						fieldtodel = t
					end
					imgui.SameLine()
				end
				imgui.PushID(t)
				local ret = imgui.Selectable(v.name, t == ed.current)
				imgui.PopID()
				if ret then
					ed.current = t
				end
			end
		end
		imgui.EndTable()
	end

	if fieldtodel >= 0 then
		bitmaps[bitmaps[fieldtodel].name] = nil
		table.remove(bitmaps, fieldtodel)
	end

	if (imgui.Button("New alias")) then
		table.insert(bitmaps, {name = "new texture", file = "no file yet"})
	end

	imgui.Separator()
	local curbmp = bitmaps[ed.current]
	if curbmp ~= nil then
		local change, newname = imgui.InputText("Name", curbmp.name, 128)
		if change then
			local oldbmp = bitmaps[curbmp.name]
			bitmaps[curbmp.name] = nil
			bitmaps[newname] = oldbmp
			curbmp.name = newname
		end
		imgui.Text(curbmp.file)
		imgui.SameLine()
		if imgui.Button("...") then
			fd:Open("LoadTextureForAlias", "Open a file", "Image (*.bmp,*.png){.bmp,.png},.*")
		end
		
		local tex = bitmaps[curbmp.name]
		if tex then
			tex = tex.tex
			local w = tex.width
			local h = tex.height
			local ww = imgui.GetWindowWidth() - 20
			if w > ww then
				h = h * ww / w
				w = ww
			end
			imgui.Image(tex, w, h)
		end
	
		if fd:IsDone("LoadTextureForAlias") then
			if fd:HasResult() then
				local newfile = fd:GetResult()
				if curbmp.file ~= newfile then
					curbmp.file = newfile
					bitmaps[curbmp.name] = nil --force reload
				end
			end
			fd:Close()
		end
	end
	return true
end)