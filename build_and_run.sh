#!/bin/bash
set -e

: "${XDG_RUNTIME_DIR:=/tmp/xdg}"
mkdir -p "$XDG_RUNTIME_DIR"
chmod 0700 "$XDG_RUNTIME_DIR"

if [ ! -f "/app/data/.initialized" ]; then
    echo "Initializing volume"
    rm -rf /app/data/* 2>/dev/null || true
    touch /app/data/.initialized
fi

mkdir -p build
cd build
cmake -G Ninja ..
ninja

echo "Starting DesktopInvasion with HMR"
./DesktopInvasion &
APP_PID=$!

while inotifywait -r -e modify,create,delete ../src ../include ../qml; do
    echo -e "\n\033[1;33m========== HMR triggered at $(date '+%Y-%m-%d %H:%M') ==========\033[0m"

    kill $APP_PID 2>/dev/null || true
    ninja
    ./DesktopInvasion &
    APP_PID=$!
done
