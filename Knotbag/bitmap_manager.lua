if al.bitmap == nil then
	al.bitmap = {
		load = function(filename)
			local try = al.load_bitmap(filename)
			if try then
				return setmetatable({bmp = try}, {
					__gc = function (bmp)
						al.destroy_bitmap(bmp.bmp)
					end
				})
			else
				return nil
			end
		end,
	
		draw = function(bmp, x, y, flag)
			al.draw_bitmap(bmp.bmp,x,y,flag or 0)
		end
	}
end




local test = al.bitmap.load("errortexture.png")
al.bitmap.draw(test, 0, 0)

return true
