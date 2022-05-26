dofile("knotbag.lua")

dofile("console.lua")
dofile("run_command.lua")
dofile("bitmap_aliases.lua")

knotbag.add_window("Demo", function()
	return imgui.ShowDemoWindow(true)
end, false)