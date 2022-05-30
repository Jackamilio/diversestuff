function pdofile(filename) --WHY WOULD DESTINY GENERATE THIS NAME
	local ret, err = pcall(dofile, filename)
	if not ret then
		print(err)
	end
end

pdofile("allegro_init.lua")

pdofile("scan_all_context.lua")
pdofile("table_save.lua")

pdofile("knotbag.lua")

tablescanning.registerwindow()

pdofile("run_command.lua")
pdofile("bitmap_aliases.lua")

knotbag.set_window("Demo", function()
	return imgui.ShowDemoWindow(true)
end, nil, false)

pdofile("console.lua")