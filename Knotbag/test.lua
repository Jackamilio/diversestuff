cont = true;

show, cont = imgui.Begin("Hello", cont)
if show then
	imgui.TextUnformatted("Boop bip bap bup")
	if imgui.SmallButton("Test") then
		cont = false
	end
end
imgui.End()

return cont