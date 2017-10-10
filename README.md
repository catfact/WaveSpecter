# WaveSpecter

this is a small command-line utility to make spectrum plot thumbnails from audio files.

it is written in C++,
uses JUCE for build framework, audio reading, and image writing,
uses kiss\_fft for FFT.

tested on macos only at the moment.


## usage

`./wavespecter input [output] [frequency scale] [height]`

in the absence of an output filename, `.png` will be appended to the input filename.

oh yeah - the output is always .png, doesn't matter what extension you put. (ha!)

frequency scale is logarithmic by default.  enter`lin` in the 3rd argument to use a linear scale instead. anything but `lin` results in log-scale output. (and it's case sensitive. ha!)

the size of the DFT window is the number of samples in the input file, rounded up to the nearest even number.

the output image width is fixed at one pixel per frequency bin. so width = (samples rounded up ) / 2 + 1.

output image height can be specified as a fourth argument (integer number of pixels, defaults to 80.)


## plot characteristics

- amplitudes are plotted on a decibel scale with a -90dB floor.
- frequency range from DC - Nyquist. for log scaling, the first non-zero entry is the same as for the linear scale (nyquist / num bands,) and the zero entry is still zero. 

