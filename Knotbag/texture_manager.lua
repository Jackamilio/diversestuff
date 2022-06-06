if raylib.Load == nil then
	raylib.Load = {
		Texture = function(fileORimage)
			local try = nil
			if type(fileORimage) == "string" then
				if fileexists(fileORimage) then
					try = raylib.LoadTexture(fileORimage)
				end
			elseif type(fileORimage) == "userdata" then
				try = raylib.LoadTextureFromImage(fileORimage)
			end
			if try and try.id ~= 0.0 then
				return setmetatable({tex = try}, {
					__gc = function (tex)
						raylib.UnloadTexture(tex.tex)
					end
				})
			else
				return nil
			end
		end
	}
end
