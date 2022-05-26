dofile("knotbag.lua")

dofile("run_command.lua")
dofile("bitmap_aliases.lua")

knotbag.set_window("Demo", function()
	return imgui.ShowDemoWindow(true)
end, false)

dofile("console.lua")