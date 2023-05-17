workspace "DemosForX64"
	architecture "x64"
	startproject "SingleAxisOnOff"

	configurations
	{
		"Debug",
		"Release"
	}

	flags 
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.systme}-%{cfg.architecture}"

project "Log"
	location "Log"
	kind "SharedLib"
	language "C++"
	staticruntime "Off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/vendor/spdlog/include"
	}

	links
	{

	}

	filter "systme:windows"
		cppdialect "C++17"		
		systemversion "latest"

		defines
		{
			"LOG_LIB",
			"_CONSOLE"
		}

		postbuildcommands
		{
			("%{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/SingleAxisOnOff/\"")
		}

	filter "configurations:Debug"
		defines { "_DEBUG" }
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		optimize "On"
