workspace "volume_menu"
    language "C++"
    cppdialect "C++17"
    
    architecture "x86_64"
    configurations { "Debug", "Release" }

	warnings "Extra"

    filter { "configurations:Debug" }
        defines { "_DEBUG" }
        symbols "On"

    filter { "configurations:Release" }
        optimize "On"

    filter { }

    targetdir ("bin/%{prj.name}/%{cfg.longname}")
    objdir ("obj/%{prj.name}/%{cfg.longname}")

project "volume_menu"
    kind "ConsoleApp"
    files "volume_menu/**"

	includedirs {
		"volume_menu/src"
	}

	libdirs {

	}

	links { }

