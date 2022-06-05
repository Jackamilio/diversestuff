if tablescanning == nil then
	
	tablescanning = {}
	
	tablescanning.doit = function(t, store)
		local n,v
		for n,v in pairs(t) do
			if n ~= "loaded" and n ~= "_G" then
				table.insert(store, n)
	
				if type(v) == "table" then
					store[n] = {}
					tablescanning.doit(v, store[n])
				end
			end
		end
		table.sort(store)
	end
	
	tablescanning.print_result = function(res, offset)
		local n, v
		for n, v in pairs(res) do
			if n ~= "loaded" and n ~= "_G" then
				if type(v) == "table" then
					print(offset .. n)
					tablescanning.print_result(v, offset .. "  ")
				else
					print(offset .. v)
				end
			end
		end
	end
	
	tablescanning.show_result = function(res)
		local n, v
		for n, v in pairs(res) do
			if n ~= "loaded" and n ~= "_G" then
				if type(v) == "table" then
					if imgui.TreeNode(n) then
						tablescanning.show_result(v)
						imgui.TreePop()
					end
				else
					imgui.TextUnformatted(v)
				end
			end
		end
	end
	
	tablescanning.result = {}
	tablescanning.doit(raylib, tablescanning.result)
	
	tablescanning.registerwindow = function()
		knotbag.set_window("Scanned lua context", function()
			tablescanning.show_result(tablescanning.result)
			return true
		end)
	end

end