# Supercore — A Virtual Synthesizer Optimized for Super-Saw Sounds

Supercore is a synthesizer developed for my bachelor's thesis at the Università degli Studi di Milano.
The goal was to design a VST3-compliant audio plug-in optimized for super-saw synthesis using the JUCE framework.  
To minimize aliasing and digital artifacts, Band-Limited Impulse Trains (BLITs) and 2× oversampling were used, reducing aliasing to levels below −50 dB — nearly imperceptible to the human ear.

![Supercore GUI](/DemoSynth-supersawizzato/Resources/images/00GUI.png)

## About the synth

Supercore is a synthesizer that bases its sonic character on the detuning logic used in the supersaw technique. Supercore also introduces unique features such as per-oscillator stereo width control, which, when combined with detuned oscillators, produces distinctive and rich sonic textures.

Supercore features three sound generators: 
- **Main Oscillator**: Can produce sawtooth, sharktooth, triangle, inverse saw, square, narrow square, wide square. Includes parameters such as detune and stereo width.
- **Sub Oscillator**: Can produce sine or square waves. Has a dedicated register knob.
- **Noise Generator**: Has a dedicated release envelope and color knob that uses a high pass or lowpass filter depending on the value.

The generated sounds are then followed by the mixer, the Minimoog style low-pass filter and modulation LFO, then the envelope, and concluding with the master output.

This project was coded at **Laboratorio di Informatica Musicale (LIM)** — _the Music Informatics Laboratory_ of the Università degli Studi di Milano.

**Student**: Derin Donmez
**Supervisor**: Giorgio Presti
**Co-Supervisor**: Federico Avanzini
