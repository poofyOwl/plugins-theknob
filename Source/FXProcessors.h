//
//  FXProcessors.h
//  TheKnob - Shared Code
//
//  Created by Vou Theophanous on 2025-05-23.
//  Copyright © 2025 VOU. All rights reserved.
//

#ifndef FXProcessors_h
#define FXProcessors_h


//==============================================================================
// KNOB
const float KNOB_MIN_VALUE = 1.0;
const float KNOB_MAX_VALUE = 100.0;
const float KNOB_DEFAULT_VALUE = 1.0;

// FILTER
const float HPF_FREQ_MIN_VALUE = 50.0;
const float HPF_FREQ_MAX_VALUE = 150.0;

// EQ
const float EQ_GAIN_MIN_VALUE = 1.0;
const float EQ_GAIN_MAX_VALUE = 3.0;
const float EQ_Q_MIN_VALUE = 2.0;
const float EQ_Q_MAX_VALUE = 1.0;

// DELAY
const std::array<float, 3> DELAY_TIME_L = { 0.7, 0.2, 1.0 }; // one for each mode
const std::array<float, 3> DELAY_TIME_R = { 0.7, 0.2, 1.0 }; // one for each mode
const float DELAY_FEEDBACK_MIN_VALUE = 0.0;
const std::array<float, 3> DELAY_FEEDBACK_MAX_VALUE = { 0.6, 0.75, 0.25 }; // one for each mode
const std::array<float, 3> DELAY_WET_LEVEL_MAX_VALUE = { 0.6, 0.5, 1.0 }; // one for each mode
const float DELAY_HPF_FREQ_MIN_VALUE = 200.0;
const float DELAY_HPF_FREQ_MAX_VALUE = 400.0;
const float DELAY_LPF_FREQ_MIN_VALUE = 10000.0;
const float DELAY_LPF_FREQ_MAX_VALUE = 3500.0;

// REVERB
const float REVERB_FREEZE_MIN_VALUE = 0.0;
const std::array<float, 3> REVERB_FREEZE_MAX_VALUE = { 0.35, 0.1, 0.1 }; // one for each mode
const std::array<float, 3> REVERB_WET_LEVEL_MAX_VALUE = { 0.5, 0.8, 1.0 }; // one for each mode
const std::array<float, 3> REVERB_ROOM_SIZE_MAX_VALUE = { 0.5, 0.75, 0.9 }; // one for each mode
const std::array<float, 3> REVERB_WIDTH_MAX_VALUE = { 0.5, 0.75, 0.9 }; // one for each mode

// DISTORTION
const std::array<float, 3> DIST_INPUT_GAIN_MIN_VALUE = { 0.0, 0.0, 5.0 }; // one for each mode
const std::array<float, 3> DIST_INPUT_GAIN_MAX_VALUE = { 10.0, 15.0, 10.0 }; // one for each mode

//==============================================================================
float mapKnobValueToRange(float x, float rangeStart, float rangeEnd)
{
    return rangeStart + (x - KNOB_MIN_VALUE) * (rangeEnd - rangeStart) / (KNOB_MAX_VALUE - KNOB_MIN_VALUE);
}

float normalizeKnobValue(float x)
{
    return mapKnobValueToRange(x, 0.0, 1.0);
}

//==============================================================================
class ProcessorBase : public juce::AudioProcessor
{
public:
    //==============================================================================
    ProcessorBase(std::atomic<float>* knobParam, std::atomic<float>* modeParam)
        : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo()).withOutput ("Output", juce::AudioChannelSet::stereo()))
    {
        knobValue = knobParam;
        mode = modeParam;
    }
    //==============================================================================
    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    void processBlock (juce::AudioSampleBuffer&, juce::MidiBuffer&) override {}
    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    //==============================================================================
    const juce::String getName() const override { return {}; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0; }
    //==============================================================================
    int getNumPrograms() override { return 0; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    //==============================================================================
    void getStateInformation (juce::MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}
protected:
    std::atomic<float>* knobValue;
    std::atomic<float>* mode;
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorBase)
};

