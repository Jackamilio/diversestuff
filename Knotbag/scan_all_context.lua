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

function show_result(offset, res)
	local n, v
	for n, v in pairs(res) do
		if n ~= "loaded" and n ~= "_G" then
			if type(v) == "table" then
				print(offset .. n)
				show_result(offset .. "  ", res[n])
			else
				print(offset .. v)
			end
		end
	end
end

local test = {}
scan_table(_G, test)
show_result("", test)