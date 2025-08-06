# UniFly XPlane Plugin
This repository contains the code for the UniFly adapter plugin for XPlane compatibility with the UniFly app. This .xplm binary interfaces with the closed source UniFly App over TCP, implementing in the simulator various commands to spawn, despawn and reposition aircraft, using the XPMP2 library. This portion of the codebase has been open sourced in order to comply with licensing.

# Building
Requires an installation of protobuffers.

`cmake .`

`cmake --build .`
