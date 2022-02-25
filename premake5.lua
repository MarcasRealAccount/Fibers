require("Premake/Common")

require("Premake/Libs/Fibers")

require("Premake/Module/cmake/cmake_project_ext")
require("Premake/Module/vstudio/vs2010_vcxproj_ext")

workspace("Fibers")
	common:setConfigsAndPlatforms()

	common:addCoreDefines()

	cppdialect("C++20")
	rtti("Off")
	exceptionhandling("On")
	flags("MultiProcessorCompile")

	startproject("Tests")

	project("Fibers")
	    location("Fibers/")
	    warnings("Extra")
		libs.fibers:setup()
		common:addActions()

	project("Tests")
		location("Tests/")
		warnings("Extra")

		kind("ConsoleApp")
		common:outDirs()
		common:debugDir()

		common:addPCH("%{prj.location}/Source/PCH.cpp", "%{prj.location}/Source/PCH.h")

		includedirs({ "%{prj.location}/Source/" })

		if common.host == "windows" then
			linkoptions({ "/IGNORE:4099" })
		elseif common.host == "macosx" then
			linkoptions({ "-Wl,-rpath,'@executable_path'" })
		end

		libs.fibers:setupDep()

		files({ "%{prj.location}/Source/**" })
		removefiles({ "*.DS_Store" })

		common:addActions()
