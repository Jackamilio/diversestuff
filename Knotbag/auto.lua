appdir = raylib.GetWorkingDirectory()

function pdofile(filename) --WHY WOULD DESTINY GENERATE THIS NAME
	if not fileexists(filename) then
		filename = appdir.."\\"..filename
	end
	
	local ret, err = pcall(dofile, filename)
	if not ret then
		print(err)
	end
end

pdofile("choose_project_dir.lua")
--pdofile("start.lua")