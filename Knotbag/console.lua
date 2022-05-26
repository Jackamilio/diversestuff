knotbag.legacy_console(false)
knotbag.set_window("Console", function()
	if imgui.Button("Clear") then
		knotbag.console_string.clear()
	end
	imgui.TextUnformatted(tostring(knotbag.console_string))
	if knotbag.console_string.updated then
		knotbag.console_string.updated = false
		imgui.SetScrollHereY(1.0)
	end
	return true
end)
