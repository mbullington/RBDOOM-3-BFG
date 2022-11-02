cmake_minimum_required(VERSION 3.2)
project(RBDoom3BFG)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# set MSVC startup project
set_property (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT RBDoom3BFG)
	
option(STANDALONE
		"Skip DOOM-3-BFG base/ folder and use assets/ instead" OFF)

option(FORCE_COLOR_OUTPUT
		"Always produce ANSI-colored output (GNU/Clang only)." ON)

option(COMPILE_COMMANDS
		"Generate compile_commands.json" OFF)

option(USE_PRECOMPILED_HEADERS
		"Use precompiled headers during build" ON)
		
option(USE_VULKAN
		"Use Vulkan instead of OpenGL" OFF)

option(SPIRV_SHADERC
		"Compile SPIR-V shader byte code using shaderc instead of using Glslang directly" OFF)

option(USE_MoltenVK
		"Use MoltenVK library directly instead of Vulkan loader on macOS" OFF)

option(ONATIVE
		"Optimize for the host CPU" OFF)
		
option(RUN_TESTS
		"Run tests instead of building RBDOOM3-BFG" OFF)

set(CPU_TYPE "" CACHE STRING "When set, passes this string as CPU-ID which will be embedded into the binary.")

# Make sure to include repo root for vendor.
include_directories("../")

# SRS - Turn on optimization when cross-compiling from Apple arm64 to x86_64
if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)" OR CMAKE_OSX_ARCHITECTURES MATCHES "x86_64")
	set(CPU_OPTIMIZATION "-mmmx -msse -msse2" CACHE STRING "Which CPU specific optimitations should be used beside the compiler's default?")
endif()

# SRS - Turn off MMX/SSE intrinsics when cross-compiling from Apple x86_64 to arm64
if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)|(e2k)|(E2K)" AND NOT CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
	option(USE_INTRINSICS_SSE "Compile using SSE intrinsics (e.g mmx, sse, msse2)" ON)
else()
	option(USE_INTRINSICS_SSE "Compile using SSE intrinsics (e.g mmx, sse, msse2)" OFF)
endif()

if(UNIX)	
    # SRS - Disable precompiled headers when cross-compiling on OSX, e.g. when creating universal binaries
    if((CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64" AND CMAKE_OSX_ARCHITECTURES MATCHES "arm64" ) OR
       (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "arm64"  AND CMAKE_OSX_ARCHITECTURES MATCHES "x86_64"))
        set(USE_PRECOMPILED_HEADERS OFF)
    endif()

	if(USE_VULKAN)		
		# RB: disable precompiled headers on Linux with Vulkan because of error: POSIX thread support was disabled in PCH file but is currently enabled
		if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
			if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
            	set(USE_PRECOMPILED_HEADERS OFF)
			endif()
        endif()
	endif()
endif()

if(COMPILE_COMMANDS)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

if (FORCE_COLOR_OUTPUT)
	if (CMAKE_COMPILER_IS_GNUCC)
		add_compile_options(-fdiagnostics-color=always)
	elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
		add_compile_options(-fcolor-diagnostics)
	endif ()
endif ()

if(MSVC)
	#message(STATUS CMAKE_ROOT: ${CMAKE_ROOT})
	
	#if(CMAKE_CL_64)
	#	SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../bin/win64)
	#else()
	#	SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../bin/win32)
	#endif()
	
	#message(STATUS EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH})
	#message(STATUS PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR})
	#message(STATUS CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR})
else()
	message(STATUS CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE})
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")

	add_definitions(-pipe)
	#add_definitions(-Wall)
	add_definitions(-Werror=format-security)
	add_definitions(-Werror=format)

	# Compiler check (needs -std=c++20 flag)
	include(CheckCXXCompilerFlag)
	CHECK_CXX_COMPILER_FLAG("-std=c++20" COMPILER_SUPPORTS_CXX20)
	if(COMPILER_SUPPORTS_CXX20)
		set(CMAKE_CXX_STANDARD 20)
		set(CMAKE_CXX_STANDARD_REQUIRED ON)
		set(CMAKE_CXX_EXTENSIONS OFF)
	else()
		message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++20 support. Please use a different C++ compiler.")
	endif()

	if(CPU_TYPE)
		add_definitions(-DCPUSTRING="${CPU_TYPE}")
	endif()
	if (CPU_OPTIMIZATION)
		add_definitions(${CPU_OPTIMIZATION})
	endif()
	if(WIN32)
		# require msvcr70.dll or newer for _aligned_malloc etc
		# I think it is from Visual C++ .NET 2002, so it should be available on any remotely modern system.
		add_definitions(-D__MSVCRT_VERSION__=0x0700)
		
		find_package(DirectX REQUIRED)
		include_directories(../vendor/mingw-hacks)
		include_directories(${DirectX_INCLUDE_DIR})
		add_definitions(-DUSE_XINPUT)
	endif()	
	
	#if(NOT ANDROID)
		add_definitions(-DUSE_EXCEPTIONS)
	#endif()
	
	add_compile_options(-Wno-pragmas -Wno-unused-variable -Wno-switch -Wno-unused-value -Winvalid-pch -Wno-multichar)
    if(CMAKE_C_COMPILER_ID MATCHES "Clang")
		# add clang-specific settings for warnings (the second one make sure clang doesn't complain
		# about unknown -W flags, like -Wno-unused-but-set-variable)
		# SRS - Add -Wno-deprecated-register and -Wno-expansion-to-defined to list of warning settings
		add_definitions(-Wno-local-type-template-args -Wno-unknown-warning-option -Wno-inline-new-delete -Wno-switch-enum -Wno-deprecated-register -Wno-expansion-to-defined)
	endif()
	
	if(NOT CMAKE_CROSSCOMPILING AND ONATIVE)
		if(CMAKE_SYSTEM_PROCESSOR MATCHES "((powerpc|ppc)64le)|(mips64)")
			add_definitions(-mcpu=native)
		else()
			add_definitions(-march=native)
		endif()
	endif()

	# Otherwise, assume x86_64 v3 (Haswell and above).
	if(NOT CMAKE_CROSSCOMPILING AND NOT ONATIVE AND CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)")
		add_definitions(-march=haswell)
	endif()

	add_compile_options(-fno-strict-aliasing)

	# SRS - Make sure OSX can find system headers and add support for minimum OSX runtime version
	if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		# SRS - Also add -fasm-blocks otherwise Xcode complains and -Qunused-arguments to silence MMX/SSE unused arg warnings when compiling for Apple arm64
		add_definitions(-fasm-blocks -Qunused-arguments)
        if(CMAKE_OSX_SYSROOT)
            add_definitions(-isysroot "${CMAKE_OSX_SYSROOT}")
            message(STATUS "Using macOS sysroot: " ${CMAKE_OSX_SYSROOT})
        endif()
		if(CMAKE_OSX_DEPLOYMENT_TARGET)
			add_definitions(-mmacosx-version-min="${CMAKE_OSX_DEPLOYMENT_TARGET}")
		endif()
	endif()

    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb -D_DEBUG -O0")
    #set(CMAKE_C_FLAGS_DEBUGALL "${CMAKE_C_FLAGS_DEBUGALL} -g -ggdb -D_DEBUG")
    #set(CMAKE_C_FLAGS_PROFILE "${CMAKE_C_FLAGS_PROFILE} -g -ggdb -D_DEBUG -O1 -fno-omit-frame-pointer")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -ffp-contract=off -fomit-frame-pointer -fno-math-errno -fno-trapping-math")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g -O3 -ffp-contract=off -fomit-frame-pointer -fno-math-errno -fno-trapping-math")
	set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -Os -ffp-contract=off -fomit-frame-pointer -fno-math-errno -fno-trapping-math")
	
	set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
	#set(CMAKE_CXX_FLAGS_DEBUGALL ${CMAKE_C_FLAGS_DEBUGALL})
	#set(CMAKE_CXX_FLAGS_PROFILE ${CMAKE_C_FLAGS_PROFILE})
	set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO})
	set(CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_C_FLAGS_MINSIZEREL})

