newoption {
	trigger     = "location",
	description = "Where to generate the project files",
	value       = "path",
	default     = "."
}

workspace 'lang'
	configurations { 'debug', 'release' }
	filter 'configurations:debug'
		symbols 'On'
	filter {}

	location(_OPTIONS.location)

	project 'lang'
		kind 'ConsoleApp'
		language 'C'
		defines 'LANG_DEBUG'
		files '**'