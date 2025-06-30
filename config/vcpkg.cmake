message("Setting up vcpkg...")
include(FetchContent)
FetchContent_Declare(
  vcpkg
  GIT_REPOSITORY https://github.com/microsoft/vcpkg.git
  GIT_SHALLOW TRUE
  SOURCE_DIR ${PROJECT_BINARY_DIR}
)
FetchContent_MakeAvailable(vcpkg)

if (WIN32)
  if (NOT VCPKG_TARGET_TRIPLET)
    set (VCPKG_TARGET_TRIPLET "x64-windows-release")
  endif ()
endif ()

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_BINARY_DIR}/_deps/vcpkg-src/scripts/buildsystems/vcpkg.cmake")