elseif(MSVC)
	
	# Multiprocessor support for compiling
	# /MP
	
	# /MT  use multithreaded, static runtime library
	#      (=> binary won't need MSVCP100D.dll or similar)
	# /MTd the same with debug symbols
	# /MD and /MDd - the same with dynamic runtime lib
	#     (=> MSVCP100D.dll or similar is needed to run binary)

	# Inline Function Expansion: Any Suitable (/Ob2)
	# "/Ob2",
			
	# enable Intrinsic Functions
	# "/Oi",
			
	# Omit Frame Pointers
	# "/Oy",
	
	# if(MSVC_VERSION EQUAL 1700)
		# #message(STATUS "$ENV{LIBPATH}")
		
		# find_path(Windows_winmd_DIR NAMES Windows.winmd
			# HINTS
			# C:/Program Files (x86)/Windows Kits/8.0/References/CommonConfiguration/Neutral
			# PATH_SUFFIXES "Neutral")
			
		# message(STATUS "${Windows_winmd_DIR}")
		
		# #set(WINRT_OPTIONS "/ZW /D USE_WINRT /AI \"C:\Program Files (x86)\Windows Kits\8.0\References\CommonConfiguration\Neutral\"")
		# #set(WINRT_OPTIONS "/ZW /D USE_WINRT /AI \"$ENV{LIBPATH}\" ")
		# set(WINRT_OPTIONS "/ZW /D USE_WINRT")
	
		# set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D _DEBUG /MP ${WINRT_OPTIONS}")
		# set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP /Oi /Oy ${WINRT_OPTIONS}")
		# set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MP /Oi ${WINRT_OPTIONS}")
		# set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /MP /Oi /Oy ${WINRT_OPTIONS}")
		
		# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP ${WINRT_OPTIONS}")
		# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP /Oi /Oy ${WINRT_OPTIONS}")
		# set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MP ${WINRT_OPTIONS}")
		# set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MP /Oi /Oy ${WINRT_OPTIONS}")
	# else()
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D _DEBUG /MP /MDd")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP /Oi /Oy /MD")
		set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MP /Oi /MDd")
		set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /MP /Oi /Oy /MD")
		
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP /MDd")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP /Oi /Oy /MD")
		set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MP /MDd")
		set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MP /Oi /Oy /MD")
		
		# RB: without /SAFESEH:NO we can't link against ffmpeg libs and VS2013 or we have to build our own
		# libs for newer compilers
		set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SAFESEH:NO")
		set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} /SAFESEH:NO")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SAFESEH:NO")
		set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /SAFESEH:NO")
	# endif()

	add_definitions(-DWIN32 
					-D_WINDOWS
					-D_CRT_SECURE_NO_DEPRECATE
					-D_CRT_NONSTDC_NO_DEPRECATE
					-D_CRT_SECURE_NO_WARNINGS
					-D_MBCS
					-DUSE_EXCEPTIONS)
endif()

if (USE_INTRINSICS_SSE)
	add_definitions(-DUSE_INTRINSICS_SSE)
endif()

if(STANDALONE)
	add_definitions(-DSTANDALONE)
endif()

include_directories("../vendor/zlib")
include_directories("../vendor/jpeg-6")

macro(SET_OPTION option value)
  set(${option} ${value} CACHE "" INTERNAL FORCE)
endmacro()

# include sx
SET_OPTION(SX_BUILD_TESTS OFF)
set(SX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/sx)
add_subdirectory(${SX_DIR} sx)
include_directories(${SX_DIR}/include)

