# sofapan (under development)
Binaural Renderer Plugin. Can load HRTFs in the SOFA file format and customize it. Audio Plugin based on JUCE. Under dev, still buggy. 

This is a university project from the University of Applied Sciences Duesseldorf, Germany. 

# Features:
- VST/AU
- Mac/PC
- 360 degree (3D Full Sphere: Azimuth, Elevation & Distance(when provided)) Sourround Panning of a Mono Input Source
- Load SOFA Files from disc during runtime 
- Binaural Rendering via Fast Convolution
- Access to SOFA Metadata (view only)
- Crossfading between Convolution Products when the HRTF is exchanged (Angle changed) 
- Graphical Representation (Plot) of current HRTF and HRIR 

# Planned:
- Documentation
- Better Name
- Linux Support
- Individualization of Head Characteristics, e.g. Ear Distance (ITD)
- Early Reflections  
- Distance Simulation
- Headtracking
- Different interpolation techniques (for positions between measured HRTFs) 
- … and of course bugfixes and improved audio quality.

# Additional Dependencies:
- JUCE (Guts of Plugin & Audio Engine)
- FFTW (Fourier Transform/Fast Convolution)
- NetCDF (Access to Data in SOFA Files)
- VST2/3 SDK (For Building the VST)

NetCDF & FFTW are included in the repo. The VST SDK and the JUCE-Modules need to be added in the Jucer-Config. 

The standart HRTF („mit_kemar_normal_pinna.sofa“, included in the repo) needs to be located at "/Library/Audio/Plug-Ins/VST/SOFAFiles/mit_kemar_normal_pinna.sofa" (Mac) or "C:\\Program Files\\Steinberg\\VstPlugins\\SOFAFiles\\mit_kemar_normal_pinna.sofa" (Win64)
