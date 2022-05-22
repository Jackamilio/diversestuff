runcommand = {
	inputvalue = "",
	successtime = -9999,
	successtotal = 0,
	successduration = 1.0,
	windowfunction = function()
		local show, cont = imgui.Begin("Run command", true)
		if show then
		
		runcommand.inputvalue = imgui.InputText("##run_command_window", runcommand.inputvalue)
		imgui.SameLine()
		
		local run = imgui.IsWindowFocused() and imgui.IsKeyPressed(imgui.constant.Key.Enter)
		
		if imgui.Button("Run") or run then
			local ret, err = pcall(
				function()
					local ret, err = load(runcommand.inputvalue)
					if ret then
						ret()
					else
						error(err)
					end
				end)
			if ret then
				runcommand.errorstring = nil
				runcommand.successtime = os.clock()
				runcommand.successtotal = runcommand.successtotal + 1
			else
				runcommand.successtime = 0
				local errstr = string.match(err, ":[^:]*$")
				if not errstr then errstr = ": " .. err end
				runcommand.errorstring = "Error" .. errstr
			end
		end
		
		if os.clock() - runcommand.successtime < runcommand.successduration then
			local conc = ""
			if runcommand.successtotal > 1 then
				conc = " x" .. runcommand.successtotal
			end
			imgui.Text("Success" .. conc)
		else
			runcommand.successtotal = 0
		end
		
		if runcommand.errorstring then
			imgui.Text(runcommand.errorstring)
			if imgui.Button("ok") then
				runcommand.errorstring = nil
			end
		end
		
		end
		
		imgui.End()
		
		return cont
	end
}

knotbag.add_window("Run command", runcommand.windowfunction)