//==============================================================================
class FilterProcessor  : public ProcessorBase
{
public:
    FilterProcessor(std::atomic<float>* knobParam, std::atomic<float>* modeParam) : ProcessorBase(knobParam, modeParam){}
    
    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        setFrequency();

        juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (samplesPerBlock), 2 };
        filter.prepare (spec);
    }

    void processBlock (juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) override
    {
        setFrequency();
        
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        filter.process (context);
    }

    void reset() override
    {
        filter.reset();
    }

    const juce::String getName() const override { return "Filter"; }
    
    void setFrequency()
    {
        float frequency = mapKnobValueToRange(*knobValue, HPF_FREQ_MIN_VALUE, HPF_FREQ_MAX_VALUE);
        *filter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass (getSampleRate(), frequency);
    }

private:
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> filter;
};

//==============================================================================
class EQProcessor  : public ProcessorBase
{
public:
    EQProcessor(std::atomic<float>* knobParam, std::atomic<float>* modeParam) : ProcessorBase(knobParam, modeParam){}
    
    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        setFilterCoefs();
        
        juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (samplesPerBlock), 2 };
        filter1.prepare (spec);
//        filter2.prepare (spec);
        filter3.prepare (spec);
        filter4.prepare (spec);
    }

    void processBlock (juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) override
    {
        setFilterCoefs();
        
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        filter1.process (context);
//        filter2.process (context);
        filter3.process (context);
        filter4.process (context);
    }

    void reset() override
    {
        filter1.reset();
//        filter2.reset();
        filter3.reset();
        filter4.reset();
    }

    const juce::String getName() const override { return "EQ"; }
    
    void setFilterCoefs()
    {
        float knobVal = *knobValue;
        float gain = mapKnobValueToRange(knobVal, EQ_GAIN_MIN_VALUE, EQ_GAIN_MAX_VALUE);
        float q = mapKnobValueToRange(knobVal, EQ_Q_MIN_VALUE, EQ_Q_MAX_VALUE);

        // boost at 222 Hz
        *filter1.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (getSampleRate(), 250, q, gain);
        // boost at 2222 Hz
//        *filter2.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (getSampleRate(), 2222, q, gain);
        // boost at 16k Hz
        *filter3.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (getSampleRate(), 16000, q, gain);
        // remove at 400 Hz
        *filter4.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (getSampleRate(), 400, q, 1/(gain));
    }

private:
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> filter1;
//    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> filter2;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> filter3;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> filter4;
};

//==============================================================================
class SpecialEQProcessor  : public ProcessorBase
{
public:
    SpecialEQProcessor(std::atomic<float>* knobParam, std::atomic<float>* modeParam) : ProcessorBase(knobParam, modeParam){}
    
    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        setFilterCoefs();
        
        juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (samplesPerBlock), 2 };
        filter1.prepare (spec);
        filter2.prepare (spec);
        filter3.prepare (spec);
        filter4.prepare (spec);
    }

    void processBlock (juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) override
    {
        setFilterCoefs();
        
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        
        switch((int)*mode)
        {
            case VIOLET:
                filter1.process (context);
                filter2.process (context);
                break;
                
            case TEAL:
                filter1.process (context);
                filter2.process (context);
                break;
                
            case CRIMSON:
                filter1.process (context);
                filter2.process (context);
                filter3.process (context);
                filter4.process (context);
                break;
        }
    }

    void reset() override
    {
        filter1.reset();
        filter2.reset();
        filter3.reset();
        filter4.reset();
    }

    const juce::String getName() const override { return "Special EQ"; }
    
    void setFilterCoefs()
    {
        switch((int)*mode)
        {
            case VIOLET:
                *filter1.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (getSampleRate(), 400, 1, 0.5); //-3db
                *filter2.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (getSampleRate(), 10000, 0.71, 1.5); //3db
                break;
                
            case TEAL:
                *filter1.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (getSampleRate(), 1000, 2.11, 1.5); //3db
                *filter2.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (getSampleRate(), 10000, 0.71, 1.5); //3db
                break;
                
            case CRIMSON:
                *filter1.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass (getSampleRate(), 111, 0.71);
                *filter2.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass (getSampleRate(), 2500, 0.66);
                *filter3.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (getSampleRate(), 177, 0.71, 1.67); //4db
                *filter4.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (getSampleRate(), 1777, 0.71, 1.83); //5db
                break;
        }
    }

