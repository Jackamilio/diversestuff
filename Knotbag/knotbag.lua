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

knotbag = {
	windows = {},
	add_window = function(n, f)
		if type(n) == "string" and type(f) == "function" then
			table.insert(knotbag.windows, {name = n, func = f, isopen = true})
		else
			print("add_window error : wrong arguments")
		end
	end,
	scripts = {},
	add_script = function(n, s, sel)
		local tn = type(n)
		local ts = type(s)
		if tn == "string" and (ts == "function" or ts == "string") then
			table.insert(knotbag.scripts, {name = n, script = s, selected = sel or false})
		else
			print("add_script error : wrong arguments")
		end
	end,
	framescript = function()
		for t,v in pairs(knotbag.scripts) do
			local func = nil
			local script = v.script
			if type(script) == "function" then
				func = script
			elseif type(script) == "string" and fileexists(script) then
				local ret, err = loadfile(script)
				if ret then
					func = ret
				else
					print(err)
				end
			end
			
			if func then
				local ret, ret2 = pcall(func)
				if ret then
					v.keepme = (ret2 == true)
					if imgui.CleanEndStack() then
						print("Warning! Script \""..v.name.."\" needed imgui stack cleaning.")
					end
				else
					imgui.CleanEndStack()
					print(ret2)
				end
			end
		end
		ArrayRemove(knotbag.scripts, function(t,i,j)
			return t[i].keepme
		end)
	end
}

-- windows script
knotbag.add_script("Windows", function()
	if imgui.BeginMainMenuBar() then
		if imgui.BeginMenu("Windows") then
			for t,v in pairs(knotbag.windows) do
				if imgui.MenuItem(v.name, nil, v.isopen) then
					v.isopen = not v.isopen
				end
			end
			imgui.EndMenu()
		end
		imgui.EndMainMenuBar()
	end
	
	for t,v in pairs(knotbag.windows) do
		if v.isopen then
			local ret, ret2 = pcall(v.func)
			if ret then
				v.isopen = (ret2 == true)
				if imgui.CleanEndStack() then
					print("Warning! Window \""..v.name.."\" needed imgui stack cleaning.")
				end
			else
				imgui.CleanEndStack()
				print(ret2)
			end
		end
	end
	
	return true
end)

-- scripts window
knotbag.add_window("Scripts", function()
	local show, cont = imgui.Begin("Scripts", true)
	if show then
		if imgui.Button("Kill selected") then
			ArrayRemove(knotbag.scripts, function(t,i,j)
				return not t[i].selected
			end)
		end
		
		local i = 0
		for t,v in pairs(knotbag.scripts) do
			imgui.PushID(i)
			i = i + 1
			if imgui.Selectable(v.name, v.selected) then
				v.selected = not v.selected
			end
			imgui.PopID()
		end
	end
	imgui.End()
	return cont
end)
