dofile("texture_manager.lua")

if textures == nil then
	textures = {
		{name = "errortexture", file = "errortexture.png"},
		{name = "failtex", file = "nofileherelol.png"},
		{name = "banana", file = "banana.png"},
		{name = "monkey", file = "cutemonkey.png"}
	}

	setmetatable(textures, { __index = function(tb,key)
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
					local gen = raylib.GenImageChecked(256,256,32,32,raylib.YELLOW,raylib.DARKGRAY)
					raylib.ImageDrawText(gen, "NOT", 60, 70, 50, raylib.WHITE)
					raylib.ImageDrawText(gen, "FOUND", 2, 140, 50, raylib.WHITE)
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

	textures.__editor_values__ = { current = -1, elemwidth = 100 }
end

knotbag.set_window("Texture aliases", function()
	local fd = imgui.GetFileDialog()
	local ed = textures.__editor_values__
	local fieldtodel = -1
	if imgui.BeginTable("Textures table", math.max(imgui.GetWindowWidth() / ed.elemwidth, 1), imgui.constant.WindowFlags.NoSavedSettings) then
		for t, v in pairs(textures) do
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
		textures[textures[fieldtodel].name] = nil
		table.remove(textures, fieldtodel)
	end

	if (imgui.Button("New alias")) then
		table.insert(textures, {name = "new texture", file = "no file yet"})
	end

	imgui.Separator()
	local curtex = textures[ed.current]
	if curtex ~= nil then
		local change, newname = imgui.InputText("Name", curtex.name, 128)
		if change then
			local oldtex = textures[curtex.name]
			textures[curtex.name] = nil
			textures[newname] = oldtex
			curtex.name = newname
		end
		imgui.Text(curtex.file)
		imgui.SameLine()
		if imgui.Button("...") then
			fd:Open("LoadTextureForAlias", "Open a file", "Image (*.bmp,*.png){.bmp,.png},.*")
		end
		
		local tex = textures[curtex.name]
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
				if curtex.file ~= newfile then
					curtex.file = newfile
					textures[curtex.name] = nil --force reload
				end
			end
			fd:Close()
		end
	end
	return true
end)