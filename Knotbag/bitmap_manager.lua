if raylib.Load == nil then
	raylib.Load = {
		Texture = function(filename)
			local try = raylib.LoadTexture(filename)
			if try.id ~= 0.0 then
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
