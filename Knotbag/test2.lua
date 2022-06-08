if not raylib.IsAudioDeviceReady() then
	raylib.InitAudioDevice()
end

if not quacksound then
	quacksound = raylib.LoadSound("coin.ogg")
end

raylib.PlaySound(quacksound)