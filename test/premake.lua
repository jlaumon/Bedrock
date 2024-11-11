solution "BedrockTest"
	
	platforms { "x64" }
	configurations { "Debug", "DebugASAN", "DebugOpt", "Release" }
	startproject "BedrockTest"

	project "BedrockTest"

		kind "ConsoleApp"
		symbols "On"
		cppdialect "C++latest"
		exceptionhandling "Off"
		flags 
		{
			"MultiProcessorCompile",
			"FatalWarnings"
		}
		
		filter { "toolset:msc*" }
			buildoptions
			{
				"/utf-8" 
			}

		filter { "configurations:Debug" }
			targetsuffix "Debug"
			defines "ASSERTS_ENABLED"
			optimize "Debug"
			editandcontinue "On"
			
		filter { "configurations:DebugASAN" }
			targetsuffix "DebugASAN"
			defines "ASSERTS_ENABLED"
			optimize "Debug"
			editandcontinue "Off"	  -- incompatble with ASAN
			flags "NoIncrementalLink" -- incompatble with ASAN
			sanitize "Address"
			
		filter { "configurations:DebugOpt" }
			targetsuffix "DebugOpt"
			defines "ASSERTS_ENABLED"
			optimize "Full"
			editandcontinue "On"
			
		filter {}

		vpaths { ["*"] = ".." } -- get rid of the .. folder in the vstudio solution explorer
		
		files 
		{
			"../Bedrock/*.h",
			"../Bedrock/*.cpp",
			"../Bedrock/*.natvis",
			"../.clang-format",
			"../.editorconfig",
			"Main.cpp"
		}
		
		includedirs 
		{
			"..",
		}
