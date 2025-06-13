solution "BedrockTest"
	
	platforms { "x64", "Clang" }
	configurations { "Debug", "DebugASAN", "DebugOpt", "Release" }
	startproject "BedrockTest"

	project "BedrockTest"

		kind "ConsoleApp"
		symbols "On"
		cppdialect "C++latest"
		exceptionhandling "Off"
		rtti "Off"
		architecture "x64"
		flags 
		{
			"MultiProcessorCompile",
			"FatalWarnings"
		}

		buildoptions
		{
			"/utf-8" 
		}
		
		filter { "platforms:Clang" }
			toolset "clang"

		filter { "configurations:Debug" }
			targetsuffix "Debug"
			defines "ASSERTS_ENABLED"
			optimize "Debug"
			runtime "Debug"
			editandcontinue "On"
			
		filter { "configurations:DebugASAN" }
			targetsuffix "DebugASAN"
			defines "ASSERTS_ENABLED"
			optimize "Debug"
			runtime "Debug"
			editandcontinue "Off"	  -- incompatible with ASAN
			flags "NoIncrementalLink" -- incompatible with ASAN
			sanitize "Address"

		-- Note: ASAN + Clang is not working. This is enough to make it link, but it doesn't run.
		-- filter { "configurations:DebugASAN", "platforms:Clang" }
		-- 	runtime "Release"		  -- debug runtime is incompatible with ASAN (clang-cl)
		-- 	links 
		-- 	{
		-- 		-- apparently VS doesn't automatically link the libs when using ASAN with clang-cl
		-- 		"clang_rt.asan_dynamic-x86_64",
		-- 		"clang_rt.asan_dynamic_runtime_thunk-x86_64"
		-- 	}

		filter { "configurations:DebugOpt" }
			targetsuffix "DebugOpt"
			defines "ASSERTS_ENABLED"
			optimize "Full"
			editandcontinue "On"

		filter { "configurations:Release" }
			optimize "Full"
			
		filter {}

		vpaths { ["*"] = ".." } -- get rid of the .. folder in the vstudio solution explorer
		
		files 
		{
			"../Bedrock/*.h",
			"../Bedrock/*.cpp",
			"../Bedrock/*.natvis",
			"../Bedrock/*.natstepfilter",
			"../.clang-format",
			"../.editorconfig",
			"Main.cpp"
		}
		
		includedirs 
		{
			"..",
		}
