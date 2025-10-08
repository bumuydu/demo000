# Implementation of a virtual synthesizer optimized for super-saw sounds

For my bachelor's thesis, I developed a synthesizer using the JUCE framework. The goal of this project was to create a VST-3-compliant audio plug-in that focused on optimizing super-saw sounds. To reduce aliasing and the artifact noise that comes with it, Band-Limited Impulse Trains (BLITs) were used in conjunction with oversampling. After implementing these two techniques, the aliasing levels became nearly imperceptible to the human ear, occurring predominantly below -50 dB.

Student: Derin Donmez

Supervisor: Giorgio Presti

Co-Supervisor: Federico Avanzini
