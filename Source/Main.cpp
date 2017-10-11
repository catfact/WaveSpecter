/*
  ==============================================================================

   WaveSpecter
 
 command-line utility for making spectrum plot thumbnails from audio files.
 
 made very quickly by ezra buchla, october 2017
 
 uses JUCE and kiss_fft

  ==============================================================================
*/

#include <complex>
#include "../JuceLibraryCode/JuceHeader.h"
#include "kissfft/kiss_fftr.h"

//---- types,
typedef enum { LIN_FREQ, LOG_FREQ } freq_mode_t;

//---- utilities
// convert between hz (linear frequency) and midi (log2 frequency)
float hzmidi(float hz) {
    double r = hz / 440.0;
    return 69.f + log2(r) * 12.f;
}

float midihz(float midi) {
    double d = midi - 69.0;
    return 440.0 * pow(2.0, (d / 12.0));
}

// linear interpolation into a table using normalized phase in [0, 1]
// boundaries are fixed, not wrapped
float linInterp(float* tab, float x, int size) {
    x = std::min(1.f, std::max(0.f, x));
    x *= size;
    int ix = (int)x;
    float fx = x - (float)ix;
    float y0, y1;
    y0 = tab[ix];
    if(ix == (size-1)) {
        y1 = tab[size-1];
    } else {
        y1 = tab[ix + 1];
    }
    return y0 + fx*(y1 - y0);
}

// convert a linear frequency plot to a log2-scaled plot.
// arguments are two arrays, assumed to be the same size.
void convertLinPlotToLog(float *src, float *dst, int size, double min=0.0, double max=0.5) {
    // ok, for now assume that first index is zero on both scales,
    // that the first non-zero index is also equal,
    // and that the final index is equal.
    // for each entry in the output, we want the corresponding normalized index into the input,
    // which we then use for interpolation.
    // here's the value of the first non-zero linear index:
    double ix1 = max / (double)size;
    // this gives us an exponential coefficient;
    double c = pow(max/ix1, 1.0 / (double)(size-1));
    // and we can now iteratively build the log-scaled indices
    double iy = ix1;
    for(int i=0; i<size; ++i) {
        dst[i] = linInterp(src, iy/max, size);
        iy *= c;
    }
}


//----- app functions

// analyze a buffer with DFT and export a spectrum plot
void exportPlotImage(String outPath, AudioSampleBuffer& buf, int h, freq_mode_t freqMode) {
    
    const float* src = buf.getReadPointer(0);
    
    int n = buf.getNumSamples();
    int nfft = n&1 ? n+1 : n; // must be even
    int nr = nfft/2+1; // number of real frequency bands
    kiss_fftr_cfg cfg = kiss_fftr_alloc(static_cast<int>(nfft), false, NULL, NULL);
    kiss_fft_scalar ksrc[n];
    kiss_fft_cpx spec[nr];
    
    // create hann windowing function (raised cosine)
    float win[nfft];
    for(int i=0; i<nfft; ++i) {
        double t = 2.0 * M_PI * (double)i / (double)(nfft-1);
        double y = 0.5 - 0.5 * std::cos(t);
        win[i] = static_cast<float>(y);
    }
    // copy input and apply window
    for(int i=0; i<n; ++i) {
        ksrc[i] = src[i] * win[i];
    }
    
    // add a zero at the end if we rounded up
    if(nfft > n) { ksrc[nfft-1] = 0.f; }
    // run the fft
    kiss_fftr(cfg, ksrc, spec);
    
    float mag[nr];
    for(int x=0; x<nr; ++x) {
        float val = std::abs(std::complex<float>(spec[x].r, spec[x].i)) / (float)(nr);
        // convert to db
        val = 20.f * log10(val);
        val = 1.f - (val / -90.f);
        val = std::min(1.f, std::max(0.f, val));
        mag[x] = (int)(val * h);
    }
    
    if(freqMode == LOG_FREQ) {
        float logScaleMag[nr];
        convertLinPlotToLog(mag, logScaleMag, nr);
        // just copy the log-scaled version back... hacky
        for(int i=0; i<nr; ++i) {
            mag[i] = logScaleMag[i];
        }
    }
    
    // image width is fixed to FFT size
    Image img(Image::PixelFormat::RGB, nr, h, true);
    for(int x=0; x<nr; ++x) {
        const int y = mag[x];
        int j = 0;
        while(j < h) {
            if(j < y) {
                // NB: y axis is inverted (px offset from top edge)
                img.setPixelAt(x, h-1-j, Colour(0, 0, 0));
            } else {
                img.setPixelAt(x, h-1-j, Colour(255, 255, 255));
            }
            ++j;
        }
    }
    
    // rescale the image to the requested height and write it to disk
    File outFile = File::getCurrentWorkingDirectory().getChildFile(outPath);
    FileOutputStream stream (outFile);
    PNGImageFormat pngWriter;
    pngWriter.writeImageToStream(img, stream);
    stream.flush();
}

//==============================================================================
// ye maine
int main (int argc, char* argv[])
{
    freq_mode_t freqMode = LOG_FREQ;
    
    String inPath;
    String outPath;
    int h;
    
    // 1st arg (required) : input path
    if(argc > 1) {
        inPath = String(argv[1]);
    } else {
        std::cout << "warning: requires an input audio file; using test.wav \n";
        inPath = "test.wav";
    }
    
    // 2nd arg: output path
    if(argc > 2) {
        outPath = argv[2];
    } else {
        outPath = inPath + ".png";
    }
    
    // 3rd arg: lin / log
    if(argc > 3) {
        if(strcmp(argv[3], "lin") == 0) {
            freqMode = LIN_FREQ;
        }
    }
    
    // 4th arg: image height
    if(argc > 4) {
        h = atoi(argv[4]);
    } else {
        h = 80;
    }
    
    File inFile = File::getCurrentWorkingDirectory().getChildFile(inPath);
    AudioSampleBuffer buf;
    AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    ScopedPointer<AudioFormatReader> reader (formatManager.createReaderFor (inFile));
    
    if (reader != nullptr) {
        const int nsamps = static_cast<int>(reader->lengthInSamples);
        buf.setSize (reader->numChannels, nsamps);
        reader->read (&buf, 0, nsamps, 0, true, true);
    } else {
        std::cout << "error: failed to read audio file \n";
        return 1;
    }
    
    exportPlotImage(outPath, buf, h, freqMode);
    return 0;
}
