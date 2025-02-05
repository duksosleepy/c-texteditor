# ———————————————————————————
# standard options
# ———————————————————————————
cmake_minimum_required(VERSION 3.22)
project(kilo DESCRIPTION "text editor" LANGUAGES C)  # Changed to C
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

option(ONLY_COVERAGE "Build only tests necessary for coverage" FALSE)
option(ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" FALSE)
option(ENABLE_ASAN "Enable address sanitizer" FALSE)
option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" FALSE)
option(ENABLE_TESTING "Enable the building of the test" TRUE)
option(ENABLE_CLANG_TIDY "Enable testing with clang-tidy" FALSE)
option(ENABLE_CPPCHECK "Enable testing with cppcheck" FALSE)
option(SIMPLE_BUILD "Build the project as minimally as possible" FALSE)
option(BUILD_DOC "Build the project's documentation" ON)
option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
option(DEBUG_LOGGING "Enabling debug logging" FALSE)
option(OPTIMIZE "Build the project with compiler optimization" FALSE)
option(DEBUG "Build the project in debugging mode" FALSE)
option(GPROF "Build the project for gprof (requires DEBUG)" FALSE)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
set(CPACK_SOURCE_IGNORE_FILES
    /.git
    /.vscode
    /.*build.*
    /\\\\.gitignore
    /\\\\.DS_Store
)
include(CPack)

if (MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Debug option that also enables asserts and profiling
if (DEBUG OR CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(
    APP_DEBUG APP_ENABLE_ASSERTS APP_PROFILE)
endif ()

# ———————————————————————————
# compiler options
# ———————————————————————————
add_library(project_warnings INTERFACE)
add_library(project_options INTERFACE)

if(ONLY_COVERAGE OR ENABLE_COVERAGE)
    target_compile_options(project_options INTERFACE --coverage -O0 -g)
    target_link_libraries(project_options INTERFACE --coverage)
endif()

if(ENABLE_ASAN)
    target_compile_options(project_options INTERFACE -fsanitize=address)
    target_link_libraries(project_options INTERFACE -fsanitize=address)
endif()

if(OPTIMIZE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -DNDEBUG")  # Changed to CMAKE_C_FLAGS
endif()

if(DEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -DDEBUG -DPERF")  # Changed to CMAKE_C_FLAGS
endif()

if(GPROF)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg -DGPROF")  # Changed to CMAKE_C_FLAGS
    set(GPROFFLAG "-pg")
endif()

function(set_project_warnings project_name)
  option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" TRUE)
  message(STATUS "Treat compiler warnings as errors")

  set(MSVC_WARNINGS
    /W4
    /WX
  )

  set(CLANG_WARNINGS
    -Wall
    -Wextra
    -Wshadow
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    -Wformat=2
  )

  if (WARNINGS_AS_ERRORS)
    set(CLANG_WARNINGS ${CLANG_WARNINGS} -Werror)
    set(MSVC_WARNINGS ${MSVC_WARNINGS} /WX)
  endif ()

  set(GCC_WARNINGS
    ${CLANG_WARNINGS}
    -Wmisleading-indentation
    -Wduplicated-cond
    -Wlogical-op
  )

  if (MSVC)
    set(PROJECT_WARNINGS ${MSVC_WARNINGS})
  elseif (CMAKE_C_COMPILER_ID MATCHES ".*Clang")
    set(PROJECT_WARNINGS ${CLANG_WARNINGS})
  elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(PROJECT_WARNINGS ${GCC_WARNINGS})
  else ()
    message(AUTHOR_WARNING "No compiler warnings set for '${CMAKE_C_COMPILER_ID}' compiler.")
  endif ()

  target_compile_options(${project_name} INTERFACE ${PROJECT_WARNINGS})
endfunction()

set_project_warnings(project_warnings)

if (${FORCE_COLORED_OUTPUT})
    if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
        target_compile_options(project_options INTERFACE -fdiagnostics-color=always)
    elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
        target_compile_options(project_options INTERFACE -fcolor-diagnostics)
    endif ()
endif ()

find_program(CCACHE ccache)
if(CCACHE)
    message("using ccache")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE}")  # Changed to CMAKE_C_COMPILER_LAUNCHER
else()
    message("ccache not available")
endif()

# ———————————————————————————
# documentation
# ———————————————————————————
if(BUILD_DOC)
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        add_custom_target(doc ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Building Doxygen documentation"
            VERBATIM)
    else ()
        message("No doxygen binary found on the system.")
        set(${BUILD_DOC} OFF)
    endif ()
endif()

# ———————————————————————————
# testing
# ———————————————————————————
if(ENABLE_TESTING)
    enable_testing()
endif()


message("###############################################################")
message("\t \t                  Summary")
message("###############################################################")
message("Build type:       \t ${CMAKE_BUILD_TYPE}")
message("Install prefix:   \t ${CMAKE_INSTALL_PREFIX}")
message("Testing enabled:  \t ${ENABLE_TESTING}")
message("Clang-tidy:       \t ${ENABLE_CLANG_TIDY}")
message("Cppcheck:         \t ${ENABLE_CPPCHECK}")
message("Compiler:         \t ${CMAKE_C_COMPILER_ID}")  # Changed to CMAKE_C_COMPILER_ID
message("Sanitizers:       \t ${ENABLE_ASAN}")
message("Shared libs:      \t ${BUILD_SHARED_LIBS}")
message("CCache executable:\t ${CCACHE}")
message("Building doc:     \t ${BUILD_DOC}")
message("---------------------------------------------------------------")
message("Version:          \t ${PROJECT_VERSION}")
message("C compiler:       \t ${CMAKE_C_COMPILER}")  # Changed to CMAKE_C_COMPILER
message("Compiler id:      \t ${CMAKE_C_COMPILER_ID}")  # Changed to CMAKE_C_COMPILER_ID
message("Compiler version: \t ${CMAKE_C_COMPILER_VERSION}")  # Changed to CMAKE_C_COMPILER_VERSION
message("###############################################################")

add_executable(kilo src/kilo.c)
