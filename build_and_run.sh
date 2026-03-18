#!/bin/bash

# --- Ensure XDG_RUNTIME_DIR exists ---
: "${XDG_RUNTIME_DIR:=/tmp/xdg}"
mkdir -p "$XDG_RUNTIME_DIR"
chmod 0700 "$XDG_RUNTIME_DIR"

# --- Create build dir and build ---
# mkdir -p build
# cd build

# Initial CMake and build
cmake .
make

# --- Run app under XWayland ---
export QT_QPA_PLATFORM=xcb
./DesktopInvasion &
APP_PID=$!

# --- Watch for HMR (src, resources.qrc, and assets) ---
while inotifywait -r -e modify,create,delete src assets sprites; do
    echo -e "\n\033[1;33m========== HMR triggered at $(date '+%Y-%m-%d %H:%M') ==========\033[0m"

    # Stop old app
    kill $APP_PID 2>/dev/null

    # Force CMake to regenerate resources and rebuild
    cmake .
    make

    # Restart app under XWayland
    export QT_QPA_PLATFORM=xcb
    ./DesktopInvasion &
    APP_PID=$!
done

