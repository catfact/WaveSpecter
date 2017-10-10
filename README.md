# WaveSpecter

this is a small command-line utility to make spectrum plot thumbnails from audio files.

it is written in C++,
uses JUCE for build framework, audio reading, and image writing,
uses kiss\_fft for FFT.

tested on macos only at the moment.


## usage

`./wavespecter input [output] [frequency scale] [height]`

frequency scale is logarithmic by default.  enter`lin` in the 3rd argument to use a linear scale instead.

the size of the DFT window is the number of samples in the input file< is, rounded up to the nearest even number.

output image height can be specified as a third argument (integer number of pixels, defaults to 80.)

the output image width is fixed at one pixel per frequency bin. so width = (samples rounded up ) / 2 + 1.

## plot characteristics

- amplitudes are plotted on a decibel scale with a -90dB floor.
- frequency scale is linear from DC - Nyquist.

## TODO
- make frequency scale logarithmic.


