# libstdaudio

This is the first draft of a CoreAudio implementation of the planned Audio TS for C++. 

This is a private repository and work in progess. Please do not share.

`include` contains the "audio.h" header, which is the only header users of the library should include.

`examples` contains three example apps:

* `print_devices` lists the currently connected audio devices.

* `white_noise` plays white noise through the default output device.

* `level_meter` measures the input volume through the microphone, and outputs the current value on cout every second.

`test` contains some unit tests written in Catch2.