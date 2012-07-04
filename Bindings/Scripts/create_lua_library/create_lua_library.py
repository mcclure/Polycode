import sys
import CppHeaderParser
import os
import errno
import re
  
def mkdir_p(path): # Same effect as mkdir -p, create dir and all necessary parent dirs
	try:
		os.makedirs(path)
	except OSError as e:
		if e.errno == errno.EEXIST: # Dir already exists; not really an error
			pass
		else: raise

def createLUABindings(inputPath, prefix, mainInclude, libSmallName, libName, apiPath, apiClassPath, includePath, sourcePath, inheritInModuleFiles):
	wrappersHeaderOut = "" # Def: Global C++ *LUAWrappers.h
	cppRegisterOut = "" # Def: Global C++ *LUA.cpp
	
	luaIndexOut = "" # Def: Global Lua everything-gets-required-from-this-file file
	
	# Header boilerplate for wrappersHeaderOut and cppRegisterOut
	cppRegisterOut += "#include \"%sLUA.h\"\n" % (prefix)
	cppRegisterOut += "#include \"%sLUAWrappers.h\"\n" % (prefix)
	cppRegisterOut += "#include \"PolyCoreServices.h\"\n\n"
	cppRegisterOut += "using namespace Polycode;\n\n"
	cppRegisterOut += "int luaopen_%s(lua_State *L) {\n" % (prefix)
	if prefix != "Polycode":
		cppRegisterOut += "CoreServices *inst = (CoreServices*)lua_topointer(L, 1);\n"
		cppRegisterOut += "CoreServices::setInstance(inst);\n"
	cppRegisterOut += "\tstatic const struct luaL_reg %sLib [] = {" % (libSmallName)
	
	wrappersHeaderOut += "#pragma once\n\n"

	wrappersHeaderOut += "extern \"C\" {\n\n"
	wrappersHeaderOut += "#include <stdio.h>\n"
	wrappersHeaderOut += "#include \"lua.h\"\n"
	wrappersHeaderOut += "#include \"lualib.h\"\n"
	wrappersHeaderOut += "#include \"lauxlib.h\"\n"
	wrappersHeaderOut += "} // extern \"C\" \n\n"

	# Get list of headers to create bindings from
	files = os.listdir(inputPath)
	filteredFiles = []
	for fileName in files:
		ignore = ["PolyGLSLProgram", "PolyGLSLShader", "PolyGLSLShaderModule", "PolyWinCore", "PolyCocoaCore", "PolyAGLCore", "PolySDLCore", "Poly_iPhone", "PolyGLES1Renderer", "PolyGLRenderer", "tinyxml", "tinystr", "OpenGLCubemap", "PolyiPhoneCore", "PolyGLES1Texture", "PolyGLTexture", "PolyGLVertexBuffer", "PolyThreaded", "PolyGLHeaders", "GLee"]
		if fileName.split(".")[1] == "h" and fileName.split(".")[0] not in ignore:
			filteredFiles.append(fileName)
			wrappersHeaderOut += "#include \"%s\"\n" % (fileName)

	wrappersHeaderOut += "\nusing namespace std;\n\n"
	wrappersHeaderOut += "\nnamespace Polycode {\n\n"
	
	# Special case: If we are building the Polycode library itself, inject the LuaEventHandler class
	if prefix == "Polycode":
		wrappersHeaderOut += "class LuaEventHandler : public EventHandler {\n"
		wrappersHeaderOut += "public:\n"
		wrappersHeaderOut += "	LuaEventHandler() : EventHandler() {}\n"
		wrappersHeaderOut += "	void handleEvent(Event *e) {\n"
		wrappersHeaderOut += "		lua_rawgeti( L, LUA_REGISTRYINDEX, wrapperIndex );\n"
		wrappersHeaderOut += "		lua_getfield(L, -1, \"__handleEvent\");\n"
		wrappersHeaderOut += "		lua_rawgeti( L, LUA_REGISTRYINDEX, wrapperIndex );\n"
		wrappersHeaderOut += "		lua_pushlightuserdata(L, e);\n"
		wrappersHeaderOut += "		lua_call(L, 2, 0);\n"
		wrappersHeaderOut += "	}\n"
		wrappersHeaderOut += "	int wrapperIndex;\n"
		wrappersHeaderOut += "	lua_State *L;\n"
		wrappersHeaderOut += "};\n\n"
	
	# Iterate, process each input file
	for fileName in filteredFiles:
		# "Package owned" classes that ship with Polycode
		inheritInModule = ["PhysicsSceneEntity", "CollisionScene", "CollisionSceneEntity"]
		
		# A file or comma-separated list of files can be given to specify classes which are "package owned"
		# and should not be inherited out of Polycode/. The files should contain one class name per line,
		# and the class name may be prefixed with a path (which will be ignored).
		if inheritInModuleFiles:
			for filename in inheritInModuleFiles.split(","):
				with open(filename) as f:
					for line in f.readlines():
						inheritInModule.append(line.strip().split("/",1)[-1]) # Strip whitespace, path/
					
		headerFile = "%s/%s" % (inputPath, fileName) # Def: Full path to input file
		print "Parsing %s" % fileName
		try: # One input file parse.
			f = open(headerFile) # Def: Input file handle
			contents = f.read().replace("_PolyExport", "") # Def: Input file contents, strip out "_PolyExport"
			cppHeader = CppHeaderParser.CppHeader(contents, "string") # Def: Input file contents, parsed structure
			ignore_classes = ["PolycodeShaderModule", "Object", "Threaded", "OpenGLCubemap", "ParticleEmitter"]

			# Iterate, check each class in this file.
			for ckey in cppHeader.classes: 
				print ">> Parsing class %s" % ckey
				c = cppHeader.classes[ckey] # Def: The class structure

				luaClassBindingOut = "" # Def: The local lua file to generate for this class.
				inherits = False
				if len(c["inherits"]) > 0: # Does this class have parents?
					if c["inherits"][0]["class"] not in ignore_classes:
						if c["inherits"][0]["class"] in inheritInModule: # Parent class is in this module
							luaClassBindingOut += "require \"%s/%s\"\n\n" % (prefix, c["inherits"][0]["class"])
						else: # Parent class is in Polycore
							luaClassBindingOut += "require \"Polycode/%s\"\n\n" % (c["inherits"][0]["class"])
						luaClassBindingOut += "class \"%s\" (%s)\n\n" % (ckey, c["inherits"][0]["class"])
						inherits = True
				if inherits == False: # Class does not have parents
					luaClassBindingOut += "class \"%s\"\n\n" % ckey

				if ckey in ignore_classes:
					continue

				if len(c["methods"]["public"]) < 2:
					print("Warning: Lua-binding class with no methods")
					continue

				parsed_methods = [] # Def: List of discovered methods
				ignore_methods = ["readByte32", "readByte16", "getCustomEntitiesByType", "Core", "Renderer", "Shader", "Texture", "handleEvent", "secondaryHandler", "getSTLString"]
				luaClassBindingOut += "\n\n"

				classProperties = [] # Def: List of found property structures ("properties" meaning "data members")
				for pp in c["properties"]["public"]:
					pp["type"] = pp["type"].replace("Polycode::", "")
					pp["type"] = pp["type"].replace("std::", "")
					if pp["type"].find("static ") != -1: # If static. FIXME: Static doesn't work?
						if "defaltValue" in pp: # FIXME: defaltValue is misspelled.
							luaClassBindingOut += "%s = %s\n" % (pp["name"], pp["defaltValue"])
					else: # FIXME: Nonstatic method ? variable ?? found.
						#there are some bugs in the class parser that cause it to return junk
						if pp["type"].find("*") == -1 and pp["type"].find("vector") == -1 and pp["name"] != "setScale" and pp["name"] != "setPosition" and pp["name"] != "BUFFER_CACHE_PRECISION" and not pp["name"].isdigit():
							classProperties.append(pp)

				# Iterate over properties, creating getters
				pidx = 0 # Def: Count of properties processed so far

				if len(classProperties) > 0: # If there are properties, add index lookup to the metatable
					luaClassBindingOut += "function %s:__index__(name)\n" % ckey
					for pp in classProperties: # Iterate over property structures, creating if/else clauses for each.
						pp["type"] = pp["type"].replace("Polycode::", "")
						pp["type"] = pp["type"].replace("std::", "")
						if pidx == 0:
							luaClassBindingOut += "\tif name == \"%s\" then\n" % (pp["name"])
						else:
							luaClassBindingOut += "\telseif name == \"%s\" then\n" % (pp["name"])

						# FIXME: Don't put in so much special casing just for ScreenParticleEmitter.
						if pp["type"] == "Number" or  pp["type"] == "String" or pp["type"] == "int" or pp["type"] == "bool":
							luaClassBindingOut += "\t\treturn %s.%s_get_%s(self.__ptr)\n" % (libName, ckey, pp["name"])
						elif (ckey == "ScreenParticleEmitter" or ckey == "SceneParticleEmitter") and pp["name"] == "emitter":
							luaClassBindingOut += "\t\tlocal ret = %s(\"__skip_ptr__\")\n" % (pp["type"])
							luaClassBindingOut += "\t\tret.__ptr = self.__ptr\n"
							luaClassBindingOut += "\t\treturn ret\n"
						else:
							luaClassBindingOut += "\t\tretVal = %s.%s_get_%s(self.__ptr)\n" % (libName, ckey, pp["name"])
							luaClassBindingOut += "\t\tif Polycore.__ptr_lookup[retVal] ~= nil then\n"
							luaClassBindingOut += "\t\t\treturn Polycore.__ptr_lookup[retVal]\n"
							luaClassBindingOut += "\t\telse\n"
							luaClassBindingOut += "\t\t\tPolycore.__ptr_lookup[retVal] = %s(\"__skip_ptr__\")\n" % (pp["type"])
							luaClassBindingOut += "\t\t\tPolycore.__ptr_lookup[retVal].__ptr = retVal\n"
							luaClassBindingOut += "\t\t\treturn Polycore.__ptr_lookup[retVal]\n"
							luaClassBindingOut += "\t\tend\n"

						if not ((ckey == "ScreenParticleEmitter" or ckey == "SceneParticleEmitter") and pp["name"] == "emitter"):
							cppRegisterOut += "\t\t{\"%s_get_%s\", %s_%s_get_%s},\n" % (ckey, pp["name"], libName, ckey, pp["name"])
							wrappersHeaderOut += "static int %s_%s_get_%s(lua_State *L) {\n" % (libName, ckey, pp["name"])
							wrappersHeaderOut += "\tluaL_checktype(L, 1, LUA_TLIGHTUSERDATA);\n"
							wrappersHeaderOut += "\t%s *inst = (%s*)lua_topointer(L, 1);\n" % (ckey, ckey)

							outfunc = "lua_pushlightuserdata"
							retFunc = ""
							if pp["type"] == "Number":
								outfunc = "lua_pushnumber"
							if pp["type"] == "String":
								outfunc = "lua_pushstring"
								retFunc = ".c_str()"
							if pp["type"] == "int":
								outfunc = "lua_pushinteger"
							if pp["type"] == "bool":
								outfunc = "lua_pushboolean"

							if pp["type"] == "Number" or  pp["type"] == "String" or pp["type"] == "int" or pp["type"] == "bool":
								wrappersHeaderOut += "\t%s(L, inst->%s%s);\n" % (outfunc, pp["name"], retFunc)
							else:
								wrappersHeaderOut += "\t%s(L, &inst->%s%s);\n" % (outfunc, pp["name"], retFunc)
							wrappersHeaderOut += "\treturn 1;\n"
							wrappersHeaderOut += "}\n\n"
						pidx = pidx + 1

					luaClassBindingOut += "\tend\n"
					luaClassBindingOut += "end\n"

				luaClassBindingOut += "\n\n"
				
				# Iterate over properties again, creating setters
				pidx = 0 # Def: Count of 
				if len(classProperties) > 0: # If there are properties, add index setter to the metatable
					luaClassBindingOut += "function %s:__set_callback(name,value)\n" % ckey
					for pp in classProperties:
						pp["type"] = pp["type"].replace("Polycode::", "")
						pp["type"] = pp["type"].replace("std::", "")
						if pp["type"] == "Number" or  pp["type"] == "String" or pp["type"] == "int" or pp["type"] == "bool":
							if pidx == 0:
								luaClassBindingOut += "\tif name == \"%s\" then\n" % (pp["name"])
							else:
								luaClassBindingOut += "\telseif name == \"%s\" then\n" % (pp["name"])
							luaClassBindingOut += "\t\t%s.%s_set_%s(self.__ptr, value)\n" % (libName, ckey, pp["name"])
							luaClassBindingOut += "\t\treturn true\n"

							cppRegisterOut += "\t\t{\"%s_set_%s\", %s_%s_set_%s},\n" % (ckey, pp["name"], libName, ckey, pp["name"])
							wrappersHeaderOut += "static int %s_%s_set_%s(lua_State *L) {\n" % (libName, ckey, pp["name"])
							wrappersHeaderOut += "\tluaL_checktype(L, 1, LUA_TLIGHTUSERDATA);\n"
							wrappersHeaderOut += "\t%s *inst = (%s*)lua_topointer(L, 1);\n" % (ckey, ckey)

							outfunc = "lua_topointer"
							if pp["type"] == "Number":
								outfunc = "lua_tonumber"
							if pp["type"] == "String":
								outfunc = "lua_tostring"
							if pp["type"] == "int":
								outfunc = "lua_tointeger"
							if pp["type"] == "bool":
								outfunc = "lua_toboolean"

							wrappersHeaderOut += "\t%s param = %s(L, 2);\n" % (pp["type"], outfunc)
							wrappersHeaderOut += "\tinst->%s = param;\n" % (pp["name"])

							wrappersHeaderOut += "\treturn 0;\n"
							wrappersHeaderOut += "}\n\n"
							pidx = pidx + 1
					if pidx != 0:
						luaClassBindingOut += "\tend\n"
					luaClassBindingOut += "\treturn false\n"
					luaClassBindingOut += "end\n"

				# Iterate over methods
				luaClassBindingOut += "\n\n"
				for pm in c["methods"]["public"]:
					if pm["name"] in parsed_methods or pm["name"].find("operator") > -1 or pm["name"] in ignore_methods:
						continue

					if pm["name"] == "~"+ckey or pm["rtnType"].find("<") > -1:
						wrappersHeaderOut += ""
					else:
						basicType = False
						voidRet = False
						if pm["name"] == ckey:
							cppRegisterOut += "\t\t{\"%s\", %s_%s},\n" % (ckey, libName, ckey)
							wrappersHeaderOut += "static int %s_%s(lua_State *L) {\n" % (libName, ckey)
							idx = 1
						else:
							cppRegisterOut += "\t\t{\"%s_%s\", %s_%s_%s},\n" % (ckey, pm["name"], libName, ckey, pm["name"])
							wrappersHeaderOut += "static int %s_%s_%s(lua_State *L) {\n" % (libName, ckey, pm["name"])

							if pm["rtnType"].find("static ") == -1:
								wrappersHeaderOut += "\tluaL_checktype(L, 1, LUA_TLIGHTUSERDATA);\n"
								wrappersHeaderOut += "\t%s *inst = (%s*)lua_topointer(L, 1);\n" % (ckey, ckey)
							idx = 2
						paramlist = []
						lparamlist = []
						for param in pm["parameters"]:
							if not param.has_key("type"):
								continue
							if param["type"] == "0":
								continue
							param["type"] = param["type"].replace("Polycode::", "")
							param["type"] = param["type"].replace("std::", "")
							param["type"] = param["type"].replace("const", "")
							param["type"] = param["type"].replace("&", "")
							param["type"] = param["type"].replace(" ", "")
							param["type"] = param["type"].replace("long", "long ")
							param["type"] = param["type"].replace("unsigned", "unsigned ")

							param["name"] = param["name"].replace("end", "_end").replace("repeat", "_repeat")
							if"type" in param:
								luatype = "LUA_TLIGHTUSERDATA"
								checkfunc = "lua_islightuserdata"
								if param["type"].find("*") > -1:
									luafunc = "(%s)lua_topointer" % (param["type"].replace("Polygon", "Polycode::Polygon").replace("Rectangle", "Polycode::Rectangle"))
								elif param["type"].find("&") > -1:
									luafunc = "*(%s*)lua_topointer" % (param["type"].replace("const", "").replace("&", "").replace("Polygon", "Polycode::Polygon").replace("Rectangle", "Polycode::Rectangle"))
								else:
									luafunc = "*(%s*)lua_topointer" % (param["type"].replace("Polygon", "Polycode::Polygon").replace("Rectangle", "Polycode::Rectangle"))
								lend = ".__ptr"
								if param["type"] == "int" or param["type"] == "unsigned int":
									luafunc = "lua_tointeger"
									luatype = "LUA_TNUMBER"
									checkfunc = "lua_isnumber"
									lend = ""
								if param["type"] == "bool":
									luafunc = "lua_toboolean"
									luatype = "LUA_TBOOLEAN"
									checkfunc = "lua_isboolean"
									lend = ""
								if param["type"] == "Number" or param["type"] == "float" or param["type"] == "double":
									luatype = "LUA_TNUMBER"
									luafunc = "lua_tonumber"
									checkfunc = "lua_isnumber"
									lend = ""
								if param["type"] == "String":
									luatype = "LUA_TSTRING"
									luafunc = "lua_tostring"
									checkfunc = "lua_isstring"
									lend = ""

								param["type"] = param["type"].replace("Polygon", "Polycode::Polygon").replace("Rectangle", "Polycode::Rectangle")

								if "defaltValue" in param:
									if checkfunc != "lua_islightuserdata" or (checkfunc == "lua_islightuserdata" and param["defaltValue"] == "NULL"):
										#param["defaltValue"] = param["defaltValue"].replace(" 0f", ".0f")
										param["defaltValue"] = param["defaltValue"].replace(": :", "::")
										#param["defaltValue"] = param["defaltValue"].replace("0 ", "0.")
										param["defaltValue"] = re.sub(r'([0-9]+) ([0-9])+', r'\1.\2', param["defaltValue"])

										wrappersHeaderOut += "\t%s %s;\n" % (param["type"], param["name"])
										wrappersHeaderOut += "\tif(%s(L, %d)) {\n" % (checkfunc, idx)
										wrappersHeaderOut += "\t\t%s = %s(L, %d);\n" % (param["name"], luafunc, idx)
										wrappersHeaderOut += "\t} else {\n"
										wrappersHeaderOut += "\t\t%s = %s;\n" % (param["name"], param["defaltValue"])
										wrappersHeaderOut += "\t}\n"
									else:
										wrappersHeaderOut += "\tluaL_checktype(L, %d, %s);\n" % (idx, luatype);
										if param["type"] == "String":
											wrappersHeaderOut += "\t%s %s = String(%s(L, %d));\n" % (param["type"], param["name"], luafunc, idx)
										else:
											wrappersHeaderOut += "\t%s %s = %s(L, %d);\n" % (param["type"], param["name"], luafunc, idx)
								else:
									wrappersHeaderOut += "\tluaL_checktype(L, %d, %s);\n" % (idx, luatype);
									if param["type"] == "String":
										wrappersHeaderOut += "\t%s %s = String(%s(L, %d));\n" % (param["type"], param["name"], luafunc, idx)
									else:
										wrappersHeaderOut += "\t%s %s = %s(L, %d);\n" % (param["type"], param["name"], luafunc, idx)
								paramlist.append(param["name"])

								lparamlist.append(param["name"]+lend)
								idx = idx +1

						if pm["name"] == ckey:
							if ckey == "EventHandler":
								wrappersHeaderOut += "\tLuaEventHandler *inst = new LuaEventHandler();\n"
								wrappersHeaderOut += "\tinst->wrapperIndex = luaL_ref(L, LUA_REGISTRYINDEX );\n"
								wrappersHeaderOut += "\tinst->L = L;\n"
							else:
								wrappersHeaderOut += "\t%s *inst = new %s(%s);\n" % (ckey, ckey, ", ".join(paramlist))
							wrappersHeaderOut += "\tlua_pushlightuserdata(L, (void*)inst);\n"
							wrappersHeaderOut += "\treturn 1;\n"
						else:
							if pm["rtnType"].find("static ") == -1:
								call = "inst->%s(%s)" % (pm["name"], ", ".join(paramlist))
							else:
								call = "%s::%s(%s)" % (ckey, pm["name"], ", ".join(paramlist))
							if pm["rtnType"] == "void" or pm["rtnType"] == "static void" or pm["rtnType"] == "virtual void" or pm["rtnType"] == "inline void":
								wrappersHeaderOut += "\t%s;\n" % (call)
								basicType = True
								voidRet = True
								wrappersHeaderOut += "\treturn 0;\n"
							else:
								outfunc = "lua_pushlightuserdata"
								retFunc = ""
								basicType = False
								if pm["rtnType"] == "Number" or  pm["rtnType"] == "inline Number":
									outfunc = "lua_pushnumber"
									basicType = True
								if pm["rtnType"] == "String" or pm["rtnType"] == "static String":
									outfunc = "lua_pushstring"
									basicType = True
									retFunc = ".c_str()"
								if pm["rtnType"] == "int" or pm["rtnType"] == "static int" or  pm["rtnType"] == "size_t" or pm["rtnType"] == "static size_t" or pm["rtnType"] == "long" or pm["rtnType"] == "unsigned int" or pm["rtnType"] == "static long":
									outfunc = "lua_pushinteger"
									basicType = True
								if pm["rtnType"] == "bool" or pm["rtnType"] == "static bool" or pm["rtnType"] == "virtual bool":
									outfunc = "lua_pushboolean"
									basicType = True

								if pm["rtnType"].find("*") > -1:
									wrappersHeaderOut += "\tvoid *ptrRetVal = (void*)%s%s;\n" % (call, retFunc)
									wrappersHeaderOut += "\tif(ptrRetVal == NULL) {\n"
									wrappersHeaderOut += "\t\tlua_pushnil(L);\n"
									wrappersHeaderOut += "\t} else {\n"
									wrappersHeaderOut += "\t\t%s(L, ptrRetVal);\n" % (outfunc)
									wrappersHeaderOut += "\t}\n"
								elif basicType == True:
									wrappersHeaderOut += "\t%s(L, %s%s);\n" % (outfunc, call, retFunc)
								else:
									className = pm["rtnType"].replace("const", "").replace("&", "").replace("inline", "").replace("virtual", "").replace("static", "")
									if className == "Polygon":
										className = "Polycode::Polygon"
									if className == "Rectangle":
										className = "Polycode::Rectangle"
									wrappersHeaderOut += "\t%s *retInst = new %s();\n" % (className, className)
									wrappersHeaderOut += "\t*retInst = %s;\n" % (call)
									wrappersHeaderOut += "\t%s(L, retInst);\n" % (outfunc)
								wrappersHeaderOut += "\treturn 1;\n"
						wrappersHeaderOut += "}\n\n"

						if pm["name"] == ckey:
							luaClassBindingOut += "function %s:%s(...)\n" % (ckey, ckey)
							if inherits:
								luaClassBindingOut += "\tif type(arg[1]) == \"table\" and count(arg) == 1 then\n"
								luaClassBindingOut += "\t\tif \"\"..arg[1]:class() == \"%s\" then\n" % (c["inherits"][0]["class"])
								luaClassBindingOut += "\t\t\tself.__ptr = arg[1].__ptr\n"
								luaClassBindingOut += "\t\t\treturn\n"
								luaClassBindingOut += "\t\tend\n"
								luaClassBindingOut += "\tend\n"
							luaClassBindingOut += "\tfor k,v in pairs(arg) do\n"
							luaClassBindingOut += "\t\tif type(v) == \"table\" then\n"
							luaClassBindingOut += "\t\t\tif v.__ptr ~= nil then\n"
							luaClassBindingOut += "\t\t\t\targ[k] = v.__ptr\n"
							luaClassBindingOut += "\t\t\tend\n"
							luaClassBindingOut += "\t\tend\n"
							luaClassBindingOut += "\tend\n"
							luaClassBindingOut += "\tif self.__ptr == nil and arg[1] ~= \"__skip_ptr__\" then\n"
							if ckey == "EventHandler":
								luaClassBindingOut += "\t\tself.__ptr = %s.%s(self)\n" % (libName, ckey)
							else:
								luaClassBindingOut += "\t\tself.__ptr = %s.%s(unpack(arg))\n" % (libName, ckey)
							luaClassBindingOut += "\t\tPolycore.__ptr_lookup[self.__ptr] = self\n"
							luaClassBindingOut += "\tend\n"
							luaClassBindingOut += "end\n\n"
						else:
							luaClassBindingOut += "function %s:%s(%s)\n" % (ckey, pm["name"], ", ".join(paramlist))
							if pm["rtnType"].find("static ") == -1:
								if len(lparamlist):
									luaClassBindingOut += "\tlocal retVal = %s.%s_%s(self.__ptr, %s)\n" % (libName, ckey, pm["name"], ", ".join(lparamlist))
								else:
									luaClassBindingOut += "\tlocal retVal =  %s.%s_%s(self.__ptr)\n" % (libName, ckey, pm["name"])
							else:
								if len(lparamlist):
									luaClassBindingOut += "\tlocal retVal = %s.%s_%s(%s)\n" % (libName, ckey, pm["name"], ", ".join(lparamlist))
								else:
									luaClassBindingOut += "\tlocal retVal =  %s.%s_%s()\n" % (libName, ckey, pm["name"])

							if not voidRet:
								if basicType == True:
									luaClassBindingOut += "\treturn retVal\n"
								else:
									className = pm["rtnType"].replace("const", "").replace("&", "").replace("inline", "").replace("virtual", "").replace("static", "").replace("*","").replace(" ", "")
									luaClassBindingOut += "\tif retVal == nil then return nil end\n"
									luaClassBindingOut += "\tif Polycore.__ptr_lookup[retVal] ~= nil then\n"
									luaClassBindingOut += "\t\treturn Polycore.__ptr_lookup[retVal]\n"
									luaClassBindingOut += "\telse\n"
									luaClassBindingOut += "\t\tPolycore.__ptr_lookup[retVal] = %s(\"__skip_ptr__\")\n" % (className)
									luaClassBindingOut += "\t\tPolycore.__ptr_lookup[retVal].__ptr = retVal\n"
									luaClassBindingOut += "\t\treturn Polycore.__ptr_lookup[retVal]\n"
									luaClassBindingOut += "\tend\n"
							luaClassBindingOut += "end\n\n"

					parsed_methods.append(pm["name"])

				#cleanup
				cppRegisterOut += "\t\t{\"delete_%s\", %s_delete_%s},\n" % (ckey, libName, ckey)
				wrappersHeaderOut += "static int %s_delete_%s(lua_State *L) {\n" % (libName, ckey)
				wrappersHeaderOut += "\tluaL_checktype(L, 1, LUA_TLIGHTUSERDATA);\n"
				wrappersHeaderOut += "\t%s *inst = (%s*)lua_topointer(L, 1);\n" % (ckey, ckey)
				wrappersHeaderOut += "\tdelete inst;\n"
				wrappersHeaderOut += "\treturn 0;\n"
				wrappersHeaderOut += "}\n\n"

				luaClassBindingOut += "\n\n"
				luaClassBindingOut += "function %s:__delete()\n" % (ckey)
				luaClassBindingOut += "\tPolycore.__ptr_lookup[self.__ptr] = nil\n"
				luaClassBindingOut += "\t%s.delete_%s(self.__ptr)\n" % (libName, ckey)
				luaClassBindingOut += "end\n"
				if ckey == "EventHandler":
					luaClassBindingOut += "\n\n"
					luaClassBindingOut += "function EventHandler:__handleEvent(event)\n"
					luaClassBindingOut += "\tevt = Event(\"__skip_ptr__\")\n"
					luaClassBindingOut += "\tevt.__ptr = event\n"
					luaClassBindingOut += "\tself:handleEvent(evt)\n"
					#luaClassBindingOut += "\tself:handleEvent(event)\n"
					luaClassBindingOut += "end\n"
				luaIndexOut += "require \"%s/%s\"\n" % (prefix, ckey)
				mkdir_p(apiClassPath)
				fout = open("%s/%s.lua" % (apiClassPath, ckey), "w")
				fout.write(luaClassBindingOut)
		except CppHeaderParser.CppParseError,  e: # One input file parse; failed.
			print e
			sys.exit(1)

	# Footer boilerplate for wrappersHeaderOut and cppRegisterOut.
	wrappersHeaderOut += "} // namespace Polycode\n"
	
	cppRegisterOut += "\t\t{NULL, NULL}\n"
	cppRegisterOut += "\t};\n"
	cppRegisterOut += "\tluaL_openlib(L, \"%s\", %sLib, 0);\n" % (libName, libSmallName)
	cppRegisterOut += "\treturn 1;\n"
	cppRegisterOut += "}"
	
	
	cppRegisterHeaderOut = "" # Def: Global C++ *LUA.h
	cppRegisterHeaderOut += "#pragma once\n"
	cppRegisterHeaderOut += "#include <%s>\n" % (mainInclude)
	cppRegisterHeaderOut += "extern \"C\" {\n"
	cppRegisterHeaderOut += "#include <stdio.h>\n"
	cppRegisterHeaderOut += "#include \"lua.h\"\n"
	cppRegisterHeaderOut += "#include \"lualib.h\"\n"
	cppRegisterHeaderOut += "#include \"lauxlib.h\"\n"
	cppRegisterHeaderOut += "int _PolyExport luaopen_%s(lua_State *L);\n" % (prefix)
	cppRegisterHeaderOut += "}\n"
	
	# Write out global files
	mkdir_p(includePath)
	mkdir_p(apiPath)
	mkdir_p(sourcePath)

	fout = open("%s/%sLUA.h" % (includePath, prefix), "w")
	fout.write(cppRegisterHeaderOut)

	fout = open("%s/%s.lua" % (apiPath, prefix), "w")
	fout.write(luaIndexOut)
	
	fout = open("%s/%sLUAWrappers.h" % (includePath, prefix), "w")
	fout.write(wrappersHeaderOut)
	
	fout = open("%s/%sLUA.cpp" % (sourcePath, prefix), "w")
	fout.write(cppRegisterOut)
	
	# Create .pak zip archive
	pattern = '*.lua'
	os.chdir(apiPath)
	if libName == "Polycore":
		with ZipFile("api.pak", 'w') as myzip:
			for root, dirs, files in os.walk("."):
			    for filename in fnmatch.filter(files, pattern):
				myzip.write(os.path.join(root, filename))

if len(sys.argv) < 10:
	print ("Usage:\n%s [input path] [prefix] [main include] [lib small name] [lib name] [api path] [api class-path] [include path] [source path] [inherit-in-module-file path (optional)]" % (sys.argv[0]))
	sys.exit(1)
else:
	createLUABindings(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], sys.argv[6], sys.argv[7], sys.argv[8], sys.argv[9], sys.argv[10] if len(sys.argv)>10 else None)