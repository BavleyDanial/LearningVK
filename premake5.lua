--------------------------------------- WorkSpace ---------------------------------------
workspace "LearningVK"
	architecture "x64"
	startproject "SandboxVK"

	configurations
	{
		"Debug",
		"Release"
	}

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	include "ExternalVK.lua"
--------------------------------------- WorkSpace ---------------------------------------

--------------------------------------- EngineVK ---------------------------------------
project "EngineVK"
	location "EngineVK"
	kind "StaticLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	pchheader "vkpch.h"
	pchsource "%{prj.name}/src/vkpch.cpp"

	postbuildcommands
	{
		("{MKDIR} ../Bin/" .. outputdir .. "/DunnSandbox/")
	}
	

	defines
	{
		"GLFW_INCLUDE_NONE"
	}

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src/",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.glm}"
	}

	links
	{
		"GLFW",
		"%{Library.Vulkan}"
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"
        defines "VK_PLATFORM_WINDOWS"

		filter "configurations:Debug"
		    defines "VK_DEBUG"
		    runtime "Debug"
		    symbols "On"

	    filter "configurations:Release"
		    defines "VK_RELEASE"
		    runtime "Release"
		    optimize "On"
--------------------------------------- EngineVK ---------------------------------------


--------------------------------------- SandboxVK ---------------------------------------
project "SandboxVK"
	location "SandboxVK"
	kind "ConsoleApp"
	language "C++"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"EngineVK/src/",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.glm}"
	}

    links
    {
        "EngineVK"
    }

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"
        defines "VK_PLATFORM_WINDOWS"

	    filter "configurations:Debug"
		    defines "VK_DEBUG"
		    runtime "Debug"
		    symbols "On"

        filter "configurations:Release"
            defines "VK_RELEASE"
            runtime "Release"
            optimize "On"
--------------------------------------- SandboxVK ---------------------------------------
