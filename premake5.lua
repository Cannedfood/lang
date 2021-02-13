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
		defines 'LANG_DEBUG'
	filter 'configurations:release'
		optimize 'Speed'
		flags 'LinkTimeOptimization'
		defines 'NDEBUG'
	filter {}

	location(_OPTIONS.location)


	project 'lang'
		kind 'SharedLib'
		language 'C'
		files 'src/**'

	project 'lang-cli'
		kind     'ConsoleApp'
		language 'C'
		files    'extra/lang-cli/**'
		links    'lang' includedirs 'src'

	project 'lang-lsp'
		kind     'ConsoleApp'
		language 'C'
		files    'extra/lang-lsp/**'
		links    'lang' includedirs 'src'

	project 'test'
		kind     'ConsoleApp'

		language 'C++'
			cppdialect 'C++17'
		defines  'LANG_DEBUG'
		files    'test/**'

		links    'lang' includedirs 'src'
