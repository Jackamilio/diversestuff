if knotbag.console_callback == nil then
	--trying to optimize a bit
	knotbag.console_string = {
		bits = {},
		full = "",
		needsconcat = false,
		clear = function()
			knotbag.console_string.bits = {}
			knotbag.console_string.full = ""
			knotbag.console_string.needsconcat = false
		end
	}
	knotbag.console_string.clear()
	setmetatable(knotbag.console_string, {__tostring = function(v)
		if v.needsconcat then
			v.needsconcat = false
			v.full = table.concat(v.bits)
			v.bits = {v.full}
		end
		return v.full
	end})
	knotbag.console_callback = function (s)
		table.insert(knotbag.console_string.bits, s)
		knotbag.console_string.needsconcat = true
	end
	
	knotbag.add_window("Legacy console clone", function()
		local show, cont = imgui.Begin("Legacy console clone", true)
		if show then
			if imgui.Button("Clear") then
				knotbag.console_string.clear()
			end
			imgui.TextUnformatted(tostring(knotbag.console_string))
		end
		imgui.End()
		return cont
	end)
end