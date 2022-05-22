function fileexists(name)
	local f=io.open(name,"r")
	if f~=nil then
		io.close(f)
		return true
	else
		return false
	end
end

knotbag = {
	windows = {},
	add_window = function(f,n)
		if type(f) == "function" and type(n) == "string" then
			knotbag.windows[n] = {func = f, isopen = true}
		else
			print("add_window error : wrong arguments")
		end
	end
}

if fileexists("start.lua") then
	dofile("start.lua")
end