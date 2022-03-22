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
	common:outDirs(false)
	
	common:addPCH(self.location .. "/Source/PCH.cpp", self.location .. "/Source/PCH.h")

	includedirs({
		self.location .. "/Include/",
		self.location .. "/Source/"
	})

	files({
		self.location .. "/Include/**",
		self.location .. "/Source/**"
	})
	removefiles({ "*.DS_Store" })
end

function fibers:setupDep()
	links({ self.name })
	sysincludedirs({ self.location .. "/include/" })
end
