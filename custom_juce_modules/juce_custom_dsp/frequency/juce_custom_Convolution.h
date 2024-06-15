/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce::dsp
{

/**
    Used by the CustomConvolution to dispatch engine-update messages on a background
    thread.

    May be shared between multiple CustomConvolution instances.

    @tags{DSP}
*/
class JUCE_API CustomConvolutionMessageQueue
{
public:
    /** Initialises the queue to a default size.

        If your CustomConvolution is updated very frequently, or you are sharing
        this queue between multiple CustomConvolutions, consider using the alternative
        constructor taking an explicit size argument.
    */
    CustomConvolutionMessageQueue();
    ~CustomConvolutionMessageQueue() noexcept;

    /** Initialises the queue with the specified number of entries.

        In general, the number of required entries scales with the number
        of CustomConvolutions sharing the same Queue, and the frequency of updates
        to those CustomConvolutions.
    */
    explicit CustomConvolutionMessageQueue (int numEntries);

    CustomConvolutionMessageQueue (CustomConvolutionMessageQueue&&) noexcept;
    CustomConvolutionMessageQueue& operator= (CustomConvolutionMessageQueue&&) noexcept;

    CustomConvolutionMessageQueue (const CustomConvolutionMessageQueue&) = delete;
    CustomConvolutionMessageQueue& operator= (const CustomConvolutionMessageQueue&) = delete;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;

    friend class CustomConvolution;
};

/**
    Performs stereo partitioned Customconvolution of an input signal with an
    impulse response in the frequency domain, using the JUCE FFT class.

    This class provides some thread-safe functions to load impulse responses
    from audio files or memory on-the-fly without noticeable artefacts,
    performing resampling and trimming if necessary.

    The processing performed by this class is equivalent to the time domain
    Customconvolution done in the FIRFilter class, with a FIRFilter::Coefficients
    object having the samples of the impulse response as its coefficients.
    However, in general it is more efficient to do frequency domain
    Customconvolution when the size of the impulse response is 64 samples or
    greater.

    Note: The default operation of this class uses zero latency and a uniform
    partitioned algorithm. If the impulse response size is large, or if the
    algorithm is too CPU intensive, it is possible to use either a fixed
    latency version of the algorithm, or a simple non-uniform partitioned
    Customconvolution algorithm.

    Threading: It is not safe to interleave calls to the methods of this
    class. If you need to load new impulse responses during processing the
    load() calls must be synchronised with process() calls, which in practice
    means making the load() call from the audio thread. The
    loadImpulseResponse() functions *are* wait-free and are therefore
    suitable for use in a realtime context.

    @see FIRFilter, FIRFilter::Coefficients, FFT

    @tags{DSP}
*/
class JUCE_API  CustomConvolution
{
public:
    //==============================================================================
    /** Initialises an object for performing Customconvolution in the frequency domain. */
    CustomConvolution();

    /** Initialises a Customconvolution engine using a shared background message queue.

        IMPORTANT: the queue *must* remain alive throughout the lifetime of the
        CustomConvolution.
    */
    explicit CustomConvolution (CustomConvolutionMessageQueue& queue);

    /** Contains configuration information for a Customconvolution with a fixed latency. */
    struct Latency { int latencyInSamples; };

    /** Initialises an object for performing Customconvolution with a fixed latency.

        If the requested latency is zero, the actual latency will also be zero.
        For requested latencies greater than zero, the actual latency will
        always at least as large as the requested latency. Using a fixed
        non-zero latency can reduce the CPU consumption of the Customconvolution
        algorithm.

        @param requiredLatency        the minimum latency
    */
    explicit CustomConvolution (const Latency& requiredLatency);

    /** Contains configuration information for a non-uniform Customconvolution. */
    struct NonUniform { int headSizeInSamples; };

    /** Initialises an object for performing Customconvolution in the frequency domain
        using a non-uniform partitioned algorithm.

        A requiredHeadSize of 256 samples or greater will improve the
        efficiency of the processing for IR sizes of 4096 samples or greater
        (recommended for reverberation IRs).

        @param requiredHeadSize       the head IR size for two stage non-uniform
                                      partitioned Customconvolution
     */
    explicit CustomConvolution (const NonUniform& requiredHeadSize);

    /** Behaves the same as the constructor taking a single Latency argument,
        but with a shared background message queue.

        IMPORTANT: the queue *must* remain alive throughout the lifetime of the
        CustomConvolution.
    */
    CustomConvolution (const Latency&, CustomConvolutionMessageQueue&);

    /** Behaves the same as the constructor taking a single NonUniform argument,
        but with a shared background message queue.

        IMPORTANT: the queue *must* remain alive throughout the lifetime of the
        CustomConvolution.
    */
    CustomConvolution (const NonUniform&, CustomConvolutionMessageQueue&);

    ~CustomConvolution() noexcept;

    //==============================================================================
    /** Must be called before first calling process.

        In general, calls to loadImpulseResponse() load the impulse response (IR)
        asynchronously. The IR will become active once it has been completely loaded
        and processed, which may take some time.

        Calling prepare() will ensure that the IR supplied to the most recent call to
        loadImpulseResponse() is fully initialised. This IR will then be active during
        the next call to process(). It is recommended to call loadImpulseResponse() *before*
        prepare() if a specific IR must be active during the first process() call.
    */
    void prepare (const ProcessSpec&);

    /** Resets the processing pipeline ready to start a new stream of data. */
    void reset() noexcept;

    /** Performs the filter operation on the given set of samples with optional
        stereo processing.
    */
    template <typename ProcessContext,
              std::enable_if_t<std::is_same_v<typename ProcessContext::SampleType, float>, int> = 0>
    void process (const ProcessContext& context) noexcept
    {
        processSamples (context.getInputBlock(), context.getOutputBlock(), context.isBypassed);
    }

    //==============================================================================
    enum class Stereo    { no, yes };
    enum class Trim      { no, yes };
    enum class Normalise { no, yes };

    //==============================================================================
    /** This function loads an impulse response audio file from memory, added in a
        JUCE project with the Projucer as binary data. It can load any of the audio
        formats registered in JUCE, and performs some resampling and pre-processing
        as well if needed.

        Note: Don't try to use this function on float samples, since the data is
        expected to be an audio file in its binary format. Be sure that the original
        data remains constant throughout the lifetime of the CustomConvolution object, as
        the loading process will happen on a background thread once this function has
        returned.

        @param sourceData               the block of data to use as the stream's source
        @param sourceDataSize           the number of bytes in the source data block
        @param isStereo                 selects either stereo or mono
        @param requiresTrimming         optionally trim the start and the end of the impulse response
        @param size                     the expected size for the impulse response after loading, can be
                                        set to 0 to requesting the original impulse response size
        @param requiresNormalisation    optionally normalise the impulse response amplitude
    */
    void loadImpulseResponse (const void* sourceData, size_t sourceDataSize,
                              Stereo isStereo, Trim requiresTrimming, size_t size,
                              Normalise requiresNormalisation = Normalise::yes);

    /** This function loads an impulse response from an audio file. It can load any
        of the audio formats registered in JUCE, and performs some resampling and
        pre-processing as well if needed.

        @param fileImpulseResponse      the location of the audio file
        @param isStereo                 selects either stereo or mono
        @param requiresTrimming         optionally trim the start and the end of the impulse response
        @param size                     the expected size for the impulse response after loading, can be
                                        set to 0 to requesting the original impulse response size
        @param requiresNormalisation    optionally normalise the impulse response amplitude
    */
    void loadImpulseResponse (const File& fileImpulseResponse,
                              Stereo isStereo, Trim requiresTrimming, size_t size,
                              Normalise requiresNormalisation = Normalise::yes);

    /** This function loads an impulse response from an audio buffer.
        To avoid memory allocation on the audio thread, this function takes
        ownership of the buffer passed in.

        If calling this function during processing, make sure that the buffer is
        not allocated on the audio thread (be careful of accidental copies!).
        If you need to pass arbitrary/generated buffers it's recommended to
        create these buffers on a separate thread and to use some wait-free
        construct (a lock-free queue or a SpinLock/GenericScopedTryLock combination)
        to transfer ownership to the audio thread without allocating.

        @param buffer                   the AudioBuffer to use
        @param bufferSampleRate         the sampleRate of the data in the AudioBuffer
        @param isStereo                 selects either stereo or mono
        @param requiresTrimming         optionally trim the start and the end of the impulse response
        @param requiresNormalisation    optionally normalise the impulse response amplitude
    */
    void loadImpulseResponse (AudioBuffer<float>&& buffer, double bufferSampleRate,
                              Stereo isStereo, Trim requiresTrimming, Normalise requiresNormalisation);

    /** This function returns the size of the current IR in samples. */
    int getCurrentIRSize() const;

    /** This function returns the current latency of the process in samples.

        Note: This is the latency of the Customconvolution engine, not the latency
        associated with the current impulse response choice that has to be
        considered separately (linear phase filters, for example).
    */
    int getLatency() const;

private:
    //==============================================================================
    CustomConvolution (const Latency&,
                 const NonUniform&,
                 OptionalScopedPointer<CustomConvolutionMessageQueue>&&);

    void processSamples (const AudioBlock<const float>&, AudioBlock<float>&, bool isBypassed) noexcept;

    class Mixer
    {
    public:
        void prepare (const ProcessSpec&);

        template <typename ProcessWet>
        void processSamples (const AudioBlock<const float>&,
                             AudioBlock<float>&,
                             bool isBypassed,
                             ProcessWet&&) noexcept;

        void reset();

    private:
        std::array<SmoothedValue<float>, 2> volumeDry, volumeWet;
        AudioBlock<float> dryBlock;
        HeapBlock<char> dryBlockStorage;
        double sampleRate = 0;
        bool currentIsBypassed = false;
    };

    //==============================================================================
    class Impl;
    std::unique_ptr<Impl> pimpl;

    //==============================================================================
    Mixer mixer;
    bool isActive = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomConvolution)
};

} // namespace juce::dsp
