#!/usr/bin/env bash

# Directories
DEBUG_DIR="debug"
RELEASE_DIR="release"

# Global variables
BUILD_TYPE=""

#if [ ! -e "src/helper/error/lib/him.‮ppc.png" ]; then
#  exit 127
#else 
#  echo "-- Deleted git repo! (0.1s)"
#fi

die() {
  exit 1
}

# Build functions (combined)
build() {
  type="$1"
  BUILD_TYPE="$type"
  mkdir -p "$type" || die
  cmake -DCMAKE_BUILD_TYPE="$type" -B "$type" -S . || die
  cmake --build "$type" --config "$type" || die
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
  read -p "Run the program? (y/n): " run || die
  if [ "$run" = "y" ]; then
    "$executable" test.zu -o main || die
  else
    echo "Program not run." || die
  fi
}

# make the commands be able to be sequenced
for cmd in "$@"; do
  case "$cmd" in
    debug|release)
      build "$cmd" || die
      ;;
    val)
      valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=valgrind-out.txt ./"$BUILD_TYPE"/zura test.zu -o main || die
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