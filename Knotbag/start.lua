dofile("knotbag.lua")
knotbag.add_window("Demo", function()
	return imgui.ShowDemoWindow(true)
end)

dofile("run_command.lua")
dofile("bitmap_aliases.lua")