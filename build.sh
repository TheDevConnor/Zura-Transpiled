#!/usr/bin/env bash

# Directories
DEBUG_DIR="debug"
RELEASE_DIR="release"

# Global variables
BUILD_TYPE=""

die() {
  exit 1
}

# Build functions (combined)
build() {
  type="$1"
  BUILD_TYPE="$type"
  mkdir -p "$type" || die
  cmake -G Ninja -DCMAKE_BUILD_TYPE="$type" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B "$type" -S . || die
  ninja -C "$type" || die
}

# Clean function
clean() {
  rm -rf "$DEBUG_DIR" "$RELEASE_DIR" "valgrind-out.txt" || die
}

# Choose & Run (combined)
run() {
  executable="./$BUILD_TYPE/zura"
  echo "Executable: $executable" || die
  if [ ! -f "$executable" ]; then
    echo "Executable not found in $BUILD_TYPE build." || die
    return 1
  fi
  "$executable" build zura_files/main.zu -save -name main|| die
}

# make the commands be able to be sequenced
for cmd in "$@"; do
  case "$cmd" in
    debug|release)
      build "$cmd" || die
      ;;
    val)
      valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=valgrind-out.txt ./"$BUILD_TYPE"/zura build zura_files/main.zu -name main || die
      ;;
    clean)
      clean
      ;;
    run)
      run "$2" || die
      ;;
    *)
      echo "Usage: $0 {debug|release|val|clean|run}" || die
      ;;
  esac
done