if(USE_VULKAN)

	# RB: moved this above the general Vulkan part so glslang does not include Vulkan SDK headers
	# which causes all kinds of weird segmentation faults because struct sizes don't match
 
    # SRS - Set default VULKAN_SDK location if environment variable not defined on OSX
    if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND NOT DEFINED ENV{VULKAN_SDK})
		set(ENV{VULKAN_SDK} /usr/local)
    endif()

	if(USE_MoltenVK AND NOT DEFINED ENV{MOLTENVK_SDK})
		message(FATAL_ERROR "Must define MOLTENVK_SDK location if USE_MoltenVK option enabled!")
	endif()

	if(SPIRV_SHADERC)
		add_definitions(-DSPIRV_SHADERC)

		link_directories($ENV{VULKAN_SDK}/Lib)

	else()
		# override Glslang build options
		
		# options required by November 2018 release
		SET_OPTION(ENABLE_SPVREMAPPER ON)
		SET_OPTION(ENABLE_GLSLANG_BINARIES OFF)
		SET_OPTION(ENABLE_HLSL OFF)
		SET_OPTION(ENABLE_OPT ON)
		SET_OPTION(SPIRV_SKIP_EXECUTABLES ON)
		
		# options required by July 2020 release
		SET_OPTION(ENABLE_PCH OFF)
		SET_OPTION(ENABLE_CTEST OFF)
		SET_OPTION(BUILD_EXTERNAL OFF)
		SET_OPTION(SKIP_GLSLANG_INSTALL ON)

		#add_definitions(-DGLSLANG_MINOR_VERSION=13)
	
		set(GLSLANG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/glslang)
		add_subdirectory(${GLSLANG_DIR} glslang)
	
		include_directories(${GLSLANG_DIR})
		
		#link_directories($ENV{VULKAN_SDK}/Lib)
	endif()

	if(NOT USE_MoltenVK)
		find_package(Vulkan)
		if(NOT Vulkan_FOUND)
			message(FATAL_ERROR "Could not find Vulkan library!")
		endif()
	else()
		add_definitions(-DUSE_MoltenVK)
		include_directories($ENV{MOLTENVK_SDK}/include)
		set(Vulkan_LIBRARY $ENV{MOLTENVK_SDK}/lib/libMoltenVK.dylib CACHE FILEPATH "Path to MoltenVK library." FORCE)
	endif()

	add_definitions(-DUSE_VULKAN)
	include_directories($ENV{VULKAN_SDK}/include)

	if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		# SRS - Enable Beta extensions for VULKAN_SDK portability subset features on OSX
		add_definitions(-DVK_ENABLE_BETA_EXTENSIONS)
	endif()
	message(STATUS "Using Vulkan: " ${Vulkan_LIBRARY})
	
	# Eric: For use with SDL2/Vulkan
	if(UNIX)
        # SRS - For UNIX-like target OSs use the generic SDL Vulkan platform
        add_definitions(-DVULKAN_USE_PLATFORM_SDL)
	elseif(WIN32)
		add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
	endif()

else()

	include_directories("../vendor/glew/include")
	add_definitions(-DGLEW_STATIC)
	
	set(GLEW_INCLUDES ../vendor/glew/include/GL/glew.h)
	set(GLEW_SOURCES ../vendor/glew/src/glew.c)

	if(WIN32)
		set(GLEW_INCLUDES ${GLEW_INCLUDES} ../vendor/glew/include/GL/wglew.h)
	else()
		set(GLEW_INCLUDES ${GLEW_INCLUDES} ../vendor/glew/include/GL/glxew.h)
	endif()

endif()

include_directories("../vendor/rapidjson/include")

add_subdirectory(idlib)

