FROM stateoftheartio/qt6:6.8-gcc-aqt

USER root
ENV DEBIAN_FRONTEND=noninteractive
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8
ENV LANGUAGE=C.UTF-8

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
    libsqlite3-dev \
    valgrind \
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
COPY resources.qrc ./resources.qrc
COPY resources.dev.qrc ./resources.dev.qrc
COPY assets ./assets
COPY qml ./qml
COPY include ./include
COPY src ./src
COPY build_dev.sh ./build_dev.sh
COPY build_prod.sh ./build_prod.sh

RUN chmod +x ./build_dev.sh ./build_prod.sh
