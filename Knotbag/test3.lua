local str = '[string "zut.flute = mince"]:12345: attempt blablaba chut'

local f,g = str:find('"]:%d*: ')

if f then
print( str:sub(g) )
else
print("no result")
end