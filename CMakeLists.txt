cmake_minimum_required(VERSION 3.16)
project(UniFly-XPLM LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY CXX_STANDARD_REQUIRED 17)
set_property(GLOBAL PROPERTY CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

set (INCLUDE_FMOD_SOUND 1)
add_compile_definitions(INCLUDE_FMOD_SOUND=1)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/inc/*.h)

include_directories(${CMAKE_SOURCE_DIR}/inc)
include_directories(${CMAKE_SOURCE_DIR}/dep/SDK/CHeaders/XPLM)


# Protobuf paths
set(Protobuf_INCLUDE_DIR "${ARCH_ROOT}/include")
set(Protobuf_LIBRARY "${ARCH_ROOT}/lib/libprotobuf.dylib")
set(Protobuf_PROTOC_EXECUTABLE "${ARCH_ROOT}/bin/protoc")
find_package(Protobuf CONFIG)

message(STATUS "Protobuf include dirs: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf libraries: ${Protobuf_LIBRARIES}")

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})

file(
    REAL_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/../crates/sim-xplane-ipc/schema/"
    PROTO_IMPORT_DIR
)
message("PROTO SCHEMAS = ${PROTO_IMPORT_DIR}")

file(GLOB PROTO_FILES "${PROTO_IMPORT_DIR}/*.proto")
message("PROTO FILES   = ${PROTO_FILES}")

protobuf_generate(
    LANGUAGE cpp
    PROTOS ${PROTO_FILES}
    IMPORT_DIRS ${PROTO_IMPORT_DIR}
    OUT_VAR PROTO_OUT
)

# Protobuf: Filter .h and .cc
message("PROTO_OUT     = ${PROTO_OUT}")

set(PROTO_HDRS "")
set(PROTO_SRCS "")
foreach(file ${PROTO_OUT})
  if(file MATCHES "\\.h$")
    list(APPEND PROTO_HDRS ${file})
  elseif(file MATCHES "\\.cc$")
    list(APPEND PROTO_SRCS ${file})
  endif()
endforeach()

message("PROTO SOURCE  = ${PROTO_SRCS}")
message("PROTO HEADERS = ${PROTO_HDRS}")

# Header & source files

set(INCLUDES
  inc/plugin.h
  inc/unifly.h
  inc/socket.h
  inc/utilities.h
  inc/data_ref_owned.h
  inc/data_ref_access.h
  inc/stdafx.h

  ${PROTO_HDRS}
)

source_group("Header Files" FILES ${INCLUDES})

set(SRC
  src/plugin.cpp
  src/unifly.cpp
  src/socket.cpp
  src/utilities.cpp
  src/data_ref_owned.cpp
  src/data_ref_access.cpp

  ${PROTO_SRCS}
)

source_group("Source Files" FILES ${SRC})
set(ALL_FILES ${INCLUDES} ${SRC})
add_library(${PROJECT_NAME} MODULE ${ALL_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME} protobuf::libprotobuf)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/dep)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_SOURCE_DIR}/dep/asio
    ${Protobuf_INCLUDE_DIRS}
)

if (APPLE)
    list(APPEND CMAKE_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/dep/SDK/Libraries/Mac")
elseif (WIN32)
    list(APPEND CMAKE_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/dep/SDK/Libraries/Win")
endif()
list(APPEND CMAKE_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dep/fmod")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/dep/XPMP2/inc")
add_subdirectory(dep/XPMP2)
add_dependencies(${CMAKE_PROJECT_NAME} XPMP2)
target_link_libraries(${CMAKE_PROJECT_NAME} XPMP2)

if (WIN32 OR APPLE)
    # FMOD for sound
    find_library(FMOD_LIBRARY NAMES fmod_vc.lib libfmod.dylib REQUIRED)
    message ("FMOD_LIBRARY                  = ${FMOD_LIBRARY}")

    # XPLM, the X-Plane SDK
    find_library(XPLM_LIBRARY NAMES XPLM XPLM_64.lib REQUIRED)
    target_link_libraries(${PROJECT_NAME} ${XPLM_LIBRARY} ${FMOD_LIBRARY})
endif ()

if (WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    target_link_libraries(${PROJECT_NAME} ws2_32.lib wldap32.lib advapi32.lib crypt32.lib wsock32.lib opengl32.lib iphlpapi.lib)
    target_link_options(${PROJECT_NAME} PUBLIC /DEBUG)
elseif (APPLE)
    # Link OS X core system libraries.
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation REQUIRED)
    find_library(Cocoa_LIBRARY Cocoa REQUIRED)
    find_library(Security_LIBRARY Security REQUIRED)
    find_library(GSS_LIBRARY GSS REQUIRED)
    find_library(OpenGL_LIBRARY OpenGL REQUIRED)

    target_link_libraries(
      ${PROJECT_NAME}
      ${CORE_FOUNDATION_LIBRARY}
      ${Cocoa_LIBRARY}
      ${Security_LIBRARY}
      ${GSS_LIBRARY}
      ${OpenGL_LIBRARY})

    # Restrict set of symbols exported from the plugin to the ones required by XPLM:
    target_link_libraries(${PROJECT_NAME} "-exported_symbols_list ${CMAKE_SOURCE_DIR}/src/unifly.sym_mac")
elseif (UNIX)
    # Link library for dynamic loading of shared objects on UNIX systems.
    find_library(DL_LIBRARY dl REQUIRED)
    # Threads
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} ${DL_LIBRARY} Threads::Threads)
    # Specify additional runtime search paths for dynamically-linked libraries.
    # Restrict set of symbols exported from the plugin to the ones required by XPLM:
    target_link_libraries(${PROJECT_NAME} -Wl,--version-script -Wl,${CMAKE_SOURCE_DIR}/src/unifly.sym)
endif ()

# Link OpenGL
set (OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)  # apt install freeglut3-dev
include_directories(${OpenGL_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${OpenGL_LIBRARIES})

# Enable all X-Plane SDK APIs up to the newest version.
add_definitions(-DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1 -DXPLM303=1)

# Define platform macros.
add_definitions(-DAPL=$<BOOL:${APPLE}> -DIBM=$<BOOL:${WIN32}> -DLIN=$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>>)

if (UNIX OR APPLE)
    add_compile_options(-fexceptions -fpermissive)
    add_compile_options(-fvisibility=hidden)
    add_compile_options(-Wall -Wshadow -Wfloat-equal -Wextra)
    add_compile_options(-Wno-unused)
    add_compile_options(-O3 -fPIC)
    add_compile_options(-pthread)
elseif(WIN32)
    add_compile_options(/wo4996)
    add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
endif()

if (MSVC)
   set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/win_x64")
   set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO	"${CMAKE_BINARY_DIR}/win_x64")
   set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/win_x64")
   set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/win_x64")
elseif (APPLE)
    # Because we're here building for two architectures and then putting them together we'll just leave it
   # set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/mac_x64")
elseif (UNIX)
   set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lin_x64")
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE "inc/stdafx.h")

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    PREFIX ""
    OUTPUT_NAME "UniFly-XPLM"
    SUFFIX ".xpl"
)
