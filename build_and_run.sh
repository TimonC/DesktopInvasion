#!/bin/bash

# --- Ensure XDG_RUNTIME_DIR exists ---
: "${XDG_RUNTIME_DIR:=/tmp/xdg}"   # default if empty
mkdir -p "$XDG_RUNTIME_DIR"
chmod 0700 "$XDG_RUNTIME_DIR"

# --- Create build dir and build ---
mkdir -p build
cd build
cmake ..
make

# --- Run app under XWayland ---
export QT_QPA_PLATFORM=xcb       # force XWayland
./DesktopInvasion &
APP_PID=$!

# --- Watch for HMR ---
while inotifywait -r -e modify,create,delete ../src; do
    echo -e "\n\033[1;33m========== HMR triggered at $(date '+%Y-%m-%d %H:%M') ==========\033[0m"

    kill $APP_PID 2>/dev/null       # stop old app

    cmake ..                        # rebuild
    make

    # restart app under XWayland
    export QT_QPA_PLATFORM=xcb
    ./DesktopInvasion &
    APP_PID=$!
done

