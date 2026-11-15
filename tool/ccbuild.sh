#!/bin/bash

export CXX=/opt/homebrew/opt/llvm/bin/clang++
export CC=/opt/homebrew/opt/llvm/bin/clang

set -euo pipefail
IFS=$'\n\t'

BUILD_DIR="build"
BUILD_TYPE="Debug"

CPU_CORES="$(getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu)"

check_cmake() {
  if ! command -v cmake &>/dev/null; then
    echo "Error: cmake is not installed."
    return 1
  fi
}

# Read project name robustly (prefer CMakeCache if configured)
get_project_name() {
  if [[ -f "$BUILD_DIR/CMakeCache.txt" ]]; then
    awk -F= '/^CMAKE_PROJECT_NAME:STATIC=/{print $2; exit}' "$BUILD_DIR/CMakeCache.txt" || true
  else
    # Fallback: parse first project(...) occurrence
    grep -iE '^[[:space:]]*project\(' CMakeLists.txt \
      | head -n 1 \
      | sed -E 's/^[[:space:]]*project\(([^ )]+).*$/\1/I' \
      || true
  fi
}

configure() {
  check_cmake || return 1
  cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
}

xmode() {
  echo "Select build mode:"
  select choice in "Debug" "Release"; do
    case "$choice" in
      Debug|Release)
        BUILD_TYPE="$choice"
        echo "Build mode set to: $BUILD_TYPE"
        return 0
        ;;
      *)
        echo "Invalid selection."
        ;;
    esac
  done
}

xclean() {
  check_cmake || return 1
  if [[ ! -d "$BUILD_DIR" ]]; then
    echo "No build directory to clean."
    return 0
  fi
  echo "Cleaning build directory..."
  cmake --build "$BUILD_DIR" --target clean
}

xcleanall() {
  check_cmake || return 1
  echo "Performing full cleanup..."
  if [[ -d "$BUILD_DIR" ]]; then
    cmake --build "$BUILD_DIR" --target clean || true
  fi
  rm -rf "$BUILD_DIR" .cache
}

xbuild() {
  check_cmake || return 1

  configure

  local project_name
  project_name="$(get_project_name)"
  if [[ -z "${project_name}" ]]; then
    echo "Error: could not determine project name."
    return 1
  fi

  echo "Building main project: $project_name ($BUILD_TYPE mode)..."
  cmake --build "$BUILD_DIR" --target "$project_name" --parallel "$CPU_CORES"
}

xrun() {
  xbuild

  local project_name
  project_name="$(get_project_name)"
  local executable="./$BUILD_DIR/bin/$project_name"

  if [[ -x "$executable" ]]; then
    echo "Running $project_name..."
    "$executable"
  else
    echo "Error: executable not found: $executable"
    return 1
  fi
}

xrebuild() {
  xcleanall
  xrun
}

xtest() {
  check_cmake || return 1

  configure

  echo "Building tests..."
  cmake --build "$BUILD_DIR" --target tests --parallel "$CPU_CORES"

  # Preferred: run via ctest if tests are registered
  if [[ -f "$BUILD_DIR/CTestTestfile.cmake" ]]; then
    echo "Running tests via ctest..."
    ctest --test-dir "$BUILD_DIR" --output-on-failure
    return 0
  fi

  # Fallback: run executables named test_*
  echo "CTest not configured; running test_* executables directly..."
  while read -r test_exec; do
    echo
    echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Running $(basename "$test_exec")"
    "$test_exec" --reporter console --success --colour-mode ansi
  done < <(find "$BUILD_DIR/bin" -maxdepth 1 -type f -perm -111 -name 'test_*' | sort)
}