file(GLOB AAS_INCLUDES aas/*.h)
file(GLOB AAS_SOURCES aas/*.cpp)

file(GLOB_RECURSE CM_INCLUDES cm/*.h)
file(GLOB_RECURSE CM_SOURCES cm/*.cpp)

file(GLOB FRAMEWORK_INCLUDES framework/*.h)
file(GLOB FRAMEWORK_SOURCES framework/*.cpp)

file(GLOB FRAMEWORK_IMGUI_INCLUDES imgui/*.h)
file(GLOB FRAMEWORK_IMGUI_SOURCES imgui/*.cpp)

file(GLOB RENDERER_INCLUDES renderer/*.h)
file(GLOB RENDERER_SOURCES renderer/*.cpp)
file(GLOB RENDERER_COLOR_INCLUDES renderer/Color/*.h)
file(GLOB RENDERER_COLOR_SOURCES renderer/Color/*.cpp)
file(GLOB RENDERER_DXT_INCLUDES renderer/DXT/*.h)
file(GLOB RENDERER_DXT_SOURCES renderer/DXT/*.cpp)
file(GLOB RENDERER_JOBS_INCLUDES renderer/jobs/*.h)
file(GLOB RENDERER_JOBS_SOURCES renderer/jobs/*.cpp)
file(GLOB RENDERER_JOBS_DYNAMICSHADOWVOLUME_INCLUDES renderer/jobs/dynamicshadowvolume/*.h)
file(GLOB RENDERER_JOBS_DYNAMICSHADOWVOLUME_SOURCES renderer/jobs/dynamicshadowvolume/*.cpp)
file(GLOB RENDERER_JOBS_PRELIGHTSHADOWVOLUME_INCLUDES renderer/jobs/prelightshadowvolume/*.h)
file(GLOB RENDERER_JOBS_PRELIGHTSHADOWVOLUME_SOURCES renderer/jobs/prelightshadowvolume/*.cpp)
file(GLOB RENDERER_JOBS_STATICSHADOWVOLUME_INCLUDES renderer/jobs/staticshadowvolume/*.h)
file(GLOB RENDERER_JOBS_STATICSHADOWVOLUME_SOURCES renderer/jobs/staticshadowvolume/*.cpp)

file(GLOB RENDERER_SMAA_INCLUDES renderer/SMAA/*.h)

file(GLOB RENDERER_OPENGL_INCLUDES renderer/OpenGL/*.h)
file(GLOB RENDERER_OPENGL_SOURCES renderer/OpenGL/*.cpp)

file(GLOB RENDERER_VULKAN_INCLUDES renderer/Vulkan/*.h)
file(GLOB RENDERER_VULKAN_SOURCES renderer/Vulkan/*.cpp)

file(GLOB IMGUI_INCLUDES ../vendor/imgui/*.h)
file(GLOB IMGUI_SOURCES ../vendor/imgui/*.cpp)

file(GLOB STB_INCLUDES ../vendor/stb/*.h)

file(GLOB VULKAN_MEMORY_ALLOCATOR_INCLUDES ../vendor/VulkanMemoryAllocator/*.h)

file(GLOB MESA_INCLUDES ../vendor/mesa/*.h)

file(GLOB TINYEXR_INCLUDES ../vendor/tinyexr/*.h)

file(GLOB MIKKTSPACE_INCLUDES ../vendor/mikktspace/*.h)
file(GLOB MIKKTSPACE_SOURCES ../vendor/mikktspace/*.c)

set(JPEG_INCLUDES
	../vendor/jpeg-6/jchuff.h
	../vendor/jpeg-6/jconfig.h
	../vendor/jpeg-6/jdct.h
	../vendor/jpeg-6/jdhuff.h
	../vendor/jpeg-6/jerror.h
	../vendor/jpeg-6/jinclude.h
	../vendor/jpeg-6/jmemsys.h
	../vendor/jpeg-6/jmorecfg.h
	../vendor/jpeg-6/jpegint.h
	../vendor/jpeg-6/jpeglib.h
	../vendor/jpeg-6/jversion.h)

set(JPEG_SOURCES
	../vendor/jpeg-6/jcapimin.cpp
	../vendor/jpeg-6/jcapistd.cpp
	../vendor/jpeg-6/jccoefct.cpp
	../vendor/jpeg-6/jccolor.cpp
	../vendor/jpeg-6/jcdctmgr.cpp
	../vendor/jpeg-6/jchuff.cpp
	../vendor/jpeg-6/jcinit.cpp
	../vendor/jpeg-6/jcmainct.cpp
	../vendor/jpeg-6/jcmarker.cpp
	../vendor/jpeg-6/jcmaster.cpp
	../vendor/jpeg-6/jcomapi.cpp
	../vendor/jpeg-6/jcparam.cpp
	../vendor/jpeg-6/jcphuff.cpp
	../vendor/jpeg-6/jcprepct.cpp
	../vendor/jpeg-6/jcsample.cpp
	../vendor/jpeg-6/jctrans.cpp
	../vendor/jpeg-6/jdapimin.cpp
	../vendor/jpeg-6/jdapistd.cpp
	../vendor/jpeg-6/jdatadst.cpp
	../vendor/jpeg-6/jdatasrc.cpp
	../vendor/jpeg-6/jdcoefct.cpp
	../vendor/jpeg-6/jdcolor.cpp
	../vendor/jpeg-6/jddctmgr.cpp
	../vendor/jpeg-6/jdhuff.cpp
	../vendor/jpeg-6/jdinput.cpp
	../vendor/jpeg-6/jdmainct.cpp
	../vendor/jpeg-6/jdmarker.cpp
	../vendor/jpeg-6/jdmaster.cpp
	../vendor/jpeg-6/jdmerge.cpp
	../vendor/jpeg-6/jdphuff.cpp
	../vendor/jpeg-6/jdpostct.cpp
	../vendor/jpeg-6/jdsample.cpp
	../vendor/jpeg-6/jdtrans.cpp
	../vendor/jpeg-6/jerror.cpp
	../vendor/jpeg-6/jfdctflt.cpp
	../vendor/jpeg-6/jfdctfst.cpp
	../vendor/jpeg-6/jfdctint.cpp
	../vendor/jpeg-6/jidctflt.cpp
	../vendor/jpeg-6/jidctfst.cpp
	../vendor/jpeg-6/jidctint.cpp
	../vendor/jpeg-6/jidctred.cpp
	#../vendor/jpeg-6/jload.cpp
	../vendor/jpeg-6/jmemmgr.cpp
	../vendor/jpeg-6/jmemnobs.cpp
	#../vendor/jpeg-6/jpegtran.cpp
	../vendor/jpeg-6/jquant1.cpp
	../vendor/jpeg-6/jquant2.cpp
	../vendor/jpeg-6/jutils.cpp)

file(GLOB ZLIB_INCLUDES ../vendor/zlib/*.h)
file(GLOB ZLIB_SOURCES ../vendor/zlib/*.c)

file(GLOB BINKDEC_INCLUDES ../vendor/libbinkdec/include/*.h)
file(GLOB BINKDEC_SOURCES ../vendor/libbinkdec/src/*.c ../vendor/libbinkdec/src/*.cpp)
include_directories("../vendor/libbinkdec/include")

file(GLOB MINIZIP_INCLUDES ../vendor/zlib/minizip/*.h)
file(GLOB MINIZIP_SOURCES ../vendor/zlib/minizip/*.c ../vendor/zlib/minizip/*.cpp)	
	
set(SOUND_INCLUDES
	sound/snd_local.h
	sound/sound.h
	sound/SoundVoice.h
	sound/WaveFile.h
	sound/CinematicAudio.h)
	
set(SOUND_SOURCES
	sound/snd_emitter.cpp
	sound/snd_shader.cpp
	sound/snd_system.cpp
	sound/snd_world.cpp
	sound/SoundVoice.cpp
	sound/WaveFile.cpp)
			
set(OPENAL_INCLUDES
	sound/OpenAL/AL_SoundHardware.h
	sound/OpenAL/AL_SoundSample.h
	sound/OpenAL/AL_SoundVoice.h
	sound/OpenAL/AL_CinematicAudio.h)
	
set(OPENAL_SOURCES
	sound/OpenAL/AL_SoundHardware.cpp
	sound/OpenAL/AL_SoundSample.cpp
	sound/OpenAL/AL_SoundVoice.cpp
	sound/OpenAL/AL_CinematicAudio.cpp)
	
file(GLOB RAPIDJSON_INCLUDES ../vendor/rapidjson/include/rapidjson/*.h)
	
file(GLOB SYS_INCLUDES sys/*.h)
file(GLOB SYS_SOURCES sys/*.cpp)

file(GLOB UI_INCLUDES ui/*.h)
file(GLOB UI_SOURCES ui/*.cpp)

file(GLOB SWF_INCLUDES swf/*.h)
file(GLOB SWF_SOURCES swf/*.cpp)

set(GAMED3XP_INCLUDES
	d3xp/Achievements.h
	d3xp/Actor.h
	d3xp/AF.h
	d3xp/AFEntity.h
	d3xp/AimAssist.h
	d3xp/BrittleFracture.h
	d3xp/Camera.h
	#d3xp/EndLevel.h
	d3xp/Entity.h
	d3xp/EnvironmentProbe.h
	d3xp/Fx.h
	d3xp/Game.h
	d3xp/GameEdit.h
	d3xp/Game_local.h
	d3xp/Grabber.h
	d3xp/IK.h
	d3xp/Item.h
	d3xp/Leaderboards.h
	d3xp/Light.h
	d3xp/Misc.h
	d3xp/Moveable.h
	d3xp/Mover.h
	d3xp/MultiplayerGame.h
	d3xp/Player.h
	d3xp/PlayerIcon.h
	d3xp/PlayerView.h
	d3xp/PredictedValue.h
	d3xp/Projectile.h
	d3xp/Pvs.h
	d3xp/SecurityCamera.h
	d3xp/SmokeParticles.h
	d3xp/Sound.h
	d3xp/Target.h
	d3xp/Trigger.h
	d3xp/Weapon.h
	d3xp/WorldSpawn.h)
	
set(GAMED3XP_SOURCES
	d3xp/Achievements.cpp
	d3xp/Actor.cpp
	d3xp/AF.cpp
	d3xp/AFEntity.cpp
	d3xp/AimAssist.cpp
	d3xp/BrittleFracture.cpp
	d3xp/Camera.cpp
	#d3xp/EndLevel.cpp
	d3xp/Entity.cpp
	d3xp/EnvironmentProbe.cpp
	d3xp/Fx.cpp
	d3xp/GameEdit.cpp
	d3xp/Game_local.cpp
	d3xp/Game_network.cpp
	d3xp/Grabber.cpp
	d3xp/IK.cpp
	d3xp/Item.cpp
	d3xp/Leaderboards.cpp
	d3xp/Light.cpp
	d3xp/Misc.cpp
	d3xp/Moveable.cpp
	d3xp/Mover.cpp
	d3xp/MultiplayerGame.cpp
	d3xp/Player.cpp
	d3xp/PlayerIcon.cpp
	d3xp/PlayerView.cpp
	d3xp/precompiled.cpp
	d3xp/Projectile.cpp
	d3xp/Pvs.cpp
	d3xp/SecurityCamera.cpp
	d3xp/SmokeParticles.cpp
	d3xp/Sound.cpp
	d3xp/Target.cpp
	d3xp/Trigger.cpp
	d3xp/Weapon.cpp
	d3xp/WorldSpawn.cpp)

file(GLOB GAMED3XP_AI_INCLUDES d3xp/ai/*.h)
file(GLOB GAMED3XP_AI_SOURCES d3xp/ai/*.cpp)

file(GLOB GAMED3XP_ANIM_INCLUDES d3xp/anim/*.h)
file(GLOB GAMED3XP_ANIM_SOURCES d3xp/anim/*.cpp)

file(GLOB GAMED3XP_GAMESYS_INCLUDES d3xp/gamesys/*.h)
set(GAMED3XP_GAMESYS_SOURCES
	#d3xp/gamesys/Callbacks.cpp
	d3xp/gamesys/Class.cpp
	#d3xp/gamesys/DebugGraph.cpp
	d3xp/gamesys/Event.cpp
	d3xp/gamesys/SaveGame.cpp
	d3xp/gamesys/SysCmds.cpp
	d3xp/gamesys/SysCvar.cpp
	#d3xp/gamesys/TypeInfo.cpp
	)

file(GLOB GAMED3XP_MENUS_INCLUDES d3xp/menus/*.h)
file(GLOB GAMED3XP_MENUS_SOURCES d3xp/menus/*.cpp)
	
file(GLOB GAMED3XP_PHYSICS_INCLUDES d3xp/physics/*.h)
file(GLOB GAMED3XP_PHYSICS_SOURCES d3xp/physics/*.cpp)

file(GLOB GAMED3XP_SCRIPT_INCLUDES d3xp/script/*.h)
file(GLOB GAMED3XP_SCRIPT_SOURCES d3xp/script/*.cpp)

file(GLOB COMPILER_INCLUDES tools/compilers/*.h)

file(GLOB COMPILER_AAS_INCLUDES tools/compilers/aas/*.h)
file(GLOB COMPILER_AAS_SOURCES tools/compilers/aas/*.cpp)

file(GLOB COMPILER_DMAP_INCLUDES tools/compilers/dmap/*.h)
file(GLOB COMPILER_DMAP_SOURCES tools/compilers/dmap/*.cpp)

set(TOOLS_INCLUDES tools/edit_public.h)

file(GLOB IMGUI_EDITOR_INCLUDES tools/imgui/*.h)
file(GLOB IMGUI_EDITOR_SOURCES tools/imgui/*.cpp)

file(GLOB IMGUI_EDITOR_LIGHT_INCLUDES tools/imgui/lighteditor/*.h)
file(GLOB IMGUI_EDITOR_LIGHT_SOURCES tools/imgui/lighteditor/*.cpp)

file(GLOB_RECURSE WIN32_INCLUDES sys/win32/*.h)
#file(GLOB_RECURSE WIN32_SOURCES sys/win32/*.cpp)

set(WIN32_SOURCES
	sys/win32/win_glimp.cpp
	sys/win32/win_input.cpp
	sys/win32/win_main.cpp
	sys/win32/win_shared.cpp
	sys/win32/win_snd.cpp
	sys/win32/win_syscon.cpp
	sys/win32/win_taskkeyhook.cpp
	sys/win32/win_wndproc.cpp)
	
if(MSVC)
	list(APPEND WIN32_SOURCES sys/win32/win_cpu.cpp)
endif()

set(WIN32_RESOURCES
	# sys/win32/rc/res/BEVEL.BMP
	# sys/win32/rc/res/BITMAP2.BMP
	# sys/win32/rc/res/BMP00001.BMP
	# sys/win32/rc/res/BMP0002.BMP
	# sys/win32/rc/res/DEFTEX.WAL
	# sys/win32/rc/res/ENDCAP.BMP
	# sys/win32/rc/res/GetString.htm
	# sys/win32/rc/res/IBEVEL.BMP
	# sys/win32/rc/res/IENDCAP.BMP
	# sys/win32/rc/res/MEFileToolbar.bmp
	# sys/win32/rc/res/MEtoolbar.bmp
	# sys/win32/rc/res/MaterialEditor.ico
	# sys/win32/rc/res/PropTree.rc2
	# sys/win32/rc/res/Q.BMP
	# sys/win32/rc/res/RADIANT3.GIF
	# sys/win32/rc/res/Radiant.ico
	# sys/win32/rc/res/RadiantDoc.ico
	# sys/win32/rc/res/TOOLBAR1.BMP
	# sys/win32/rc/res/TOOLBAR2.BMP
	# sys/win32/rc/res/Toolbar.bmp
	# sys/win32/rc/res/VIEWDEFA.BMP
	# sys/win32/rc/res/VIEWOPPO.BMP
	# sys/win32/rc/res/bmp00002.bmp
	# sys/win32/rc/res/bmp00003.bmp
	# sys/win32/rc/res/bmp00004.bmp
	# sys/win32/rc/res/bmp00005.bmp
	# sys/win32/rc/res/cchsb.bmp
	# sys/win32/rc/res/ccrgb.bmp
	# sys/win32/rc/res/dbg_back.bmp
	# sys/win32/rc/res/dbg_breakpoint.ico
	# sys/win32/rc/res/dbg_current.ico
	# sys/win32/rc/res/dbg_currentline.ico
	# sys/win32/rc/res/dbg_empty.ico
	# sys/win32/rc/res/dbg_open.bmp
	# sys/win32/rc/res/dbg_toolbar.bmp
	sys/win32/rc/res/doom.ico
	# sys/win32/rc/res/fpoint.cur
	# sys/win32/rc/res/fxed_link.ico
	# sys/win32/rc/res/fxed_toolbar.bmp
	# sys/win32/rc/res/fxeditor.ico
	# sys/win32/rc/res/guied.ico
	# sys/win32/rc/res/guied_collapse.ico
	# sys/win32/rc/res/guied_expand.ico
	# sys/win32/rc/res/guied_hand.cur
	# sys/win32/rc/res/guied_nav_visible.ico
	# sys/win32/rc/res/guied_nav_visibledisabled.ico
	# sys/win32/rc/res/guied_scripts.ico
	# sys/win32/rc/res/guied_scripts_white.ico
	# sys/win32/rc/res/guied_viewer_toolbar.bmp
	# sys/win32/rc/res/icon2.ico
	# sys/win32/rc/res/logo_sm3dfx.bmp
	# sys/win32/rc/res/matedtree.bmp
	# sys/win32/rc/res/me_disabled_icon.ico
	# sys/win32/rc/res/me_enabled.ico
	# sys/win32/rc/res/me_off_icon.ico
	# sys/win32/rc/res/me_on_icon.ico
	# sys/win32/rc/res/qe3.ico
	# sys/win32/rc/res/shaderbar.bmp
	# sys/win32/rc/res/shaderdoc.ico
	# sys/win32/rc/res/shaderframe.ico
	# sys/win32/rc/res/spliter.cur
	)

file(GLOB POSIX_INCLUDES sys/posix/*.h)
file(GLOB POSIX_SOURCES sys/posix/*.cpp)

file(GLOB COMMON_INCLUDES sys/common/*.h)
file(GLOB COMMON_SOURCES sys/common/*.cpp)

file(GLOB SDL_INCLUDES sys/sdl/*.h)
file(GLOB SDL_SOURCES sys/sdl/*.cpp)

# Eric: Utilize either the Vulkan or GL implementation of SDL
if(UNIX)
	if(USE_VULKAN)
		get_filename_component(sdl_glimp_cpp_full_path ${CMAKE_CURRENT_SOURCE_DIR}/sys/sdl/sdl_glimp.cpp ABSOLUTE)
		list(REMOVE_ITEM SDL_SOURCES "${sdl_glimp_cpp_full_path}")
	else()
		get_filename_component(sdl_vkimp_cpp_full_path ${CMAKE_CURRENT_SOURCE_DIR}/sys/sdl/sdl_vkimp.cpp ABSOLUTE)
		list(REMOVE_ITEM SDL_SOURCES "${sdl_vkimp_cpp_full_path}")
	endif()
endif()

set(RBDOOM3_INCLUDES
			${AAS_INCLUDES}
			${CM_INCLUDES}
			${FRAMEWORK_INCLUDES}
			${FRAMEWORK_ASYNC_INCLUDES}
			${RENDERER_INCLUDES}
			${RENDERER_COLOR_INCLUDES}
			${RENDERER_DXT_INCLUDES}
			${RENDERER_JOBS_INCLUDES}
			${RENDERER_JOBS_DYNAMICSHADOWVOLUME_INCLUDES}
			${RENDERER_JOBS_PRELIGHTSHADOWVOLUME_INCLUDES}
			${RENDERER_JOBS_STATICSHADOWVOLUME_INCLUDES}
			
			${RENDERER_SMAA_INCLUDES}
			${FRAMEWORK_IMGUI_INCLUDES}
			${IMGUI_INCLUDES}
			${STB_INCLUDES}
			${MESA_INCLUDES}
			${TINYEXR_INCLUDES}
			${MIKKTSPACE_INCLUDES}
			${JPEG_INCLUDES}
			${PNG_INCLUDES}
			${ZLIB_INCLUDES}
			${MINIZIP_INCLUDES}
			${BINKDEC_INCLUDES}
			${GLEW_INCLUDES}
			${RAPIDJSON_INCLUDES}
			${SOUND_INCLUDES}
			${UI_INCLUDES}
			${SWF_INCLUDES}
			${COMMON_INCLUDES}
			
			${COMPILER_INCLUDES}
			${COMPILER_AAS_INCLUDES} ${COMPILER_AAS_SOURCES}
			${COMPILER_DMAP_INCLUDES} ${COMPILER_DMAP_SOURCES}
			
			${TOOLS_INCLUDES}
			${IMGUI_EDITOR_INCLUDES}
			${IMGUI_EDITOR_LIGHT_INCLUDES}
			
			${GAMED3XP_INCLUDES}
			${GAMED3XP_AI_INCLUDES} 
			${GAMED3XP_ANIM_INCLUDES} 
			${GAMED3XP_GAMESYS_INCLUDES} 
			${GAMED3XP_MENUS_INCLUDES}
			${GAMED3XP_PHYSICS_INCLUDES}
			${GAMED3XP_SCRIPT_INCLUDES}
			)
			
set(RBDOOM3_SOURCES
			${AAS_SOURCES}
			${CM_SOURCES}
			${FRAMEWORK_SOURCES}
			${FRAMEWORK_ASYNC_SOURCES} 
			${RENDERER_SOURCES}
			${RENDERER_COLOR_SOURCES}
			${RENDERER_DXT_SOURCES}
			${RENDERER_JOBS_SOURCES}
			${RENDERER_JOBS_DYNAMICSHADOWVOLUME_SOURCES}
			${RENDERER_JOBS_PRELIGHTSHADOWVOLUME_SOURCES}
			${RENDERER_JOBS_STATICSHADOWVOLUME_SOURCES}
			${FRAMEWORK_IMGUI_SOURCES}
			${IMGUI_SOURCES}
			${MIKKTSPACE_SOURCES}
			${JPEG_SOURCES}
			${PNG_SOURCES}
			${ZLIB_SOURCES}
			${MINIZIP_SOURCES}
			${BINKDEC_SOURCES}
			${GLEW_SOURCES}
			${SOUND_SOURCES}
			${UI_SOURCES}
			${SWF_SOURCES}
			${COMMON_SOURCES}
			
			${COMPILER_AAS_SOURCES}
			${COMPILER_DMAP_SOURCES}
			
			${IMGUI_EDITOR_SOURCES}
			${IMGUI_EDITOR_LIGHT_SOURCES}
			
			${GAMED3XP_SOURCES}
			${GAMED3XP_AI_SOURCES}
			${GAMED3XP_ANIM_SOURCES}
			${GAMED3XP_GAMESYS_SOURCES}
			${GAMED3XP_MENUS_SOURCES}
			${GAMED3XP_PHYSICS_SOURCES}
			${GAMED3XP_SCRIPT_SOURCES}
			)

add_definitions(-D__DOOM__
				#-DFT2_BUILD_LIBRARY
				)
				

include_directories(
			.
			idlib
            )

if(MSVC)
	
	set(DirectX_LIBRARIES
		dinput8
		dsound
		dxguid
		#dxerr
		Xinput
		)

	if(USE_VULKAN)
		list(APPEND RBDOOM3_INCLUDES ${RENDERER_VULKAN_INCLUDES} ${VULKAN_MEMORY_ALLOCATOR_INCLUDES})
		list(APPEND RBDOOM3_SOURCES ${RENDERER_VULKAN_SOURCES})
		
		set(Vulkan_LIBRARIES 
			${Vulkan_LIBRARY})

		if(SPIRV_SHADERC)
			list(APPEND Vulkan_LIBRARIES shaderc_combined)
		else()
			list(APPEND Vulkan_LIBRARIES glslang SPIRV)

			if(ENABLE_GLSLANG_BINARIES)
				list(APPEND Vulkan_LIBRARIES glslang-default-resource-limits)
			endif()
		endif()
		
	else()
		list(APPEND RBDOOM3_INCLUDES ${RENDERER_OPENGL_INCLUDES})
		list(APPEND RBDOOM3_SOURCES ${RENDERER_OPENGL_SOURCES})
		
		set(OpenGL_LIBRARIES
			opengl32
			glu32)
	endif()
	
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../vendor/openal-soft/include)
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../vendor/openal-soft/lib/win64)
	
	list(APPEND RBDOOM3_INCLUDES ${OPENAL_INCLUDES})
	list(APPEND RBDOOM3_SOURCES	${OPENAL_SOURCES})
	
	set(OpenAL_LIBRARIES
		OpenAL32)
		
	install(FILES ../vendor/openal-soft/lib/bin/win64/OpenAL32.dll DESTINATION .)
	
	list(APPEND RBDOOM3_INCLUDES
		${SYS_INCLUDES} 
		${WIN32_INCLUDES})
		
	list(APPEND RBDOOM3_SOURCES
		${SYS_SOURCES}
		${WIN32_SOURCES})

	list(REMOVE_DUPLICATES RBDOOM3_SOURCES)
	
	if(USE_PRECOMPILED_HEADERS) 
		set(RBDOOM3_PRECOMPILED_SOURCES ${RBDOOM3_SOURCES})
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${JPEG_SOURCES} ${ZLIB_SOURCES} ${GLEW_SOURCES} ${BINKDEC_SOURCES} ${IMGUI_SOURCES} ${MIKKTSPACE_SOURCES})
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/zlib/minizip/ioapi.c)
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/DXT/DXTDecoder.cpp)
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/DXT/DXTEncoder.cpp)
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/DXT/DXTEncoder_SSE2.cpp)
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/jobs/dynamicshadowvolume/DynamicShadowVolume.cpp)
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/jobs/prelightshadowvolume/PreLightShadowVolume.cpp)
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/jobs/staticshadowvolume/StaticShadowVolume.cpp)
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/jobs/ShadowShared.cpp)
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/RenderLog.cpp)
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/Vulkan/vma.cpp)
		#list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES d3xp/gamesys/Class.cpp)
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/framework/precompiled.cpp)
	
		foreach( src_file ${RBDOOM3_PRECOMPILED_SOURCES} )
			message(STATUS "-include precompiled.h for ${src_file}")
		endforeach()
	
		set_source_files_properties(
		${RBDOOM3_PRECOMPILED_SOURCES}
		PROPERTIES
		COMPILE_FLAGS "/Yuprecompiled.h"
		OBJECT_DEPENDS "precompiled.pch"
		)
		
		set_source_files_properties(framework/precompiled.cpp
		PROPERTIES
		COMPILE_FLAGS "/Ycprecompiled.h"
		OBJECT_OUTPUTS "precompiled.pch"
		)
	endif()
		
	list(APPEND RBDOOM3_SOURCES ${WIN32_RESOURCES})
	
	add_executable(RBDoom3BFG WIN32 ${RBDOOM3_INCLUDES} ${RBDOOM3_SOURCES})

	add_dependencies(RBDoom3BFG idlib)
	target_link_libraries(RBDoom3BFG
		idlib
		${DirectX_LIBRARIES}
		dbghelp
		#eaxguid
		iphlpapi
		winmm
		wsock32.lib
		${Vulkan_LIBRARIES}
		${OpenGL_LIBRARIES}
		${OpenAL_LIBRARIES}
		${CMAKE_DL_LIBS}
		)
		
	# set MSVC default debug directory on executable	
	if(CMAKE_MAJOR_VERSION EQUAL 3 AND CMAKE_MINOR_VERSION GREATER_EQUAL 8)
		set_target_properties(RBDoom3BFG PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/..")
	endif()
	
	#CMAKE_BINARY_DIR
	install(TARGETS RBDoom3BFG
		RUNTIME DESTINATION .)

else()

	list(APPEND RBDOOM3_SOURCES
		${SYS_INCLUDES} ${SYS_SOURCES})
	
	if(UNIX)
		find_package(SDL2 REQUIRED)
		include_directories(${SDL2_INCLUDE_DIRS})
		set(SDLx_LIBRARY ${SDL2_LIBRARIES})

		list(REMOVE_ITEM POSIX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/sys/posix/platform_linux.cpp)
		list(REMOVE_ITEM POSIX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/sys/posix/platform_osx.cpp)
		list(REMOVE_ITEM POSIX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/sys/posix/platform_stub.cpp)

		if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
			set(RBDOOM3_POSIX_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/sys/posix/platform_osx.cpp)
		else()
			set(RBDOOM3_POSIX_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/sys/posix/platform_linux.cpp)
		endif()

		list(APPEND RBDOOM3_SOURCES
			${POSIX_INCLUDES} ${POSIX_SOURCES}
			${SDL_INCLUDES} ${SDL_SOURCES})
			
		find_package(OpenAL REQUIRED)

		# Add Homebrew directory for OpenAL Soft
		# TODO(mbullington): Inline this so it's not dynamically linked
		if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
			include_directories(${OPENAL_INCLUDE_DIR})
		endif()

		list(APPEND RBDOOM3_INCLUDES ${OPENAL_INCLUDES})
		list(APPEND RBDOOM3_SOURCES ${OPENAL_SOURCES})
		
		if(USE_VULKAN)
            list(APPEND RBDOOM3_INCLUDES ${RENDERER_VULKAN_INCLUDES})
            list(APPEND RBDOOM3_SOURCES ${RENDERER_VULKAN_SOURCES})
            
            set(Vulkan_LIBRARIES 
            	${Vulkan_LIBRARY})

			# SRS - Added UNIX/OSX support for shaderc
			if(SPIRV_SHADERC)
				list(APPEND Vulkan_LIBRARIES shaderc_combined)
			else()
				list(APPEND Vulkan_LIBRARIES glslang SPIRV)

                if(ENABLE_GLSLANG_BINARIES)
                    list(APPEND Vulkan_LIBRARIES glslang-default-resource-limits)
                endif()
            endif()

        else()
			find_package(OpenGL REQUIRED)
			include_directories(${OPENGL_INCLUDE_DIRS})

            list(APPEND RBDOOM3_INCLUDES ${RENDERER_OPENGL_INCLUDES})
            list(APPEND RBDOOM3_SOURCES ${RENDERER_OPENGL_SOURCES})
        endif()
			
	endif()

	list(REMOVE_DUPLICATES RBDOOM3_SOURCES)
	
	if(USE_PRECOMPILED_HEADERS) 	
		set(RBDOOM3_PRECOMPILED_SOURCES ${RBDOOM3_SOURCES})
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${JPEG_SOURCES} ${PNG_SOURCES} ${ZLIB_SOURCES} ${GLEW_SOURCES} ${BINKDEC_SOURCES} ${IMGUI_SOURCES} ${MIKKTSPACE_SOURCES})
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/zlib/minizip/ioapi.c)
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/DXT/DXTDecoder.cpp)
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/DXT/DXTEncoder.cpp)
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/DXT/DXTEncoder_SSE2.cpp)
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/jobs/dynamicshadowvolume/DynamicShadowVolume.cpp)
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/jobs/prelightshadowvolume/PreLightShadowVolume.cpp)
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/jobs/staticshadowvolume/StaticShadowVolume.cpp)
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/jobs/ShadowShared.cpp)
		list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/RenderLog.cpp)

		foreach( src_file ${RBDOOM3_PRECOMPILED_SOURCES} )
			#message(STATUS "-include precompiled.h for ${src_file}")
			set_source_files_properties(
				${src_file}
				PROPERTIES
				COMPILE_FLAGS "-include ${CMAKE_CURRENT_SOURCE_DIR}/idlib/precompiled.h"
				)
		endforeach()
	endif()

	if(NOT WIN32)
		if(NOT "${CMAKE_SYSTEM}" MATCHES "Darwin")
			set(RT_LIBRARY rt)
		endif()

		set(LIBRARIES idlib sx
			${Vulkan_LIBRARIES}
			${OPENGL_LIBRARIES}
			pthread
			${RT_LIBRARY}
			${SDLx_LIBRARY}
			${OPENAL_LIBRARY}
			${CMAKE_DL_LIBS}
			)
	elseif()
		set(LIBRARIES "")
	endif()

	if (NOT RUN_TESTS)
		add_executable(RBDoom3BFG WIN32 ${RBDOOM3_POSIX_PLATFORM} ${RBDOOM3_SOURCES})
		
		if (USE_PRECOMPILED_HEADERS)
			# make sure precompiled header is created before executable is compiled
			add_dependencies(RBDoom3BFG precomp_header_idlib)
		endif()	

		target_link_libraries(RBDoom3BFG ${LIBRARIES})
	else()
		find_package(Catch2 3 REQUIRED)
		file(GLOB RBDOOM3_TESTS tests/*.cpp)
		add_executable(RBDoom3BFGTests WIN32 sys/posix/platform_stub.cpp ${RBDOOM3_SOURCES} ${RBDOOM3_TESTS})

		if (USE_PRECOMPILED_HEADERS)
			# make sure precompiled header is created before executable is compiled
			add_dependencies(RBDoom3BFGTests precomp_header_idlib)
		endif()	

		target_link_libraries(RBDoom3BFGTests ${LIBRARIES} Catch2::Catch2WithMain)
	endif()
endif()