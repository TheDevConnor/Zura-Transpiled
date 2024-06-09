#!/bin/sh

build_debug() {
    # check if debug folder exists
    if [ ! -d "debug" ]; then
        mkdir debug
    fi

    cd debug
    cmake -DCMAKE_BUILD_TYPE=Debug
    make
    cd ..
}

build_release() {
    # check if release folder exists
    if [ ! -d "release" ]; then
        mkdir release
    fi

    cd release
    cmake -DCMAKE_BUILD_TYPE=Release
    make
    cd ..
}

clean() {
    if [ -d "debug" ]; then
        rm -rf debug/CMakeFiles
        find debug -type f ! -name 'CMakeLists.txt' -delete
    fi

    if [ -d "release" ]; then
        rm -rf release/CMakeFiles
        find release -type f ! -name 'CMakeLists.txt' -delete
    fi
}

choice() {
    # ask user to choose debug or release
    echo "Choose build type: "
    echo "1. Debug"
    echo "2. Release"
    read -p "Enter your choice: " choice

    if [ "$choice" = "1" ]; then
        build_debug
        BUILD_TYPE="debug"
    elif [ "$choice" = "2" ]; then
        build_release
        BUILD_TYPE="release"
    else
        echo "Invalid choice"
        exit 1
    fi
}

run() {
    # call choice function
    choice

    # ask user to run the program
    read -p "Run the program? (y/n): " run 

    if [ "$run" = "y" ]; then
        if [ "$BUILD_TYPE" = "debug" ]; then
            if [ -f "./debug/zura" ]; then
                ./debug/zura test.zu -o main
            else
                echo "Debug build failed or executable not found."
            fi
        elif [ "$BUILD_TYPE" = "release" ]; then
            if [ -f "./release/zura" ]; then
                ./release/zura test.zu -o main
            else
                echo "Release build failed or executable not found."
            fi
        fi
    else
        echo "Program not run."
    fi
}

if [ "$1" = "debug" ]; then
    build_debug
elif [ "$1" = "release" ]; then
    build_release
elif [ "$1" = "clean" ]; then
    clean
elif [ "$1" = "run" ]; then
    run
else
    echo "Usage: $0 {debug|release|clean|run}"
fi
