# WaveSpecter

this is a smal command-line utility to make spectrum plot thumbnails from audio files.

it is written in C++,
uses JUCE for build framework, audio reading, and image writing,
uses kiss_fft for FFT.

tested on macos only at the moment.


## usage

`./wavespecter input [output] [height]`

the size of the DFT window is the number of samples in the input file is rounded up to an even number.

the output image width is fixed at one pixel per frequency bin. so width = (samples rounded up ) / 2 + 1.

output image height can be specified as a third

## plot characteristics

- amplitudes are plotted on a decibel scale with a -90dB floor.
- frequency scale is linear from DC - Nyquist.

## TODO
- make frequency scale logarithmic.


