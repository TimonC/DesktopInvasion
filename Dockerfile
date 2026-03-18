FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y \
        build-essential \
        cmake \
        qtbase5-dev \
        qtbase5-dev-tools \
        qtwayland5 \
        libwayland-client0 \
        libwayland-cursor0 \
        libwayland-egl1-mesa \
        inotify-tools \
        wget \
        qt5-qmake \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY build_and_run.sh /app/build_and_run.sh
COPY CMakeLists.txt /app/CMakeLists.txt
COPY resources.qrc /app/resources.qrc
COPY assets /app/assets
COPY src /app/src

RUN chmod +x /app/build_and_run.sh
ENTRYPOINT ["/app/build_and_run.sh"]
