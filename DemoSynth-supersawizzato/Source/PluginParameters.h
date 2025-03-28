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
    static const String nameNRel = "NOISEREL";
    static const String nameSubReg = "SUBREG";
    static const String nameSubWf = "SUBWF";
    static const String nameFiltHz = "FREQ";
    static const String nameFiltQ = "RES";
//    static const String nameFiltEG = "FILTEG";
//    static const String nameFiltLFO = "LFOAMT";
    

    // CONSTANTS
    static const float dbFloor = -48.0f;

    // PARAM DEFAULTS
	static const float defaultAtk = 0.000f;
	static const float defaultDcy = 0.025f;
	static const float defaultSus = 0.600f;
	static const float defaultRel = 1.000f;
    static const float defaultSaw = 0.000f;
    static const float defaultNoiseRel = 0.025f;
    static const float defaultFiltHz = 1000.0f;
    static const float defaultFiltQ = 0.0f;
    
    static const int defaultSubReg = 0;
    static const int defaultSubWf = 0;
    
//    static const float defaultSub = -20.000f;   // I want these two off by default --> set to dbFloor
//    static const float defaultNoise = -20.000f;


	static AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
	{
		std::vector<std::unique_ptr<RangedAudioParameter>> params;

        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameSawLev,  1 }, "Sawtooth(s) Level (dB)", NormalisableRange<float>(dbFloor, 6.0f, 0.1f), defaultSaw));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameSubLev,  2 }, "Sub Level (dB)", NormalisableRange<float>(dbFloor, 6.0f, 0.1f), dbFloor));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameNLev,  3 }, "Noise Level (dB)", NormalisableRange<float>(dbFloor, 6.0f, 0.1f), dbFloor));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameNRel,  4 }, "Noise Release (s)", NormalisableRange<float>(0.0f, 10.0f, 0.001f, 0.3f), defaultNoiseRel));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameAtk,  5 }, "Attack (s)",    NormalisableRange<float>(0.0f, 10.0f, 0.001f, 0.3f), defaultAtk));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameDcy,  6 }, "Decay (s)",     NormalisableRange<float>(0.0f, 10.0f, 0.001f, 0.3f), defaultDcy));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameSus,  7 }, "Sustain (amp)", NormalisableRange<float>(0.0f, 1.00f, 0.010f, 0.5f), defaultSus));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameRel,  8 }, "Release (s)",   NormalisableRange<float>(0.0f, 10.0f, 0.001f, 0.3f), defaultRel));
        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { nameSubReg,  9 }, "Sub Register", StringArray{"0","-1","-2"}, defaultSubReg));
        params.push_back(std::make_unique<AudioParameterChoice>(ParameterID { nameSubWf,  10 }, "Sub Waveform", StringArray{"Sinusoidal","Square"}, defaultSubWf));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameFiltHz,  11 }, "Cutoff",   NormalisableRange<float>(0.0f, 18000.0f, 1.0f, 0.3f), defaultFiltHz));
        params.push_back(std::make_unique<AudioParameterFloat>(ParameterID { nameFiltQ,  12 }, "Quality", NormalisableRange<float>(0.05f, 1.8f, 0.01f, 0.025f), defaultFiltQ));
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
