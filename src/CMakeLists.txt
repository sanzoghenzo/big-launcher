set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}")
set(SOURCES
  config.cpp
  gamepad.cpp
  hotkey.cpp
  image.cpp
  layout.cpp
  main.cpp
  menu.cpp
  menu_highlight.cpp
  renderer_sdl.cpp
  screensaver.cpp
  sidebar_entry.cpp
  sidebar_highlight.cpp
  sound.cpp
  text.cpp
  util.cpp
)

set(HEADERS
  config.hpp
  drawable.hpp
  gamepad.hpp
  hotkey.hpp
  image.hpp
  layout.hpp
  logger.hpp
  main.hpp
  menu.hpp
  menu_highlight.hpp
  object.hpp
  renderer.hpp
  renderer_sdl.hpp
  screensaver.hpp
  sidebar_entry.hpp
  sidebar_highlight.hpp
  sound.hpp
  text.hpp
  util.hpp
)
if (UNIX)
  add_executable(${EXECUTABLE_TITLE} ${SOURCES} ${HEADERS})
  target_link_libraries(${EXECUTABLE_TITLE} 
    PkgConfig::SDL3
    PkgConfig::SDL3_IMAGE
    PkgConfig::SDL3_TTF
    PkgConfig::LIBXML2 
    PkgConfig::FMT
    PkgConfig::SPDLOG
  )
elseif (WIN32)
  set(MANIFEST_FILE "${PROJECT_BINARY_DIR}/${EXECUTABLE_TITLE}.manifest")
  add_executable(${EXECUTABLE_TITLE} WIN32 ${SOURCES} ${HEADERS} ${MANIFEST_FILE})
  set_property(TARGET ${EXECUTABLE_TITLE} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_BINARY_DIR}")
  target_link_libraries(${EXECUTABLE_TITLE} 
  $<TARGET_NAME_IF_EXISTS:SDL3::SDL3main>
  $<IF:$<TARGET_EXISTS:SDL3::SDL3>,SDL3::SDL3,SDL3::SDL3-static>
  $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static>
	$<IF:$<TARGET_EXISTS:SDL3_ttf::SDL3_ttf>,SDL3_ttf::SDL3_ttf,SDL3_ttf::SDL3_ttf-static> 
	spdlog::spdlog
	LibXml2::LibXml2
	${GETOPT} 
	PowrProf
  )
  target_include_directories(${EXECUTABLE_TITLE} PUBLIC ${GETOPT_INCLUDE_DIR})
endif ()

add_subdirectory(platform)
add_subdirectory(external)
target_link_libraries(${EXECUTABLE_TITLE} platform)
target_link_libraries(${EXECUTABLE_TITLE} inih)
