# UniFly XPlane Plugin
This repository contains the code for the UniFly adapter plugin for XPlane compatibility with the UniFly app. This open source `UniFly-XPLM.xpl` binary interfaces with the closed source UniFly App over TCP, implementing in the simulator various commands to spawn, despawn and reposition aircraft, using the XPMP2 library. This portion of the codebase is heavily based upon XPilot's XPLM plugin and therefore has been open sourced under GPL-3.0 in order to comply with licensing. The rest of the codebase is a seperate system independently developed in a different programming language.

# Building on MacOS
The macos build artefact is a universal binary with both arm64 & x86_64 architectures. We compile seperately under the two architectures and lipo them together afterwards. Therefore all dependencies must be available in both architectures. Good way to achieve this is to install homebrew twice, normal and rosetta, install protobuf and cmake on both homebrew installs, then build:

## Build x86_64
`arch -x86_64 /usr/local/bin/cmake -B build_x86_64 -S . -DCMAKE_PREFIX_PATH=/usr/local`

`arch -x86_64 /usr/local/bin/cmake --build build_x86_64`

## Build arm64
`arch -arm64 cmake -B build_arm64 -S . -DCMAKE_PREFIX_PATH=/opt/homebrew`

`arch -arm64 cmake --build build_arm64`

## Create universal binary
`lipo -create build_x86_64/UniFly-XPLM.xpl build_arm64/UniFly-XPLM.xpl -output UniFly-XPLM.xpl`
