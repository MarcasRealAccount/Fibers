if not libs then libs = {} end
if not libs.fibers then
	libs.fibers = {
		name       = "",
		location   = ""
	}
end

local fibers = libs.fibers

function fibers:setup()
	self.name     = common:projectName()
	self.location = common:projectLocation()

	kind("StaticLib")
	common:outDirs(true)
	
	common:addPCH("%{prj.location}/Source/PCH.cpp", "%{prj.location/Source/PCH.h")

	includedirs({
		"%{prj.location}/Include/",
		"%{prj.location}/Source/"
	})

	files({
		"%{prj.location}/Include/**",
		"%{prj.location}/Source/**"
	})
	removefiles({ "*.DS_Store" })
end

function fibers:setupDep()
	links({ self.name })
	sysincludedirs({ self.location .. "/include/" })
end