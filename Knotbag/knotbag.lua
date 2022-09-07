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

--util to load settings
local function tryload(filename)
	if fileexists(filename) then
		local err = nil
		local t, err = table.load(filename)
		if err then
			print(err)
		else
			return t
		end
	end
	return nil
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
knotbag.windows = tryload("openedwindows.lua") or {}

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
		local w = knotbag.windows[name]
		w.isopen = (w.isopen == nil) or w.isopen --open window on first time only
		w.autowindow = (autowindow == nil) or autowindow --defaults to true
		return true
	else
		return false
	end
end

-- docking management
knotbag.docking = {findfreeID = function()
	local id=1
	while true do
		if knotbag.docking.docks[id] == nil then
			return id
		end
		id = id + 1
	end
end}

knotbag.docking.docks = tryload("docks.lua") or {}

-- windows script
knotbag.set_script("Windows menu", function()
	local clicked_add_docking = false
	if imgui.BeginMainMenuBar() then
		if imgui.BeginMenu("Windows") then
			clicked_add_docking = imgui.MenuItem("Add docking")
	
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
	
	if clicked_add_docking then
		imgui.OpenPopup("Add docking module")
		knotbag.docking.inputstate = ""
		knotbag.docking.popupstate = true
	end
	
	local dk = knotbag.docking
	local show, cont = imgui.BeginPopupModal("Add docking module", dk.popupstate)
	dk.popupstate = cont
	if show then
		local change, newname = imgui.InputText("##add_docking_module", dk.inputvalue or "", 128)
		if change then dk.inputvalue = newname end
		imgui.SameLine()
		local run = imgui.IsWindowFocused() and imgui.IsKeyPressed(imgui.constant.Key.Enter)
		if imgui.Button("Add") or run then
			dk.docks[dk.findfreeID()] = dk.inputvalue
			imgui.CloseCurrentPopup()
		end
		imgui.EndPopup()
	end
	
	return true
end)

knotbag.set_script("Docking windows", function()
	for k,v in pairs(knotbag.docking.docks) do
		imgui.PushStyleVar_2(imgui.constant.StyleVar.WindowPadding, 0, 0)
		local show, cont = imgui.Begin(v.."###dockspace_"..k, true)
		imgui.DockSpace(k)--THIS: imgui.GetID("dockspace_no_"..k)) gave me an assert telling I can't call DockSpace twice in a frame with the same ID. wtf.
		imgui.End()
		imgui.PopStyleVar()
		if not cont then
			knotbag.docking.docks[k] = nil
		end
	end
	return true
end, -math.huge) --docking must be done first

knotbag.set_script("Windows", function()
	--purposely manually counting i
	--so #knotbag.window is recalculated each loop
	--because deletion can happen inside
	local i=1
	while i <= #knotbag.windows do
		local w = knotbag.windows[i]
		if w.call then
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
		else
			-- isopen was probably saved from an earlier session
			-- but the function was never defined again
			-- let's remove this entry to prevent crashing this script
			knotbag.windows[w.name] = nil
			table.remove(knotbag.windows, i)
		end
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
		if imgui.CollapsingHeader("Docks") then
			local docks = knotbag.docking.docks
			for k,v in pairs(docks) do
				local change, newstr = imgui.InputText("ID "..k,v,128)
				if change then docks[k] = newstr end
			end
		end
	return true
end)


-- CPP CALLBACKS DEFINITIONS --

-- framescript called by cpp
-- calls everyscript inside knotbag.scripts and keeps them for next frame if it returns true
knotbag.framescript = function()
	for i=1,#knotbag.scripts do
		local s = knotbag.scripts[i]
		--print("Script "..s.name)
		s.keepme = s.call()
		if imgui.CleanEndStack() then
			print("/!\\ Aborting script \""..s.name.."\" /!\\ (needed imgui stack cleaning)")
			s.keepme = false
		end
	end
	ArrayRemove(knotbag.scripts, function(t,i) return t[i].keepme end)
end

-- saving settings on quit
knotbag.quit = function()
	table.save(knotbag.docking.docks, "docks.lua")
	--save opened windows
	local savewin = {}
	for i=1,#knotbag.windows do
		--only save name and open state
		local w = knotbag.windows[i]
		local s = {name = w.name, isopen = w.isopen}
		savewin[i] = s
		savewin[s.name] = s
	end
	table.save(savewin, "openedwindows.lua")
end
