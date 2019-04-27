cmake_minimum_required (VERSION 2.6)
project(SoundSim-target)

set(SDL2_DIR "${CMAKE_SOURCE_DIR}/extern/SDL2")
set(GLM_ROOT_DIR "${CMAKE_SOURCE_DIR}/extern/glm")
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall -march=native")

# Set these for outputting to a bin directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
 
find_package(GLM REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

# Automatically find all source files
file(GLOB SoundSim_SRC
	"src/*.h"
	"src/*.cpp"
)

include_directories(
	"./src"
	"./extern/SDL2/include"
)

add_executable(SoundSim-target
  extern/glad/glad.c
  ${SoundSim_SRC} # Add all source files in the src directory
)
 
target_include_directories(SoundSim-target PUBLIC
  ${GLM_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} "./extern/glad"
)

message("Output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message("SDL2_IMAGE_INCLUDE_DIRS: ${SDL2_IMAGE_INCLUDE_DIRS}")
message("SDL2_IMAGE_LIBRARIES: ${SDL2_IMAGE_LIBRARIES}")
message("SDL2_INCLUDE_DIRS: ${SDL2_INCLUDE_DIRS}")

target_link_libraries(SoundSim-target
  ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES}
)

function(copyToOutput copyCommand thingToCopy)
	message("Copying ${thingToCopy}")
	add_custom_command(TARGET SoundSim-target POST_BUILD COMMAND
		${CMAKE_COMMAND} -E ${copyCommand} ${thingToCopy} $<TARGET_FILE_DIR:SoundSim-target>)
endfunction()

if(WIN32)
	file(GLOB SDL2_DLLS "${CMAKE_SOURCE_DIR}/${SDL2_DIR}/lib/x64/*.dll")

	foreach(sdl_dll ${SDL2_DLLS})
		copyToOutput(copy_if_different ${sdl_dll})
	endforeach(sdl_dll)
endif()

copyToOutput(copy_directory ${ASSETS_DIR})
