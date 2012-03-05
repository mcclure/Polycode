require "Polycode/EventDispatcher"

class "Scene" (EventDispatcher)



ENTITY_MESH = 0
ENTITY_LIGHT = 1
ENTITY_CAMERA = 2
ENTITY_ENTITY = 3
ENTITY_COLLMESH = 4
function Scene:__index__(name)
	if name == "clearColor" then
		retVal = Polycore.Scene_get_clearColor(self.__ptr)
		if Polycore.__ptr_lookup[retVal] ~= nil then
			return Polycore.__ptr_lookup[retVal]
		else
			Polycore.__ptr_lookup[retVal] = Color("__skip_ptr__")
			Polycore.__ptr_lookup[retVal].__ptr = retVal
			return Polycore.__ptr_lookup[retVal]
		end
	elseif name == "useClearColor" then
		return Polycore.Scene_get_useClearColor(self.__ptr)
	elseif name == "ambientColor" then
		retVal = Polycore.Scene_get_ambientColor(self.__ptr)
		if Polycore.__ptr_lookup[retVal] ~= nil then
			return Polycore.__ptr_lookup[retVal]
		else
			Polycore.__ptr_lookup[retVal] = Color("__skip_ptr__")
			Polycore.__ptr_lookup[retVal].__ptr = retVal
			return Polycore.__ptr_lookup[retVal]
		end
	elseif name == "fogColor" then
		retVal = Polycore.Scene_get_fogColor(self.__ptr)
		if Polycore.__ptr_lookup[retVal] ~= nil then
			return Polycore.__ptr_lookup[retVal]
		else
			Polycore.__ptr_lookup[retVal] = Color("__skip_ptr__")
			Polycore.__ptr_lookup[retVal].__ptr = retVal
			return Polycore.__ptr_lookup[retVal]
		end
	elseif name == "enabled" then
		return Polycore.Scene_get_enabled(self.__ptr)
	elseif name == "ownsChildren" then
		return Polycore.Scene_get_ownsChildren(self.__ptr)
	elseif name == "ownsCamera" then
		return Polycore.Scene_get_ownsCamera(self.__ptr)
	end
end


function Scene:__set_callback(name,value)
	if name == "useClearColor" then
		Polycore.Scene_set_useClearColor(self.__ptr, value)
		return true
	elseif name == "enabled" then
		Polycore.Scene_set_enabled(self.__ptr, value)
		return true
	elseif name == "ownsChildren" then
		Polycore.Scene_set_ownsChildren(self.__ptr, value)
		return true
	elseif name == "ownsCamera" then
		Polycore.Scene_set_ownsCamera(self.__ptr, value)
		return true
	end
	return false
end


function Scene:Scene(...)
	if type(arg[1]) == "table" and count(arg) == 1 then
		if ""..arg[1]:class() == "EventDispatcher" then
			self.__ptr = arg[1].__ptr
			return
		end
	end
	for k,v in pairs(arg) do
		if type(v) == "table" then
			if v.__ptr ~= nil then
				arg[k] = v.__ptr
			end
		end
	end
	if self.__ptr == nil and arg[1] ~= "__skip_ptr__" then
		self.__ptr = Polycore.Scene(unpack(arg))
		Polycore.__ptr_lookup[self.__ptr] = self
	end
end

function Scene:addEntity(entity)
	local retVal = Polycore.Scene_addEntity(self.__ptr, entity.__ptr)
end

function Scene:removeEntity(entity)
	local retVal = Polycore.Scene_removeEntity(self.__ptr, entity.__ptr)
end

function Scene:getDefaultCamera()
	local retVal =  Polycore.Scene_getDefaultCamera(self.__ptr)
	if retVal == nil then return nil end
	if Polycore.__ptr_lookup[retVal] ~= nil then
		return Polycore.__ptr_lookup[retVal]
	else
		Polycore.__ptr_lookup[retVal] = Camera("__skip_ptr__")
		Polycore.__ptr_lookup[retVal].__ptr = retVal
		return Polycore.__ptr_lookup[retVal]
	end
end

function Scene:getActiveCamera()
	local retVal =  Polycore.Scene_getActiveCamera(self.__ptr)
	if retVal == nil then return nil end
	if Polycore.__ptr_lookup[retVal] ~= nil then
		return Polycore.__ptr_lookup[retVal]
	else
		Polycore.__ptr_lookup[retVal] = Camera("__skip_ptr__")
		Polycore.__ptr_lookup[retVal].__ptr = retVal
		return Polycore.__ptr_lookup[retVal]
	end
end

function Scene:setActiveCamera(camera)
	local retVal = Polycore.Scene_setActiveCamera(self.__ptr, camera.__ptr)
end

function Scene:enableLighting(enable)
	local retVal = Polycore.Scene_enableLighting(self.__ptr, enable)
end

function Scene:enableFog(enable)
	local retVal = Polycore.Scene_enableFog(self.__ptr, enable)
end

function Scene:setFogProperties(fogMode, color, density, startDepth, _endDepth)
	local retVal = Polycore.Scene_setFogProperties(self.__ptr, fogMode, color.__ptr, density, startDepth, _endDepth)
end

function Scene:Update()
	local retVal =  Polycore.Scene_Update(self.__ptr)
end

function Scene:setVirtual(val)
	local retVal = Polycore.Scene_setVirtual(self.__ptr, val)
end

function Scene:isVirtual()
	local retVal =  Polycore.Scene_isVirtual(self.__ptr)
	return retVal
end

