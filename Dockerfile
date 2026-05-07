# To build an image:
#   $ podman build --platform linux/amd64 -t microbit -f Dockerfile .
#
# To enter that image:
#   $ cd /path/to/project/root
#   $ podman run --tty --rm --interactive --volume "$(pwd)":/workspace --workdir /workspace microbit
#
# To build it inside that image:
#   Note: image entry point is bash, hence the extra -c.
#   $ cd /path/to/project/root
#   $ podman run --tty --rm --volume "$(pwd)":/workspace --workdir /workspace microbit -c ./build.py
#
# Inside the image you can run build.py to build the project. The
# current directory from where you run podman will be shared into the
# image under /workspace, which will also be the default directory
# when running the image. This means that the built MICROBIT.hex file
# will be available in the source folder outside the image.

FROM ubuntu:24.04 AS builder

RUN apt-get update -qq && \
    apt-get install -y --no-install-recommends \
      software-properties-common \
      git \
      make \
      cmake \
      less \
      python3 \
      python3-pip \
      gcc-arm-none-eabi \
      binutils-arm-none-eabi \
      libnewlib-arm-none-eabi \
      libstdc++-arm-none-eabi-newlib \
      ca-certificates && \
    apt-get autoremove -y && \
    apt-get clean -y && \
    rm -rf /var/lib/apt/lists/*

# Project sources volume should be mounted at /app
# COPY . /opt/microbit-samples
# WORKDIR /opt/microbit-samples

# RUN python3 build.py

# FROM scratch AS export-stage
# COPY --from=builder /opt/microbit-samples/MICROBIT.bin .
# COPY --from=builder /opt/microbit-samples/MICROBIT.hex .

ENTRYPOINT ["/bin/bash"]
