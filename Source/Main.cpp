/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include <complex>
#include "../JuceLibraryCode/JuceHeader.h"
#include "kissfft/kiss_fftr.h"



void createImage(String outPath, AudioSampleBuffer& buf, int h) {
    
    const float* src = buf.getReadPointer(0);
    
    int n = buf.getNumSamples();
    int nfft = n&1 ? n+1 : n; // must be even
    kiss_fftr_cfg cfg = kiss_fftr_alloc(static_cast<int>(nfft), false, NULL, NULL);
    kiss_fft_scalar ksrc[n];
    kiss_fft_cpx spec[nfft/2+1];
    for(int i=0; i<n; ++i) { ksrc[i] = src[i]; }
    if(nfft > n) { ksrc[nfft-1] = 0.f; }
    kiss_fftr(cfg, ksrc, spec);
    
    // image width is fixed to FFT size
    Image img(Image::PixelFormat::RGB, nfft, h, true);
    for(int x=0; x<nfft; ++x) {
        float val = std::abs(std::complex<float>(spec[x].r, spec[x].i));
        // convert to db
        val = 20.f * log10(val);
        val = 1.f - (val / -90.f);
        val = std::min(1.f, std::max(0.f, val));
        std::cout << val << "\n";
        const int y = (int)(val * h);
        int j = 0;
        while(j < h) {
            if(j < y) {
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
int main (int argc, char* argv[])
{
    String inPath;
    String outPath;
    int w, h;
    
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
    
    // 3rd arg: image width
    if(argc > 3) {
        w = atoi(argv[3]);
    } else {
        w = 120;
    }
    
    // 4th arg: image height
    if(argc > 4) {
        w = atoi(argv[4]);
    } else {
        h = 80;
    }
    File inFile = File::getCurrentWorkingDirectory().getChildFile(inPath);
    //File inFile(inPath);
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
    
    createImage(outPath, buf, h);
    return 0;
}
