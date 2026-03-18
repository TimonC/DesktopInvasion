#!/bin/bash

set -e

: "${XDG_RUNTIME_DIR:=/tmp/xdg}"
mkdir -p "$XDG_RUNTIME_DIR"
chmod 0700 "$XDG_RUNTIME_DIR"

if [ -f "resources.dev.qrc" ]; then
    cp resources.dev.qrc resources.qrc
fi
if [ -f "CMakeLists_dev.txt" ]; then
    cp CMakeLists_dev.txt CMakeLists.txt
fi

echo "First script execution - wiping volume"
rm -rf /app/data/* 2>/dev/null || true

if [ "$VALGRIND_MODE" = "1" ]; then
# docker compose run --rm -e VALGRIND_MODE=1 dev
    echo "Starting DesktopInvasion with Valgrind"
    mkdir -p /app/logs
    TIMESTAMP=$(date +%Y%m%d_%H%M%S)
    LOG_FILE="/app/logs/valgrind_${TIMESTAMP}.log"
    echo "Log will be saved to: $LOG_FILE (on host: ./valgrind_logs/)"

    mkdir -p build_valgrind
    cd build_valgrind
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-g -O0" ..
    ninja

    valgrind \
        --leak-check=full \
        --show-leak-kinds=all \
        --track-origins=yes \
        --log-file="$LOG_FILE" \
        ./bin/DesktopInvasion

    echo "Log saved. Check ./valgrind_logs/ on your host machine"
    ls -la /app/logs/

else
    # HMR mode
    mkdir -p build
    cd build
    cmake -G Ninja ..
    ninja

    echo "Starting DesktopInvasion with HMR"
    ./bin/DesktopInvasion &  # CHANGED: added bin/
    APP_PID=$!

    while inotifywait -r -e modify,create,delete ../src ../include ../qml; do
        echo -e "\n\033[1;33m========== HMR triggered at $(date '+%Y-%m-%d %H:%M') ==========\033[0m"

        kill $APP_PID 2>/dev/null || true
        ninja
        ./bin/DesktopInvasion &
        APP_PID=$!
    done

fi
