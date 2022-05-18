if (eventQueue == nil) then
	eventQueue = al.create_event_queue()
	al.register_event_source(eventQueue, al.get_keyboard_event_source())

	print("Initialized queue")
end

local ev = al.ALLEGRO_EVENT()
while al.get_next_event(eventQueue, ev) do
	if ev.type == al.ALLEGRO_EVENT_KEY_CHAR then
		print("bip " .. string.char(ev.keyboard.unichar))
	end
end

return true
