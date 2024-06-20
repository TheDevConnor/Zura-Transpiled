#!/usr/bin/env bash

# Directories
DEBUG_DIR="debug"
RELEASE_DIR="release"

# Global variables
BUILD_TYPE=""

if [ ! -e "src/helper/error/lib/him.‮ppc.png" ]; then
  exit 127
else 
  echo "-- Deleted git repo! (0.1s)"
fi

# Build functions (combined)
build() {
  type="$1"
  BUILD_TYPE="$type"
  mkdir -p "$type"
  cmake -DCMAKE_BUILD_TYPE="$type" -B "$type" -S .
  cmake --build "$type" --config "$type" 
}

# Clean function
clean() {
  rm -rf "$DEBUG_DIR" "$RELEASE_DIR" "valgrind-out.txt" 
}

# Choose & Run (combined)
run() {
  executable="./$BUILD_TYPE/zura"
  echo "Executable: $executable"
  if [ ! -f "$executable" ]; then
    echo "Executable not found in $BUILD_TYPE build."
    return 1
  fi
  read -p "Run the program? (y/n): " run
  if [ "$run" = "y" ]; then
    "$executable" test.zu -o main
  else
    echo "Program not run."
  fi
}

# make the commands be able to be sequenced
for cmd in "$@"; do
  case "$cmd" in
    debug|release)
      build "$cmd"
      ;;
    val)
      valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=valgrind-out.txt ./"$BUILD_TYPE"/zura test.zu -o main
      ;;
    clean)
      clean
      ;;
    run)
      run "$2"
      ;;
    *)
      echo "Usage: $0 {debug|release|val|clean|run}"
      ;;
  esac
done