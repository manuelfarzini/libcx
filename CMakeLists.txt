#-- UTILS --#

function(use_llvm_libcxx tgt)
  if(APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(LLVM_HOME "/opt/homebrew/opt/llvm")
    set(LIBCXX_INC   "${LLVM_HOME}/include/c++/v1")
    set(LIBCXX_LIBDIR "${LLVM_HOME}/lib/c++")
    set(LIBCXX_DYLIB "${LIBCXX_LIBDIR}/libc++.1.dylib")
    set(LIBCXXABI_DYLIB "${LIBCXX_LIBDIR}/libc++abi.1.dylib")
    if(EXISTS "${LIBCXX_INC}")
      target_compile_options(${tgt} PRIVATE
        -isystem "${LIBCXX_INC}"
      )
    endif()
    if(EXISTS "${LIBCXX_DYLIB}")
      target_link_options(${tgt} PRIVATE
        "-L${LIBCXX_LIBDIR}"
        "-Wl,-rpath,${LIBCXX_LIBDIR}"
      )
      if(EXISTS "${LIBCXXABI_DYLIB}")
        target_link_libraries(${tgt} PRIVATE
          "${LIBCXX_DYLIB}"
          "${LIBCXXABI_DYLIB}"
        )
      else()
        target_link_libraries(${tgt} PRIVATE
          "${LIBCXX_DYLIB}"
        )
      endif()
    endif()
  endif()
endfunction()

#-- PROJECT --#

cmake_minimum_required(VERSION 4.2.1)
set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++" CACHE FILEPATH "")
project(libcx)
enable_testing()

### C++ standard
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

### Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

### compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

### Compiler flags
set(CFLAGS "")
set(CFLAGS_DEBUG -g -O0 -D_DEBUG)
set(CFLAGS_RELEASE -O3 -DNDEBUG)
if(NOT MSVC)
  list(APPEND CFLAGS
    -Wall
    -Wextra
    -Wshadow
    -Werror
    -pedantic
    -Weffc++
    -fno-exceptions
    -fno-rtti
  )
else()
  list(APPEND CFLAGS /W4 /permissive-)
endif()

### Add fmt
set(_fmt_prefix "")
list(APPEND _fmt_prefix "/opt/homebrew")
list(APPEND _fmt_prefix "/usr/local")
list(APPEND _fmt_prefix "/usr")
find_package(fmt CONFIG QUIET
  HINTS ${_fmt_prefix}
  PATH_SUFFIXES
    lib/cmake/fmt
    lib64/cmake/fmt
)
if(NOT fmt_FOUND)
  find_package(fmt CONFIG REQUIRED)
endif()

### Define source files
file(GLOB CXX_SOURCES "src/*.cpp" "src/*.cc")

### Define target
add_executable(${PROJECT_NAME} ${CXX_SOURCES})

### Include headers
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_include_directories(${PROJECT_NAME} PRIVATE extern)
target_include_directories(${PROJECT_NAME} PRIVATE /opt/homebrew/include)
target_include_directories(${PROJECT_NAME} PRIVATE /Users/manuelfarzini/Personal/dev/opt)

### Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

### Apply cflags to target
target_compile_options(${PROJECT_NAME} PRIVATE ${CFLAGS})
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CONFIG:Debug>:${CFLAGS_DEBUG}>
  $<$<CONFIG:Release>:${CFLAGS_RELEASE}>
)

### Apply LLVM libc++ config (macOS+Clang)
use_llvm_libcxx(${PROJECT_NAME})


#-- TESTS --#

### Catch main library: custom main + Catch sources
add_library(catch_main_lib STATIC
  ${PROJECT_SOURCE_DIR}/test/catch/catch_main.cc
  ${PROJECT_SOURCE_DIR}/test/catch/catch_amalgamated.cc
)

### Common includes for all tests
target_include_directories(catch_main_lib PUBLIC
  ${PROJECT_SOURCE_DIR}/test
  ${PROJECT_SOURCE_DIR}/test/catch
  include
  extern
)

### Common compile flags for all tests
target_compile_options(catch_main_lib PUBLIC
  ${CFLAGS}
  $<$<CONFIG:Debug>:${CFLAGS_DEBUG}>
  $<$<CONFIG:Release>:${CFLAGS_RELEASE}>
  -Wno-unused
  -Wno-unused-parameter
)

# Apply LLVM libc++ config to the test support library
use_llvm_libcxx(catch_main_lib)

### Collect test sources
file(GLOB TEST_SOURCES "${PROJECT_SOURCE_DIR}/test/test_*.cc")

### Dummy target for all tests
add_custom_target(tests)

foreach(TEST_SRC ${TEST_SOURCES})
  get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
  add_executable(${TEST_NAME} ${TEST_SRC})
  target_link_libraries(${TEST_NAME} PRIVATE fmt::fmt catch_main_lib)
  use_llvm_libcxx(${TEST_NAME})
  add_dependencies(tests ${TEST_NAME})
  add_test(NAME ${TEST_NAME}
    COMMAND ${TEST_NAME} --reporter console --success --colour-mode ansi
  )

endforeach()
