# Microbit Lua

Lua compiled for the [micro:bit](https://microbit.org/) SBC.


# How

The
[microbit-v2-samples](https://github.com/lancaster-university/microbit-v2-samples)
repo has been forked, stripped, and modified to download Lua v5.1.5
and compile it as the main payload of the firmware. It builds upon
[CODAL](https://tech.microbit.org/software/runtime/) and exposes its
API to the Lua runtime.

When the board is powered on Lua gets initialized and the
`source/lua-script.lua` file is run.


## Building

The simplest and most reliable way to build it is by using the
included `Dockerfile`:

1) Build the container image: `podman build --platform linux/amd64 -t
   microbit -f Dockerfile .`

2) Build the firmware (from the project root): `podman run --tty --rm
   --volume "$(pwd)":/workspace --workdir /workspace microbit -c
   ./build.py`

3) Alternatively, you can start a shell and work inside the container:
   `podman run --tty --rm --interactive --volume "$(pwd)":/workspace
   --workdir /workspace microbit`

The current directory will be shared with the container.


## Flashing

The Microbit board exposes a pendrive-like interface. Mount it like
any other pendrive and just copy the `MICROBIT.hex` file to it. While
flashing the orange led next to the USB connector will be blinking
fast. Then the firmware is automatically started.


# Where

The project's home is at
[github.com/nagydani/microbit-lua](https://github.com/nagydani/microbit-lua).
