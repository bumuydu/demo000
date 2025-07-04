#pragma once
#include <JuceHeader.h>
#include "Tempo.h"

namespace Parameters
{
    // PARAM IDs
	static const String nameAtk = "A";
	static const String nameDcy = "D";
	static const String nameSus = "S";
	static const String nameRel = "R";
    static const String nameSawLev = "SAWLEV";
    static const String nameSubLev = "SUBLEV";
    static const String nameNLev = "NOISELEV";
    static const String nameMainWf = "MAINWF";
    static const String nameSawReg = "SAWREG";
    static const String nameSawNum = "SAWNUM";
    static const String nameDetune = "DETUNE";
    static const String nameStereoWidth = "STEREOWIDTH";
    static const String namePhase = "PHASE";
//    static const String namePhaseDegree = "PHASEDEGREE";
    static const String nameSubReg = "SUBREG";
    static const String nameSubWf = "SUBWF";
    static const String nameFiltHz = "FREQ";
    static const String nameFiltQ = "RES";
    static const String nameFiltEnv = "EGAMT";
    static const String nameFiltLfoAmt = "LFOAMT";
    static const String nameLfoWf = "LFOWF";
    static const String nameLfoFreq = "LFOFREQ";
    static const String nameLfoRate = "LFORATE";
    static const String nameLfoSync = "LFOSYNC";
    static const String nameNRel = "NOISEREL";
    static const String nameNFilt = "NFILT";
//    static const String nameOversampling = "OVERSMP";
    static const String nameMaster = "MASTER";

    // CONSTANTS
    static const float dbFloor = -48.0f;

    // PARAM DEFAULTS
	static const float defaultAtk = 0.01f;
	static const float defaultDcy = 0.025f;
	static const float defaultSus = 0.6f;
	static const float defaultRel = 1.000f;
    static const float defaultSaw = 0.000f;
    static const float defaultDetune = 15.0f;
    static const float defaultStereoWidth = 0.000f;
    static const float defaultFiltHz = 4000.0f;
    static const float defaultFiltQ = 0.000f;
    static const float defaultFiltEnv = 0.000f;
    static const float defaultFiltLfoAmt = 0.000f;
    static const float defaultLfoFreq = 0.1f;
    static const float defaultNoiseRel = 0.7f;
    static const float defaultNFilt = 0.5f;
    static const float defaultMaster = 0.8f;
    
    static const int defaultSawReg = 2; // in this case, it sets the default register to 0
    static const int defaultSawNum = 5;
    static const int defaultPhase = 0;
//    static const int defaultPhaseDegree = 0;
    static const int defaultMainWf = 0;
    static const int defaultSubReg = 3; // in this case, it sets the default register to -1
    static const int defaultSubWf = 0;
    static const int defaultLfoWf = 0;
    static const int defaultLfoSync = 0;
    static const int defaultLfoRate = 0;
//    static const int defaultOversampling = 0;

	static AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
	{
		std::vector<std::unique_ptr<RangedAudioParameter>> params;

        NormalisableRange<float> detuneRange(0.0f, 200.0f);
        detuneRange.setSkewForCentre(50.0f);
        
        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { nameMainWf, 1 }, "Main OSC Waveform", StringArray{"Saw Down","Sharktooth","Triangle","Saw Up","Square","Wide Square","Narrow Square"}, defaultMainWf));
        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { nameSawReg, 2 }, "Saw Register", StringArray{"-2","-1","0","1","2"}, defaultSawReg));
        params.push_back(std::make_unique<AudioParameterInt>(ParameterID { nameSawNum, 3 }, "# of Saws", 1, 16, defaultSawNum));
//        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameDetune, 4 }, "Detune", 0, 200, defaultDetune));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameDetune, 4 }, "Detune",   detuneRange, defaultDetune));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameStereoWidth, 5 }, "Stereo Width", NormalisableRange<float>(0.0f, 1.0f), defaultStereoWidth));
        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { namePhase, 6 }, "Phase Resetting", StringArray{"OFF","ON"}, defaultPhase));
//        params.push_back(std::make_unique<AudioParameterInt>(ParameterID { namePhaseDegree, 7 }, "Phase (degrees)", 0, 360, defaultPhaseDegree));
        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { nameSubReg, 7 }, "Sub Register", StringArray{"-3","-2","-1"}, defaultSubReg));
        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { nameSubWf, 8 }, "Sub Waveform", StringArray{"Sinusoidal","Square"}, defaultSubWf));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameSawLev, 9 }, "Saw Level (dB)", NormalisableRange<float>(dbFloor, 6.0f, 0.1f), defaultSaw));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameSubLev, 10 }, "Sub Level (dB)", NormalisableRange<float>(dbFloor, 6.0f, 0.1f), dbFloor));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameNLev, 11 }, "Noise Level (dB)", NormalisableRange<float>(dbFloor, 6.0f, 0.1f), dbFloor));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameFiltHz, 12 }, "Cutoff",   NormalisableRange<float>(5.0f, 19500.0f, 1.0f, 0.3f), defaultFiltHz));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameFiltQ, 13 }, "Quality", NormalisableRange<float>(0.05f, 1.00f, 0.01f, 0.5f), defaultFiltQ));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameFiltEnv, 14 }, "EG AMT", NormalisableRange<float>(-1.0f, 1.0f), defaultFiltEnv));
        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { nameLfoWf, 15 }, "LFO Waveform", StringArray{"Sinusoidal","Triangular","Saw Up","Square", "S&H"}, defaultLfoWf));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameFiltLfoAmt, 16 }, "LFO AMT", NormalisableRange<float>(0.0f, 1.0f), defaultFiltLfoAmt));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameLfoFreq, 17 }, "LFO FREQ", NormalisableRange<float>(0.1f, 20.0f), defaultLfoFreq));
        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { nameLfoRate, 18 }, "LFO RATE", MetricTime::timeChoices, defaultLfoRate));
        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { nameLfoSync, 19 }, "LFO SYNC", StringArray{"OFF","ON"}, defaultLfoSync));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameAtk, 20 }, "Attack (s)",    NormalisableRange<float>(0.001f, 10.0f, 0.001f, 0.3f), defaultAtk));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameDcy, 21 }, "Decay (s)",     NormalisableRange<float>(0.004f, 10.0f, 0.001f, 0.3f), defaultDcy));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameSus, 22 }, "Sustain (amp)", NormalisableRange<float>(0.0f, 1.00f), defaultSus));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameRel, 23 }, "Release (s)",   NormalisableRange<float>(0.0f, 10.0f, 0.001f, 0.3f), defaultRel));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameNRel, 24 }, "Noise Release (s)", NormalisableRange<float>(0.0f, 5.0f, 0.01f, 0.3f), defaultNoiseRel));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameNFilt, 25 }, "Noise Color/Filter (LPF,HPF)", NormalisableRange<float>(0.0f, 1.0f), defaultNFilt));
//        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { nameOversampling, 26 }, "Oversampling -- not yet implemented", StringArray{"2X","4X"}, defaultOversampling));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameMaster, 26 }, "Master", NormalisableRange<float>(-48.0f, 0.0f), defaultMaster));
        

		return { params.begin(), params.end() };
	}

	static void addListenerToAllParameters(AudioProcessorValueTreeState& valueTreeState, AudioProcessorValueTreeState::Listener* listener)
	{
		std::unique_ptr<XmlElement> xml(valueTreeState.copyState().createXml());

		for (auto element : xml->getChildWithTagNameIterator("PARAM"))
		{
			const String& id = element->getStringAttribute("id");
			valueTreeState.addParameterListener(id, listener);
		}
	}
}
