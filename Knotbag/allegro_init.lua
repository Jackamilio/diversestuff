local function mergetables(a, b)
	for k,v in pairs(b) do
		a[k] = v
	end
end

al = lallegro_core

mergetables(al, lallegro_primitives)

lallegro_core = nil
lallegro_primitives = nil