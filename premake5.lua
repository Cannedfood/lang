newoption {
	trigger     = "location",
	description = "Where to generate the project files",
	value       = "path",
	default     = "."
}

workspace 'lang'
	configurations { 'debug', 'release' }
	filter 'configurations:debug'
		optimize 'Debug'
		symbols 'On'
	filter 'configurations:release'
		optimize 'Speed'
		flags 'LinkTimeOptimization'
	filter {}

	location(_OPTIONS.location)

	project 'lang'
		kind 'SharedLib'
		language 'C'
		defines 'LANG_DEBUG'
		files 'src/**'

	project 'cli'
		kind    'ConsoleApp'
		language 'C'
		files   'extra/cli/**'
		defines 'LANG_DEBUG'
		links   'lang' includedirs 'src'

	project 'test'
		kind     'ConsoleApp'

		language 'C++'
			cppdialect 'C++17'
		defines  'LANG_DEBUG'
		files    'test/**'

		links    'lang' includedirs 'src'
