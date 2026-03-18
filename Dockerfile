FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y \
        build-essential \
        cmake \
        ninja-build \
        wget \
        inotify-tools \
        # --- Qt 6 ---
        qt6-base-dev \
        qt6-declarative-dev \
        qt6-quick3d-dev \
        qt6-l10n-tools \
        qml6-module-qtqml \
        qml6-module-qtquick \
        qml6-module-qtquick-window \
        qml6-module-qtqml-workerscript \
        # --- X11 support ---
        libx11-dev \
        libx11-xcb-dev \
        libxcb1-dev \
        libxcb-keysyms1-dev \
        libxcb-image0-dev \
        libxcb-shm0-dev \
        libxcb-icccm4-dev \
        libxcb-render-util0-dev \
        libxcb-randr0-dev \
        libxcb-shape0-dev \
        libxcb-sync-dev \
        libxcb-xfixes0-dev \
        libxcb-xinerama0-dev \
        libxcb-xkb-dev \
        libxkbcommon-dev \
        libxkbcommon-x11-0 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY build_and_run.sh /app/build_and_run.sh
COPY CMakeLists.txt /app/CMakeLists.txt
COPY resources.qrc /app/resources.qrc
COPY assets /app/assets
COPY sprites /app/sprites
COPY InvasionCanvas.qml /app/InvasionCanvas.qml
COPY src /app/src

RUN chmod +x /app/build_and_run.sh

ENTRYPOINT ["/app/build_and_run.sh"]