private:
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> filter1;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> filter2;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> filter3;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> filter4;
};

//==============================================================================
class ReverbProcessor  : public ProcessorBase
{
    // https://github.com/szkkng/simple-reverb
public:
    ReverbProcessor(std::atomic<float>* knobParam, std::atomic<float>* modeParam) : ProcessorBase(knobParam, modeParam){}
    
    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        setParams();
        
        juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (samplesPerBlock), 2 };
        reverbChain.prepare (spec);
    }

    void processBlock (juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) override
    {
        setParams();
        
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        reverbChain.process (context);
    }

    void reset() override
    {
        reverbChain.reset();
    }

    const juce::String getName() const override { return "Reverb"; }
    
    void setParams()
    {
        float knobVal = *knobValue;
        float normVal = normalizeKnobValue(knobVal);
        int modeVal = (int)(*mode);
        
        // reverb params
        juce::dsp::Reverb::Parameters params;
        params.roomSize = mapKnobValueToRange(knobVal, 0, REVERB_ROOM_SIZE_MAX_VALUE[modeVal]);
        params.damping = modeVal == CRIMSON ? normVal : 1 - normVal;
        params.wetLevel = mapKnobValueToRange(knobVal, 0, REVERB_WET_LEVEL_MAX_VALUE[modeVal]);
        params.dryLevel = 1 - params.wetLevel;
        params.width = mapKnobValueToRange(knobVal, 0, REVERB_WIDTH_MAX_VALUE[modeVal]);
        params.freezeMode = mapKnobValueToRange(knobVal, REVERB_FREEZE_MIN_VALUE, REVERB_FREEZE_MAX_VALUE[modeVal]);
        
        auto& reverb = reverbChain.template get<reverbIndex>();
        reverb.setParameters(params);
        
        // filter params
        auto& hpf = reverbChain.template get<hpfIndex>();
        auto& lpf = reverbChain.template get<lpfIndex>();
        hpf.state = FilterCoefs::makeFirstOrderHighPass (getSampleRate(), 300.0f);
        lpf.state = FilterCoefs::makeFirstOrderLowPass(getSampleRate(), 3500.0f);
    }

private:
    enum {
        hpfIndex,
        lpfIndex,
        reverbIndex
    };
    using Filter = juce::dsp::IIR::Filter<float>;
    using FilterCoefs = juce::dsp::IIR::Coefficients<float>;
    juce::dsp::ProcessorChain<juce::dsp::ProcessorDuplicator<Filter, FilterCoefs>, juce::dsp::ProcessorDuplicator<Filter, FilterCoefs>, juce::dsp::Reverb> reverbChain;
};

//==============================================================================
template <typename Type>
class DelayLine
{
public:
    void clear() noexcept
    {
        std::fill (rawData.begin(), rawData.end(), Type (0));
    }

    size_t size() const noexcept
    {
        return rawData.size();
    }

    void resize (size_t newValue)
    {
        rawData.resize (newValue);
        leastRecentIndex = 0;
    }

    Type back() const noexcept
    {
        return rawData[leastRecentIndex];
    }

    Type get (size_t delayInSamples) const noexcept
    {
        jassert (delayInSamples >= 0 && delayInSamples < size());

        return rawData[(leastRecentIndex + 1 + delayInSamples) % size()];
    }

    /** Set the specified sample in the delay line */
    void set (size_t delayInSamples, Type newValue) noexcept
    {
        jassert (delayInSamples >= 0 && delayInSamples < size());

        rawData[(leastRecentIndex + 1 + delayInSamples) % size()] = newValue;
    }

