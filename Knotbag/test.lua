#~

function ImGuiTest()
	cont = true
	
	show, cont = imgui.Begin("Hello", cont)
	if show then
		imgui.TextUnformatted("Boop bip bap bup")
		if imgui.SmallButton("Test") then
			cont = false
		end
	end
	imgui.End()
	
	return cont
end

function TestCoroutine()
	local i = 0
	for i = 0,10 do
		print(i)
		coroutine.yield()
	end
end

if testcoroutine == nil then
	testcoroutine = coroutine.create(TestCoroutine)
end

coroutine.resume(testcoroutine)

if coroutine.status(testcoroutine) == "dead" then
	testcoroutine = nil
end

return (testcoroutine ~= nil)