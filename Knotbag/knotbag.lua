--https://stackoverflow.com/questions/12394841/safely-remove-items-from-an-array-table-while-iterating
function ArrayRemove(t, fnKeep)
    local j, n = 1, #t;

    for i=1,n do
        if (fnKeep(t, i, j)) then
            -- Move i's kept value to j's position, if it's not already there.
            if (i ~= j) then
                t[j] = t[i];
                t[i] = nil;
            end
            j = j + 1; -- Increment position of where we'll place the next kept value.
        else
            t[i] = nil;
        end
    end

    return t;
end

knotbag = {}
knotbag.create_script = function(f)
	local tf = type(f)
	if tf == "function" or tf == "string" then
		local try = nil
		if type(f) == "function" then
			try = f
		elseif type(f) == "string" and fileexists(script) then
			local ret, err = loadfile(script)
			if ret then
				try = ret
			else
				print(err)
			end
		end
		if try then
			return {call = function()
				local ret, ret2 = pcall(try)
				if ret then
					return ret
				else
					print(ret2)
					return false
				end
			end}
		end
	else
		print("knotbag.create_script error : wrong arguments")
	end
	return nil
end		

knotbag.scripts = {}
knotbag.windows = {}

knotbag.add_script = function(name, func)
	local try = knotbag.create_script(func)
	if try then
		knotbag.scripts[name] = try
		return true
	end
	return false
end

knotbag.add_window = function(name, func, autowindow)
	local try = knotbag.create_script(func)
	if try then
		try.isopen = true
		try.autowindow = (autowindow == nil) or autowindow --default true
		knotbag.windows[name] = try
		return true
	end
	return false
end

knotbag.framescript = function()
	for k,v in pairs(knotbag.scripts) do
		v.keepme = v.call()
		if imgui.CleanEndStack() then
			print("/!\\ Aborting script \""..k.."\" because it needed imgui stack cleaning.")
			v.keepme = false
		end
	end
	ArrayRemove(knotbag.scripts, function(t,i,j)
		return t[i].keepme
	end)
end

-- windows script
knotbag.add_script("Windows", function()
	if imgui.BeginMainMenuBar() then
		if imgui.BeginMenu("Windows") then
			for k,v in pairs(knotbag.windows) do
				if imgui.MenuItem(k, nil, v.isopen) then
					v.isopen = not v.isopen
				end
			end
			imgui.EndMenu()
		end
		imgui.EndMainMenuBar()
	end
	
	for k,v in pairs(knotbag.windows) do
		if v.isopen then
			local show, cont = true, true
			if v.autowindow then
				show, cont = imgui.Begin(k, true)
			end
			if show then
				v.isopen = v.call()
			end
			if v.autowindow then
				imgui.End()
			end
			if v.isopen then v.isopen = cont end
			if imgui.CleanEndStack() then
				print("/!\\ Closing window \""..k.."\" because it needed imgui stack cleaning.")
				v.isopen = false
			end
		end
	end
	
	return true
end)

-- scripts window
knotbag.add_window("Scripts", function()
	if imgui.Button("Kill selected") then
		ArrayRemove(knotbag.scripts, function(t,i,j)
			return not t[i].selected
		end)
	end
	
	local i = 0
	for k,v in pairs(knotbag.scripts) do
		imgui.PushID(i)
		i = i + 1
		if imgui.Selectable(k, v.selected) then
			v.selected = not v.selected
		end
		imgui.PopID()
	end
	return true
end)
