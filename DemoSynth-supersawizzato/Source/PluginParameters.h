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
    static const String nameSawReg = "SAWREG";
    static const String nameSawNum = "SAWNUM";
    static const String nameDetune = "DETUNE";
    static const String nameStereoWidth = "STEREOWIDTH";
    static const String namePhase = "PHASE";
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
    static const String nameOversampling = "OVERSMP";

    // CONSTANTS
    static const float dbFloor = -48.0f;

    // PARAM DEFAULTS
	static const float defaultAtk = 0.000f;
	static const float defaultDcy = 0.025f;
	static const float defaultSus = 0.6f;
	static const float defaultRel = 1.000f;
    static const float defaultSaw = 0.000f;
    static const float defaultStereoWidth = 0.000f;
    static const float defaultFiltHz = 4000.0f;
    static const float defaultFiltQ = 0.000f;
    static const float defaultFiltEnv = 0.000f;
    static const float defaultFiltLfoAmt = 0.000f;
    static const float defaultLfoFreq = 0.1f;
    static const float defaultNoiseRel = 0.7f;
    static const float defaultNFilt = 0.5f;
    
    static const int defaultSawReg = 2; // in this case, it sets the default register to 0
    static const int defaultSawNum = 5;
    static const int defaultDetune = 15;
    static const int defaultPhase = 180;
    static const int defaultSubReg = 0;
    static const int defaultSubWf = 0;
    static const int defaultLfoWf = 0;
    static const int defaultLfoSync = 0;
    static const int defaultLfoRate = 0;
    static const int defaultOversampling = 2;
    
//    static const float defaultSub = -20.000f;   // I want these two off by default --> set to dbFloor
//    static const float defaultNoise = -20.000f;


	static AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
	{
		std::vector<std::unique_ptr<RangedAudioParameter>> params;

        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { nameSawReg, 1 }, "Saw Register", StringArray{"-2","-1","0","1","2"}, defaultSawReg));
        params.push_back(std::make_unique<AudioParameterInt>(ParameterID { nameSawNum, 2 }, "# of Saws", 1, 16, defaultSawNum));
        params.push_back(std::make_unique<AudioParameterInt>(ParameterID { nameDetune, 3 }, "Detune", 0, 100, defaultDetune));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameStereoWidth, 4 }, "Stereo Width", NormalisableRange<float>(0.0f, 1.0f), defaultStereoWidth));
        params.push_back(std::make_unique<AudioParameterInt>(ParameterID { namePhase, 5 }, "Phase (degrees) -- not yet implemented", -180, 180, defaultPhase));
        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { nameSubReg, 6 }, "Sub Register", StringArray{"0","-1","-2"}, defaultSubReg));
        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { nameSubWf, 7 }, "Sub Waveform", StringArray{"Sinusoidal","Square"}, defaultSubWf));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameSawLev, 8 }, "Saw Level (dB)", NormalisableRange<float>(dbFloor, 6.0f, 0.1f), defaultSaw));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameSubLev, 9 }, "Sub Level (dB)", NormalisableRange<float>(dbFloor, 6.0f, 0.1f), dbFloor));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameNLev, 10 }, "Noise Level (dB)", NormalisableRange<float>(dbFloor, 6.0f, 0.1f), dbFloor));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameFiltHz, 11 }, "Cutoff",   NormalisableRange<float>(5.0f, 18000.0f, 1.0f, 0.3f), defaultFiltHz));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameFiltQ, 12 }, "Quality", NormalisableRange<float>(0.05f, 1.00f, 0.01f, 0.5f), defaultFiltQ));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameFiltEnv, 13 }, "EG AMT", NormalisableRange<float>(-1.0f, 1.0f), defaultFiltEnv));
        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { nameLfoWf, 14 }, "LFO Waveform", StringArray{"Sinusoidal","Triangular","Saw Up","Square","Stepped S&H","Smooth S&H"}, defaultLfoWf));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameFiltLfoAmt, 15 }, "LFO AMT", NormalisableRange<float>(0.0f, 1.0f), defaultFiltLfoAmt));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameLfoFreq, 16 }, "LFO FREQ", NormalisableRange<float>(0.1f, 20.0f), defaultLfoFreq));
        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { nameLfoRate, 17 }, "LFO RATE", MetricTime::timeChoices, defaultLfoRate));
        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { nameLfoSync, 18 }, "LFO SYNC", StringArray{"OFF","ON"}, defaultLfoSync));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameAtk, 19 }, "Attack (s)",    NormalisableRange<float>(0.0f, 10.0f, 0.001f, 0.3f), defaultAtk));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameDcy, 20 }, "Decay (s)",     NormalisableRange<float>(0.0f, 10.0f, 0.001f, 0.3f), defaultDcy));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameSus, 21 }, "Sustain (amp)", NormalisableRange<float>(0.0f, 1.00f), defaultSus));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameRel, 22 }, "Release (s)",   NormalisableRange<float>(0.0f, 10.0f, 0.001f, 0.3f), defaultRel));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameNRel, 23 }, "Noise Release (s)", NormalisableRange<float>(0.0f, 5.0f, 0.01f, 0.3f), defaultNoiseRel));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameNFilt, 24 }, "Noise Color/Filter (LPF,HPF)", NormalisableRange<float>(0.0f, 1.0f), defaultNFilt));
        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { nameOversampling, 25 }, "Oversampling", StringArray{"2X","4X"}, defaultOversampling));
        

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
