outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "lwlog"
	location "lwlog"
	kind "StaticLib"
	language "C++"
	cppdialect "C++latest"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.ixx"
	}

	includedirs {
		"%{prj.name}/src"
	}

	defines {
		"_CRT_SECURE_NO_WARNINGS"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "LWLOG_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "LWLOG_RELEASE"
		runtime "Release"
		optimize "speed"
