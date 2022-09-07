-- warning : undefined behaviour if this file is ran more than once

local fd = imgui.GetFileDialog()

fd:Open("ChooseProjectDirectory","Choose project", "")

-- function overriden by knotbag.lua in start.lua once the working project is selected
knotbag.framescript = function()
	if fd:IsDone("ChooseProjectDirectory") then
		local dir = nil
		if fd:HasResult() then
			dir = fd:GetResult()
			if dir == "" or dir == "." then
				dir = nil
			end
		end
		fd:Close()
		
		if dir and raylib.ChangeDirectory(dir) then
			raylib.SetWindowTitle("KNOTBAG - "..dir)
			
			imgui.LoadIniSettingsFromDisk("imgui.ini")
			pdofile(appdir.."\\start.lua")
			
			if fileexists("auto.lua") then
				pdofile("auto.lua")
			end
		else
			raylib.SetWindowTitle("KNOTBAG - /!\\ No project set /!\\")
			pdofile("start.lua")
		end
	end
end