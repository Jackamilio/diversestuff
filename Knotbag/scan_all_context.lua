function scan_table(t, store)
	local n,v
	for n,v in pairs(t) do
		if n ~= "loaded" and n ~= "_G" then
			table.insert(store, n)

			if type(v) == "table" then
				store[n] = {}
				scan_table(v, store[n])
			end
		end
	end
	table.sort(store)
end

function print_result(res, offset)
	local n, v
	for n, v in pairs(res) do
		if n ~= "loaded" and n ~= "_G" then
			if type(v) == "table" then
				print(offset .. n)
				print_result(v, offset .. "  ")
			else
				print(offset .. v)
			end
		end
	end
end

function show_result(res)
	local n, v
	for n, v in pairs(res) do
		if n ~= "loaded" and n ~= "_G" then
			if type(v) == "table" then
				if imgui.TreeNode(n) then
					show_result(v)
					imgui.TreePop()
				end
			else
				imgui.TextUnformatted(v)
			end
		end
	end
end

if scanned_context == nil then
	local res = {}
	scan_table(_G, res)
	scanned_context = res
end

--print_result(scanned_context, "")


local show, cont = imgui.Begin("Scanned lua", true)
if show then
	show_result(scanned_context)
end
imgui.End()


return cont