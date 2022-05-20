s = "salut : :       :hoy"

local res = string.match(s, ":[^:]*$")

print(res)