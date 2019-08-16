// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MediaIOCorePlayerBase.h"

#include "AjaMediaPrivate.h"
#include "AjaMediaSource.h"

class FAjaMediaAudioSample;
class FAjaMediaAudioSamplePool;
class FAjaMediaBinarySamplePool;
class FAjaMediaTextureSample;
class FAjaMediaTextureSamplePool;
class FMediaIOCoreBinarySampleBase;
class IMediaEventSink;

enum class EMediaTextureSampleFormat;

namespace AJA
{
	class AJAInputChannel;
}

/**
 * Implements a media player using AJA.
 *
 * The processing of metadata and video frames is delayed until the fetch stage
 * (TickFetch) in order to increase the window of opportunity for receiving AJA
 * frames for the current render frame time code.
 *
 * Depending on whether the media source enables time code synchronization,
 * the player's current play time (CurrentTime) is derived either from the
 * time codes embedded in AJA frames or from the Engine's global time code.
 */
class FAjaMediaPlayer
	: public FMediaIOCorePlayerBase
	, protected AJA::IAJAInputOutputChannelCallbackInterface
{
	using Super = FMediaIOCorePlayerBase;
public:

	/**
	 * Create and initialize a new instance.
	 *
	 * @param InEventSink The object that receives media events from this player.
	 */
	FAjaMediaPlayer(IMediaEventSink& InEventSink);

	/** Virtual destructor. */
	virtual ~FAjaMediaPlayer();

public:

	//~ IMediaPlayer interface

	virtual void Close() override;
	virtual FName GetPlayerName() const override;

	virtual bool Open(const FString& Url, const IMediaOptions* Options) override;

	virtual void TickFetch(FTimespan DeltaTime, FTimespan Timecode) override;
	virtual void TickInput(FTimespan DeltaTime, FTimespan Timecode) override;

	virtual FString GetStats() const override;

protected:

	//~ IAJAInputOutputCallbackInterface interface
	
	virtual void OnInitializationCompleted(bool bSucceed) override;
	virtual bool OnRequestInputBuffer(const AJA::AJARequestInputBufferData& InRequestBuffer, AJA::AJARequestedInputBufferData& OutRequestedBuffer) override;
	virtual bool OnInputFrameReceived(const AJA::AJAInputFrameData& InInputFrame, const AJA::AJAAncillaryFrameData& InAncillaryFrame, const AJA::AJAAudioFrameData& AudioFrame, const AJA::AJAVideoFrameData& VideoFrame) override;
	virtual bool OnOutputFrameCopied(const AJA::AJAOutputFrameData& InFrameData) override;
	virtual void OnCompletion(bool bSucceed) override;

protected:

	/**
	 * Process pending audio and video frames, and forward them to the sinks.
	 */
	void ProcessFrame();
	
protected:

	/** Verify if we lost some frames since last Tick*/
	void VerifyFrameDropCount();


	virtual bool IsHardwareReady() const override;

private:

	/** Audio, MetaData, Texture  sample object pool. */
	FAjaMediaAudioSamplePool* AudioSamplePool;
	FAjaMediaBinarySamplePool* MetadataSamplePool;
	FAjaMediaTextureSamplePool* TextureSamplePool;

	TSharedPtr<FMediaIOCoreBinarySampleBase, ESPMode::ThreadSafe> AjaThreadCurrentAncSample;
	TSharedPtr<FMediaIOCoreBinarySampleBase, ESPMode::ThreadSafe> AjaThreadCurrentAncF2Sample;
	TSharedPtr<FAjaMediaAudioSample, ESPMode::ThreadSafe> AjaThreadCurrentAudioSample;
	TSharedPtr<FAjaMediaTextureSample, ESPMode::ThreadSafe> AjaThreadCurrentTextureSample;

	/** The media sample cache. */
	int32 MaxNumAudioFrameBuffer;
	int32 MaxNumMetadataFrameBuffer;
	int32 MaxNumVideoFrameBuffer;

	/** Current state of the media player. */
	EMediaState AjaThreadNewState;

	/** The media event handler. */
	IMediaEventSink& EventSink;

	/** Number of audio channels in the last received sample. */
	int32 AjaThreadAudioChannels;

	/** Audio sample rate in the last received sample. */
	int32 AjaThreadAudioSampleRate;

	/** Number of frames drop from the last tick. */
	int32 AjaThreadFrameDropCount;
	int32 AjaThreadAutoCirculateAudioFrameDropCount;
	int32 AjaThreadAutoCirculateMetadataFrameDropCount;
	int32 AjaThreadAutoCirculateVideoFrameDropCount;

	/** Number of frames drop from the last tick. */
	uint32 LastFrameDropCount;
	uint32 PreviousFrameDropCount;

	/** Whether to use the time code embedded in AJA frames. */
	bool bEncodeTimecodeInTexel;

	/** Whether to use the timecode embedded in a frame. */
	bool bUseFrameTimecode;

	/** Whether the input is in sRGB and can have a ToLinear conversion. */
	bool bIsSRGBInput;

	/** Which field need to be capture. */
	bool bUseAncillary;
	bool bUseAudio;
	bool bUseVideo;
	bool bVerifyFrameDropCount;

	/** Maps to the current input Device */
	AJA::AJAInputChannel* InputChannel;

	/** Frame Description from capture device */
	AJA::FAJAVideoFormat LastVideoFormatIndex;
	/** Previous frame timecode for stats purpose */
	AJA::FTimecode AjaThreadPreviousFrameTimecode;
};
