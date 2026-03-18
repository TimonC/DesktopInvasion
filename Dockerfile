FROM ubuntu:24.04

USER root
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    inotify-tools \
    libx11-xcb-dev \
    libgl1-mesa-dev \
    libvulkan-dev \
    fuse \
    libfuse2 \
    wget \
    file \
    squashfs-tools \
    qt6-base-dev \
    qt6-declarative-dev \
    qt6-tools-dev \
    libqt6sql6-sqlite \
    qml6-module-qtquick-layouts \
    qml6-module-qtqml-workerscript \
    qml6-module-qtquick-dialogs \
    qml6-module-qtquick-controls \
    qml6-module-qtquick-templates \
    qml6-module-qtquick-shapes \
    qml6-module-qtquick-particles \
    qml6-module-qtquick-window \
    qml6-module-qtquick-localstorage \
    && rm -rf /var/lib/apt/lists/*

RUN wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage -O /tmp/ld.AppImage \
    && chmod +x /tmp/ld.AppImage \
    && cd /tmp && /tmp/ld.AppImage --appimage-extract \
    && mv /tmp/squashfs-root /opt/linuxdeploy \
    && rm /tmp/ld.AppImage

RUN wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage -O /tmp/qt.AppImage \
    && chmod +x /tmp/qt.AppImage \
    && cd /tmp && /tmp/qt.AppImage --appimage-extract \
    && mv /tmp/squashfs-root /opt/linuxdeploy-plugin-qt \
    && rm /tmp/qt.AppImage


ENV PATH="/opt/linuxdeploy/usr/bin:/opt/linuxdeploy:$PATH"
ENV PATH="/opt/linuxdeploy-plugin-qt/usr/bin:/opt/linuxdeploy-plugin-qt:$PATH"


RUN ln -sf /opt/linuxdeploy/AppRun /usr/local/bin/linuxdeploy && \
    ln -sf /opt/linuxdeploy-plugin-qt/AppRun /usr/local/bin/linuxdeploy-plugin-qt


# Copy project files
WORKDIR /app
COPY CMakeLists.txt ./CMakeLists.txt
COPY assets ./assets
COPY qml ./qml
COPY include ./include
COPY src ./src
COPY resources.qrc ./resources.qrc
COPY build_dev.sh ./build_dev.sh
COPY build_prod.sh ./build_prod.sh

RUN chmod +x ./build_dev.sh ./build_prod.sh
