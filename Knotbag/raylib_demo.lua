--knotbag port of first person_maze in https://www.raylib.com/examples.html
--orignal cpp code:
--https://github.com/raysan5/raylib/blob/master/examples/models/models_first_person_maze.c

local rl = raylib


-- Define the camera to look into our 3d world
local camera = rl.Camera3D()
camera.position.x = 0.2
camera.position.y = 0.4
camera.position.z = 0.2
camera.up.y = 1.0
camera.fovy = 45.0

local imMap = rl.LoadImage("cubicmap.png")
local cubicmap = rl.LoadTextureFromImage(imMap)
local vec = rl.Vector3() vec.x = 1 vec.y = 1 vec.z = 1
local mesh = rl.GenMeshCubicmap(imMap, vec)
local model = rl.LoadModelFromMesh(mesh)

-- NOTE: By default each cube is mapped to one part of texture atlas
local texture = rl.LoadTexture("cubicmap_atlas.png")
local mat = rl.arrayat(model.materials, 0)
local map = rl.arrayat(mat.maps, rl.MATERIAL_MAP_ALBEDO)
map.texture = texture

local mapPixels = rl.LoadImageColors(imMap)
rl.UnloadImage(imMap)

local mapPosition = rl.Vector3()
mapPosition.x = -16
mapPosition.y = 0
mapPosition.z = -8

--rl.SetCameraMode(camera, rl.CAMERA_FIRST_PERSON)

local function raylib_demo()
	local oldCamPos = rl.Vector3()
	oldCamPos.x = camera.position.x
	oldCamPos.y = camera.position.y
	oldCamPos.z = camera.position.z
	
	rl.UpdateCamera(camera)
	
	local playerPos = rl.Vector2()
	playerPos.x = camera.position.x
	playerPos.y = camera.position.z
	local playerRadius = 0.1
	
	local playerCellX = math.floor(playerPos.x - mapPosition.x + 0.5)
	local playerCellY = math.floor(playerPos.y - mapPosition.z + 0.5)
	
	if playerCellX < 0 then playerCellX = 0
	elseif playerCellX >= cubicmap.width then playerCellX = cubicmap.width - 1 end
	
	if playerCellY < 0 then playerCellY = 0
	elseif playerCellY >= cubicmap.height then playerCellX = cubicmap.height - 1 end
	
	local rect = rl.Rectangle()
	for y=0,cubicmap.height-1 do
		for x=0,cubicmap.width-1 do
			rect.x = mapPosition.x - 0.5 + x
			rect.y = mapPosition.z - 0.5 + y
			rect.width = 1.0
			rect.height = 1.0
			local pixel = rl.arrayat(mapPixels, y*cubicmap.width + x)
			if pixel.r == 255
			and rl.CheckCollisionCircleRec(playerPos, playerRadius, rect) then
				camera.position = oldCamPos
			end
		end
	end
	
	rl.BeginMode3D(camera)
	rl.DrawModel(model, mapPosition, 1.0, rl.WHITE)
	rl.EndMode3D()
	
	local pos = rl.Vector2()
	pos.x = rl.GetScreenWidth() - cubicmap.width * 4.0 - 20
	pos.y = 20
	rl.DrawTextureEx(cubicmap, pos, 0, 4, rl.WHITE)
	rl.DrawRectangleLines(pos.x, pos.y, cubicmap.width*4, cubicmap.height*4, rl.GREEN)
	
	rl.DrawRectangle(pos.x + playerCellX * 4, pos.y + playerCellY*4, 4, 4, rl.RED)
	
	rl.DrawFPS(10,50)
	
	return true
end

knotbag.set_script("Raylib demo", raylib_demo)