function Scene:isEnabled()
	local retVal =  Polycore.Scene_isEnabled(self.__ptr)
	return retVal
end

function Scene:setEnabled(enabled)
	local retVal = Polycore.Scene_setEnabled(self.__ptr, enabled)
end

function Scene:getNumEntities()
	local retVal =  Polycore.Scene_getNumEntities(self.__ptr)
	return retVal
end

function Scene:getEntity(index)
	local retVal = Polycore.Scene_getEntity(self.__ptr, index)
	if retVal == nil then return nil end
	if Polycore.__ptr_lookup[retVal] ~= nil then
		return Polycore.__ptr_lookup[retVal]
	else
		Polycore.__ptr_lookup[retVal] = SceneEntity("__skip_ptr__")
		Polycore.__ptr_lookup[retVal].__ptr = retVal
		return Polycore.__ptr_lookup[retVal]
	end
end

function Scene:getEntityAtScreenPosition(x, y)
	local retVal = Polycore.Scene_getEntityAtScreenPosition(self.__ptr, x, y)
	if retVal == nil then return nil end
	if Polycore.__ptr_lookup[retVal] ~= nil then
		return Polycore.__ptr_lookup[retVal]
	else
		Polycore.__ptr_lookup[retVal] = SceneEntity("__skip_ptr__")
		Polycore.__ptr_lookup[retVal].__ptr = retVal
		return Polycore.__ptr_lookup[retVal]
	end
end

function Scene:Render(targetCamera)
	local retVal = Polycore.Scene_Render(self.__ptr, targetCamera.__ptr)
end

function Scene:RenderDepthOnly(targetCamera)
	local retVal = Polycore.Scene_RenderDepthOnly(self.__ptr, targetCamera.__ptr)
end

function Scene:readString(inFile)
	local retVal = Polycore.Scene_readString(inFile.__ptr)
	return retVal
end

function Scene:loadScene(fileName)
	local retVal = Polycore.Scene_loadScene(self.__ptr, fileName)
end

function Scene:generateLightmaps(lightMapRes, lightMapQuality, numRadPasses)
	local retVal = Polycore.Scene_generateLightmaps(self.__ptr, lightMapRes, lightMapQuality, numRadPasses)
end

function Scene:addLight(light)
	local retVal = Polycore.Scene_addLight(self.__ptr, light.__ptr)
end

function Scene:removeLight(light)
	local retVal = Polycore.Scene_removeLight(self.__ptr, light.__ptr)
end

function Scene:getNearestLight(pos)
	local retVal = Polycore.Scene_getNearestLight(self.__ptr, pos.__ptr)
	if retVal == nil then return nil end
	if Polycore.__ptr_lookup[retVal] ~= nil then
		return Polycore.__ptr_lookup[retVal]
	else
		Polycore.__ptr_lookup[retVal] = SceneLight("__skip_ptr__")
		Polycore.__ptr_lookup[retVal].__ptr = retVal
		return Polycore.__ptr_lookup[retVal]
	end
end

function Scene:writeEntityMatrix(entity, outFile)
	local retVal = Polycore.Scene_writeEntityMatrix(self.__ptr, entity.__ptr, outFile.__ptr)
end

function Scene:writeString(str, outFile)
	local retVal = Polycore.Scene_writeString(self.__ptr, str, outFile.__ptr)
end

function Scene:saveScene(fileName)
	local retVal = Polycore.Scene_saveScene(self.__ptr, fileName)
end

function Scene:getNumStaticGeometry()
	local retVal =  Polycore.Scene_getNumStaticGeometry(self.__ptr)
	return retVal
end

function Scene:getStaticGeometry(index)
	local retVal = Polycore.Scene_getStaticGeometry(self.__ptr, index)
	if retVal == nil then return nil end
	if Polycore.__ptr_lookup[retVal] ~= nil then
		return Polycore.__ptr_lookup[retVal]
	else
		Polycore.__ptr_lookup[retVal] = SceneMesh("__skip_ptr__")
		Polycore.__ptr_lookup[retVal].__ptr = retVal
		return Polycore.__ptr_lookup[retVal]
	end
end

function Scene:loadCollisionChild(entity, autoCollide, type)
	local retVal = Polycore.Scene_loadCollisionChild(self.__ptr, entity.__ptr, autoCollide, type)
end

function Scene:getNumLights()
	local retVal =  Polycore.Scene_getNumLights(self.__ptr)
	return retVal
end

function Scene:getLight(index)
	local retVal = Polycore.Scene_getLight(self.__ptr, index)
	if retVal == nil then return nil end
	if Polycore.__ptr_lookup[retVal] ~= nil then
		return Polycore.__ptr_lookup[retVal]
	else
		Polycore.__ptr_lookup[retVal] = SceneLight("__skip_ptr__")
		Polycore.__ptr_lookup[retVal].__ptr = retVal
		return Polycore.__ptr_lookup[retVal]
	end
end

function Scene:getCustomEntityByType(type)
	local retVal = Polycore.Scene_getCustomEntityByType(self.__ptr, type)
	if retVal == nil then return nil end
	if Polycore.__ptr_lookup[retVal] ~= nil then
		return Polycore.__ptr_lookup[retVal]
	else
		Polycore.__ptr_lookup[retVal] = SceneEntity("__skip_ptr__")
		Polycore.__ptr_lookup[retVal].__ptr = retVal
		return Polycore.__ptr_lookup[retVal]
	end
end



function Scene:__delete()
	Polycore.__ptr_lookup[self.__ptr] = nil
	Polycore.delete_Scene(self.__ptr)
end
