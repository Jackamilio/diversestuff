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

--knotbag = {} -- do not reset knotbag, the cpp code code created it and bound stuff on it

--console callback
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
	knotbag.console_string.updated = true
end

--function to create a safe callable script
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
			return function()
				local ret, ret2 = pcall(try)
				if ret then
					return ret2
				else
					print(ret2)
					return false
				end
			end
		end
	else
		print("knotbag.create_script error : wrong arguments")
	end
	return nil
end		

-- add function for general scripts and windows scripts
knotbag.scripts = {}
knotbag.windows = {}

local comp = function(l, r)
	if l.priority ~=nil and r.priority ~= nil then
		if l.priority == r.priority then
			return l.name < r.name
		else
			return l.priority < r.priority
		end
	elseif l.priority ~= nil and r.priority == nil then
		return true
	elseif r.priotity == nil and r.priority ~= nil then
		return false
	else
		return l.name < r.name
	end
end

local set_script = function(t, name, func, priority)
	local try = knotbag.create_script(func)
	if try then
		local ref = t[name]
		if not ref then
			ref = {name = name}
			t[name] = ref
			table.insert(t, ref)
		end
		ref.call = try
		ref.priority = priority
		table.sort(t,comp)
		return true
	end
	return false
end

knotbag.set_script = function(name, func, priority)
	return set_script(knotbag.scripts, name, func, priority)
end

knotbag.set_window = function(name, func, priority, autowindow)
	local ret = set_script(knotbag.windows, name, func, priority)
	if ret then
		knotbag.windows[name].isopen = true
		knotbag.windows[name].autowindow = (autowindow == nil) or autowindow --defaults to true
		return true
	else
		return false
	end
end

-- framescript called by cpp
-- calls everyscript inside knotbag.scripts and keeps them for next frame if it returns true
knotbag.framescript = function()
	for i=1,#knotbag.scripts do
		local s = knotbag.scripts[i]
		s.keepme = s.call()
		if imgui.CleanEndStack() then
			print("/!\\ Aborting script \""..w.name.."\" /!\\ (needed imgui stack cleaning)")
			s.keepme = false
		end
	end
	ArrayRemove(knotbag.scripts, function(t,i) return t[i].keepme end)
end

-- windows script
knotbag.set_script("Windows", function()
	if imgui.BeginMainMenuBar() then
		if imgui.BeginMenu("Windows") then
			local lc_enabled = knotbag.legacy_console();
			if imgui.MenuItem("Legacy console", nil, lc_enabled) then
				knotbag.legacy_console(not lc_enabled)
			end
			imgui.Separator()
			for i=1,#knotbag.windows do
				local w = knotbag.windows[i]
				if imgui.MenuItem(w.name, nil, w.isopen) then
					w.isopen = not w.isopen
				end
			end
			imgui.EndMenu()
		end
		imgui.EndMainMenuBar()
	end
	
	--purposely manually counting i
	--so #knotbag.window is recalculated each loop
	--because deletion can happen inside
	local i=1
	while i <= #knotbag.windows do
		local w = knotbag.windows[i]
		if w.isopen then
			local show, cont = true, true
			if w.autowindow then
				show, cont = imgui.Begin(w.name, true)
			end
			if show then
				w.isopen = w.call()
			end
			if w.autowindow then
				imgui.End()
			end
			if imgui.CleanEndStack() then
				print("/!\\ Closing window \""..w.name.."\" /!\\ (needed imgui stack cleaning)")
				w.isopen = false
			elseif w.isopen then
				w.isopen = cont
			end
		end
		i = i+1
	end
	
	return true
end)

-- utils for the Scripts window
local killselected = function(t)
	for i=1,#t do
		local v = t[i]
		v.keepme = not v.selected
		if v.selected then
			t[v.name] = nil
		end
	end
	ArrayRemove(t, function(t,i,j) return t[i].keepme end)
end

local showscripts = function(n,t)
	if imgui.CollapsingHeader(n) then
		for i=1,#t do
			local v = t[i]
			imgui.PushID(i)
			if imgui.Selectable(v.name, v.selected) then
				v.selected = not v.selected
			end
			imgui.PopID()
		end
	end
end

-- scripts window
knotbag.set_window("Scripts", function()
	if imgui.Button("Terminate selected") then
		killselected(knotbag.scripts)
		killselected(knotbag.windows)
	end
		showscripts("Scripts", knotbag.scripts)
		showscripts("Windows", knotbag.windows)
	return true
end)
