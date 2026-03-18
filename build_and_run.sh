#!/bin/bash
set -e

xhost +local:root 2>/dev/null || true

: "${XDG_RUNTIME_DIR:=/tmp/xdg}"
mkdir -p "$XDG_RUNTIME_DIR"
chmod 0700 "$XDG_RUNTIME_DIR"
/
rm -rf build/*
mkdir -p build
cd build
cmake ..
make

./DesktopInvasion &
APP_PID=$!

while inotifywait -r -e modify,create,delete ../src ../assets ../sprites; do
    echo -e "\n\033[1;33m========== HMR triggered at $(date '+%Y-%m-%d %H:%M') ==========\033[0m"

    # Stop old app
    kill $APP_PID 2>/dev/null || true

    # Rebuild
    cmake ..
    make

    # Restart app
    ./DesktopInvasion &
    APP_PID=$!
done

