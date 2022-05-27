dofile("scan_all_context.lua")
dofile("table_save.lua")

dofile("knotbag.lua")

tablescanning.registerwindow()

dofile("run_command.lua")
dofile("bitmap_aliases.lua")

knotbag.set_window("Demo", function()
	return imgui.ShowDemoWindow(true)
end, nil, false)

dofile("console.lua")