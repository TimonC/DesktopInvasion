FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y \
        build-essential \
        cmake \
        qt6-base-dev \
        qt6-declarative-dev \
        qt6-quick3d-dev \
        qt6-wayland \
        qt6-l10n-tools \
        qml6-module-qtqml \
        qml6-module-qtquick \
        qml6-module-qtquick-window \
        qml6-module-qtqml-workerscript \
        libwayland-client0 \
        libwayland-cursor0 \
        libwayland-egl1 \
        inotify-tools \
        wget \
        ninja-build \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/

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