    /** Adds a new value to the delay line, overwriting the least recently added sample */
    void push (Type valueToAdd) noexcept
    {
        rawData[leastRecentIndex] = valueToAdd;
        leastRecentIndex = leastRecentIndex == 0 ? size() - 1 : leastRecentIndex - 1;
    }

private:
    std::vector<Type> rawData;
    size_t leastRecentIndex = 0;
};

//==============================================================================
template <typename Type, size_t maxNumChannels = 2>
class Delay
{
public:
    //==============================================================================
    Delay(){}

    //==============================================================================
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        jassert (spec.numChannels <= maxNumChannels);
        sampleRate = (Type) spec.sampleRate;
        
        auto delayLineSizeSamples = (size_t) std::ceil (maxDelayTime * sampleRate);
        for (auto& dline : delayLines)
            dline.resize (delayLineSizeSamples);

        filterCoefs = juce::dsp::IIR::Coefficients<Type>::makeFirstOrderLowPass (sampleRate, Type (1000));
        for (auto& f : filters)
        {
            f.prepare (spec);
            f.coefficients = filterCoefs;
        }
    }

    //==============================================================================
    void reset() noexcept
    {
        for (auto& f : filters)
            f.reset();

        for (auto& dline : delayLines)
            dline.clear();
    }

    //==============================================================================
    size_t getNumChannels() const noexcept
    {
        return delayLines.size();
    }

    //==============================================================================
    void setFeedback (Type newValue) noexcept
    {
        jassert (newValue >= Type (0) && newValue <= Type (1));
        feedback = newValue;
    }

    //==============================================================================
    void setWetLevel (Type newValue) noexcept
    {
        jassert (newValue >= Type (0) && newValue <= Type (1));
        wetLevel = newValue;
    }

    //==============================================================================
    void setDelayTime (size_t channel, Type newValue)
    {
        if (channel >= getNumChannels())
        {
            jassertfalse;
            return;
        }

        jassert (newValue >= Type (0));
        delayTimesSample[channel] = (size_t) juce::roundToInt (newValue * sampleRate);
    }

    //==============================================================================
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        auto& inputBlock  = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
        auto numSamples  = outputBlock.getNumSamples();
        auto numChannels = outputBlock.getNumChannels();

        jassert (inputBlock.getNumSamples() == numSamples);
        jassert (inputBlock.getNumChannels() == numChannels);

        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto* input  = inputBlock .getChannelPointer (ch);
            auto* output = outputBlock.getChannelPointer (ch);
            
            for (size_t i = 0; i < numSamples; ++i)
            {
                auto delayedSample = filters[ch].processSample (delayLines[ch].get (delayTimesSample[ch]));
                auto inputSample = input[i];
                
                auto dlineInputSample = std::tanh (inputSample + feedback * delayedSample);
                delayLines[ch].push (dlineInputSample);
                
                auto outputSample = inputSample + wetLevel * delayedSample;
                output[i] = outputSample;
            }
        }
    }

private:
    //==============================================================================
    std::array<DelayLine<Type>, maxNumChannels> delayLines;
    std::array<size_t, maxNumChannels> delayTimesSample;

    std::array<juce::dsp::IIR::Filter<Type>, maxNumChannels> filters;
    typename juce::dsp::IIR::Coefficients<Type>::Ptr filterCoefs;

    Type feedback { Type (0) };
    Type wetLevel { Type (0) };
    Type sampleRate   { Type (44.1e3) };
    Type maxDelayTime { Type (2) };
};

//==============================================================================
class DelayProcessor  : public ProcessorBase
{
public:
    DelayProcessor(std::atomic<float>* knobParam, std::atomic<float>* modeParam) : ProcessorBase(knobParam, modeParam){}
    
    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        setParams();
        
        juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (samplesPerBlock), 2 };
        delayChain.prepare (spec);
    }

    void processBlock (juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) override
    {
        setParams();
        
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        delayChain.process (context);
    }

    void reset() override
    {
        delayChain.reset();
    }

    const juce::String getName() const override { return "Delay"; }
    
    void setParams()
    {
        float knobVal = *knobValue;
        float modeVal = (int)(*mode);
        
        auto& delay = delayChain.template get<delayIndex>();
        delay.setDelayTime(0, DELAY_TIME_L[modeVal]);
        delay.setDelayTime(1, DELAY_TIME_R[modeVal]);
        
        // wet level: 0..1
        // feedback: 0..0.49
        float wetLevel = mapKnobValueToRange(knobVal, 0, DELAY_WET_LEVEL_MAX_VALUE[modeVal]);
        float feedback = mapKnobValueToRange(knobVal, DELAY_FEEDBACK_MIN_VALUE, DELAY_FEEDBACK_MAX_VALUE[modeVal]);
        delay.setWetLevel(wetLevel);
        delay.setFeedback(feedback);
        
        // filter params
        auto& hpf = delayChain.template get<hpfIndex>();
        auto& lpf = delayChain.template get<lpfIndex>();
        float hpfCutoff = mapKnobValueToRange(knobVal, DELAY_HPF_FREQ_MIN_VALUE, DELAY_HPF_FREQ_MAX_VALUE);
        float lpfCutoff = mapKnobValueToRange(knobVal, DELAY_LPF_FREQ_MIN_VALUE, DELAY_LPF_FREQ_MAX_VALUE);
        hpf.state = FilterCoefs::makeFirstOrderHighPass (getSampleRate(), hpfCutoff);
        lpf.state = FilterCoefs::makeFirstOrderLowPass(getSampleRate(), lpfCutoff);
    }

private:
    //==============================================================================
    enum
    {
        hpfIndex,
        lpfIndex,
        delayIndex
    };
    using Filter = juce::dsp::IIR::Filter<float>;
    using FilterCoefs = juce::dsp::IIR::Coefficients<float>;
    juce::dsp::ProcessorChain<juce::dsp::ProcessorDuplicator<Filter, FilterCoefs>, juce::dsp::ProcessorDuplicator<Filter, FilterCoefs>, Delay<float>> delayChain;
};

//==============================================================================
class DistortionProcessor  : public ProcessorBase
{
public:
    DistortionProcessor(std::atomic<float>* knobParam, std::atomic<float>* modeParam) : ProcessorBase(knobParam, modeParam) {}

    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        setParams();
        
        juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (samplesPerBlock), 2 };
        distortionChain.prepare (spec);
    }

    void processBlock (juce::AudioSampleBuffer& buffer, juce::MidiBuffer&) override
    {
        setParams();
        
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        distortionChain.process (context);
    }

    void reset() override
    {
        distortionChain.reset();
    }

    const juce::String getName() const override { return "Distortion"; }
    
    void setParams()
    {
        int modeVal = (int)(*mode);
        
        float inputGain = mapKnobValueToRange(*knobValue, DIST_INPUT_GAIN_MIN_VALUE[modeVal], DIST_INPUT_GAIN_MAX_VALUE[modeVal]);
        auto& preGain = distortionChain.template get<preGainIndex>();
        preGain.setGainDecibels (inputGain);
        auto& postGain = distortionChain.template get<postGainIndex>();
        postGain.setGainDecibels (inputGain*-0.75);
        
        auto& waveshaper = distortionChain.template get<waveshaperIndex>();
        switch((int)(*mode))
        {
            case VIOLET:
                waveshaper.functionToUse = [] (float x) { return std::tanh(std::sin(x)); };
                break;
            case TEAL:
                waveshaper.functionToUse = [] (float x) { return std::tanh(x); };
                break;
            case CRIMSON:
                waveshaper.functionToUse = [] (float x) { return std::tanh(x); };
                break;
        }
    }

private:
    enum
    {
        preGainIndex,
        waveshaperIndex,
        postGainIndex
    };
    juce::dsp::ProcessorChain<juce::dsp::Gain<float>, juce::dsp::WaveShaper<float>, juce::dsp::Gain<float>> distortionChain;
};

#endif /* FXProcessors_h */
