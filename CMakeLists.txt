cmake_minimum_required (VERSION 3.15)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("VoxelCpp" LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE VOXELCPP_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE VOXELCPP_HEADERS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_executable(VoxelCpp)

target_sources(VoxelCpp PRIVATE ${VOXELCPP_SOURCES} ${VOXELCPP_HEADERS} "src/rendering/Renderer.cpp" "src/rendering/Window.cpp" "src/rendering/Pipeline.cpp" "src/rendering/Device.cpp" "src/rendering/Swapchain.cpp" "src/rendering/Model.cpp" "src/physics/RigidBody.cpp" "src/game/GameObject.cpp" "src/rendering/RenderSystem.cpp" "src/rendering/Camera.cpp")

target_include_directories(VoxelCpp
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Force libs compiled in release to work with debug mode
set_property(TARGET VoxelCpp PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
)

set_property(TARGET VoxelCpp PROPERTY CXX_STANDARD 20)
set_property(TARGET VoxelCpp PROPERTY CXX_STANDARD_REQUIRED ON)

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(VoxelCpp PUBLIC Vulkan::Vulkan)

# GLFW (vcpkg static)
set(GLFW_INCLUDE_DIR "C:/vcpkg/installed/x64-windows-static/include")
set(GLFW_LIBRARY     "C:/vcpkg/installed/x64-windows-static/lib/glfw3.lib")
target_include_directories(VoxelCpp PUBLIC ${GLFW_INCLUDE_DIR})
target_link_libraries(VoxelCpp PUBLIC ${GLFW_LIBRARY})

# Manual include of each lib is better than a recurse glob of all in /lib according to modern standards
add_library(KSC_Log STATIC IMPORTED GLOBAL)
set_target_properties(KSC_Log PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/KSC_Log.lib"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include/ksc_log"
)
target_link_libraries(VoxelCpp PRIVATE KSC_Log)

add_custom_target(Shaders
  COMMAND cmd.exe /c "${CMAKE_SOURCE_DIR}/compile_shaders.bat"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  VERBATIM
)

add_dependencies(VoxelCpp Shaders)