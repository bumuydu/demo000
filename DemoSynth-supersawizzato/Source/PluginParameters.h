#pragma once
#include <JuceHeader.h>

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
    static const String nameFiltLfo = "LFO";
    static const String nameNRel = "NOISEREL";
    static const String nameNFilt = "NFILT";
    
//    static const String nameFiltEG = "FILTEG";
//    static const String nameFiltLFO = "LFOAMT";
    

    // CONSTANTS
    static const float dbFloor = -48.0f;

    // PARAM DEFAULTS
	static const float defaultAtk = 0.000f;
	static const float defaultDcy = 0.025f;
	static const float defaultSus = 0.6f;
	static const float defaultRel = 1.000f;
    static const float defaultSaw = 0.000f;
    static const float defaultStereoWidth = 0.000f;
    static const float defaultFiltHz = 1000.0f;
    static const float defaultFiltQ = 0.000f;
    static const float defaultFiltEnv = 0.000f;
    static const float defaultFiltLfo = 0.000f;
    static const float defaultNoiseRel = 0.7f;
    static const float defaultNFilt = 0.5f;
    
    static const int defaultSawReg = 2; // in this case, it sets the default register to 0
    static const int defaultSawNum = 5;
    static const int defaultDetune = 15;
    static const int defaultPhase = 180;
    static const int defaultSubReg = 0;
    static const int defaultSubWf = 0;
    
//    static const float defaultSub = -20.000f;   // I want these two off by default --> set to dbFloor
//    static const float defaultNoise = -20.000f;


	static AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
	{
		std::vector<std::unique_ptr<RangedAudioParameter>> params;

        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { nameSawReg,  1 }, "Saw Register", StringArray{"-2","-1","0","1","2"}, defaultSawReg));
        params.push_back(std::make_unique<AudioParameterInt>(ParameterID { nameSawNum,  2 }, "# of Saws", 1, 16, defaultSawNum));
        params.push_back(std::make_unique<AudioParameterInt>(ParameterID { nameDetune,  3 }, "Detune", 0, 100, defaultDetune));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameStereoWidth,  4 }, "Stereo Width", NormalisableRange<float>(0.0f, 1.0f), defaultStereoWidth));
        params.push_back(std::make_unique<AudioParameterInt>(ParameterID { namePhase, 5 }, "Phase (degrees) -- not yet implemented", -180, 180, defaultPhase));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameSawLev,  6 }, "Saw Level (dB)", NormalisableRange<float>(dbFloor, 6.0f, 0.1f), defaultSaw));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameSubLev,  7 }, "Sub Level (dB)", NormalisableRange<float>(dbFloor, 6.0f, 0.1f), dbFloor));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameNLev,  8 }, "Noise Level (dB)", NormalisableRange<float>(dbFloor, 6.0f, 0.1f), dbFloor));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameAtk,  9 }, "Attack (s)",    NormalisableRange<float>(0.0f, 10.0f, 0.001f, 0.3f), defaultAtk));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameDcy,  10 }, "Decay (s)",     NormalisableRange<float>(0.0f, 10.0f, 0.001f, 0.3f), defaultDcy));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameSus,  11 }, "Sustain (amp)", NormalisableRange<float>(0.0f, 1.00f, 0.010f, 0.5f), defaultSus));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameRel,  12 }, "Release (s)",   NormalisableRange<float>(0.0f, 10.0f, 0.001f, 0.3f), defaultRel));
        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { nameSubReg,  13 }, "Sub Register", StringArray{"0","-1","-2"}, defaultSubReg));
        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { nameSubWf,  14 }, "Sub Waveform", StringArray{"Sinusoidal","Square"}, defaultSubWf));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameFiltHz,  15 }, "Cutoff",   NormalisableRange<float>(5.0f, 18000.0f, 1.0f, 0.3f), defaultFiltHz));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameFiltQ,  16 }, "Quality", NormalisableRange<float>(0.05f, 1.00f, 0.01f, 0.5f), defaultFiltQ));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameFiltEnv,  17 }, "EG AMT--not yet implemented!", NormalisableRange<float>(-1.0f, 1.0f), defaultFiltEnv));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameFiltLfo,  18 }, "LFO AMT--not yet implemented!", NormalisableRange<float>(0.0f, 1.0f), defaultFiltLfo));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameNRel,  19 }, "Noise Release (s)", NormalisableRange<float>(0.0f, 5.0f, 0.01f, 0.3f), defaultNoiseRel));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameNFilt,  20 }, "Noise Color/Filter (LPF,HPF)", NormalisableRange<float>(0.0f, 1.0f), defaultNFilt));
        //parameters.push_back(std::make_unique<AudioParameterChoice>(nameWaveform, "LFO Waveform", StringArray{"Sinusoidal","Triangular","Saw Up","Saw Down","Square"}, defaultWaveform));

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
