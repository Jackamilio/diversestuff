runcommand = {
	inputvalue = "",
	successtime = -9999,
	successtotal = 0,
	successduration = 1.0,
	windowfunction = function()
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
						return err
					end
				end)
			if not err then
				runcommand.errorstring = nil
				runcommand.successtime = os.clock()
				runcommand.successtotal = runcommand.successtotal + 1
			else
				runcommand.successtime = 0
				local a, b = string.find(err, '"]:%d*: ')
				if b then
					runcommand.errorstring = "Error:" .. err:sub(b)
				else
					runcommand.errorstring = err
				end
				--runcommand.errorstring = err .. "\n" .. #errstr .. "\nError" .. errstr
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
		return true
	end
}

knotbag.set_window("Run command", runcommand.windowfunction, -100)