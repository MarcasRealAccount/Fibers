require("gmake2")

local p = premake
local m = p.modules.gmake2
local cpp = m.cpp

if _ACTION == "gmake2" then
	rule("asm")
		fileExtension({ ".asm" })
		buildoutputs({ "$(OBJDIR)/%{file.objname}.o" })
		buildmessage("$(notdir $<)")
		buildcommands({ "nasm %{premake.modules.gmake2.cpp.asmFileFlags(cfg, file)} -o \"$@\" \"$<\"" })
end

p.override(cpp, "createRuleTable", function(base, prj)
	local rules = {}

	local function addRule(extension, rule)
		if type(extension) == 'table' then
			for _, value in ipairs(extension) do
				addRule(value, rule)
			end
		else
			rules[extension] = rule
		end
	end

	-- add all rules.
	local usedRules = table.join({'asm', 'cpp', 'cc', 'resource'}, prj.rules)
	for _, name in ipairs(usedRules) do
		local rule = p.global.getRule(name)
		addRule(rule.fileExtension, rule)
	end

	-- create fileset categories.
	local filesets = {
		['.o']   = 'OBJECTS',
		['.obj'] = 'OBJECTS',
		['.asm'] = 'SOURCES',
		['.cc']  = 'SOURCES',
		['.cpp'] = 'SOURCES',
		['.cxx'] = 'SOURCES',
		['.mm']  = 'SOURCES',
		['.c']   = 'SOURCES',
		['.s']   = 'SOURCES',
		['.m']   = 'SOURCES',
		['.rc']  = 'RESOURCES',
	}

	-- cache the result.
	prj._gmake = prj._gmake or {}
	prj._gmake.rules = rules
	prj._gmake.filesets = filesets
end)

local function archToNASM(architecture)
	if common.target == "windows" then
		if architecture == "x86" then
			return "win32"
		else
			return "win64"
		end
	elseif common.target == "macosx" then
		if architecture == "x86" then
			return "macho32"
		else
			return "macho64"
		end
	else
		if architecture == "x86" then
			return "elf32"
		else
			return "elf64"
		end
	end
end

function errorReport()
	if common.target == "windows" then
		return "vc"
	else
		return "gnu"
	end
end

cpp.nasmOptimizeFlags = {
	["Off"]   = "-O0",
	["On"]    = "-O1",
	["Debug"] = "-O1",
	["Size"]  = "-Ox",
	["Speed"] = "-Ox",
	["Full"]  = "-Ox"
}

function cpp.asmFileFlags(cfg, file)
	local fcfg = p.fileconfig.getconfig(file, cfg)
	cfg        = fcfg.config
	local command = "-X" .. errorReport() .. " -f " .. archToNASM(cfg.architecture)
	
	for _, inc in ipairs(cfg.includedirs) do
		command = command .. " -i \"" .. inc .. "\""
	end
	
	for _, def in ipairs(cfg.defines) do
		command = command .. " \"-d" .. def .. "\""
	end
	for _, udef in ipairs(cfg.undefines) do
		command = command .. " \"-u" .. udef .. "\""
	end
	
	if cfg.symbols == "On" then
		command = command .. " -g"
	end
	
	command = command .. " " .. cpp.nasmOptimizeFlags[cfg.optimize]
	return command
end
