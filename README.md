# libstdaudio

This is an implementation of the standard audio API for C++ proposed in P1386.

The latest published revision of this proposal can always be found at [https://wg21.link/p1386]([https://wg21.link/p1386).

## Disclaimer

> This implementation is still a work in progress and may have some rough edges, but it does attempt to follow the API design
> presented in P1386. It does not exactly match P1386, though: as we discover issues and improvements, they might be implemented here first before they appear in the next P1386 revision.
>
> Currently, this implementation works on macOS. We plan to get Windows and Linux implementations done as soon as possible.

Personally, I think this proposal wouldn't accept recently, but I really like and want to use these interfaces, so this library will be a custom implementation, which uses [SLD3](https://github.com/libsdl-org/SDL) (very awesome Directmedia Layer, version 3 WIP, I like it due to it keep to be modern) for multi-platform in fact implementation.

Basically, this repo's custom backend is a P1386-like SLD3 C++ wrapper. So, may not send pr to upstream. Because it is no longer a header-only library. (except the proposal owner accepts that)

## Repository structure

`include` contains the `audio` header, which is the only header users of the library should include. It also contains the header files of the different classes and functions, prefixed with `__audio_`. Please refer to these header files for a documentation of the API as implemented here. (We plan to set up proper documentation soon.)

`examples` contains several example apps, which you can compile and run out of the box, and play around with to get a feel for how to use the library:

* `print_devices` lists the currently connected audio devices and their current configurations.

* `white_noise` plays white noise through the default output device.

* `sine_wave` plays a 440 Hz sine wave sound through the default output device.

* `melody` synthesises a short melody using a square wave generator, and plays it through the default output device.

* `level_meter` measures the input volume through the microphone, and continuously outputs the current maximum value on cout.

`test` contains some unit tests written in Catch2.

## How to use

This library uses CMake. It is header-only: simply include the `audio` header to use it. However, you must also link against the native audio backend to compile (see `CMAKE_EXE_LINKER_FLAGS` in `CMakeLists.txt`).
