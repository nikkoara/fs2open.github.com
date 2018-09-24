// -*- mode: c++; -*-

#ifndef FREESPACE2_CUTSCENE_FFMPEG_AUDIODECODER_H
#define FREESPACE2_CUTSCENE_FFMPEG_AUDIODECODER_H

#include "cutscene/ffmpeg/internal.h"
#include "cutscene/ffmpeg/FFMPEGDecoder.h"

namespace cutscene {
namespace ffmpeg {

class AudioDecoder : public FFMPEGStreamDecoder< AudioFrame > {
private:
    SwrContext* m_resampleCtx;

    uint8_t** m_outData;
    int m_outLinesize;

    int m_maxOutNumSamples;
    int m_outNumSamples;

    std::vector< short > m_audioBuffer;

    void handleDecodedFrame (AVFrame* frame);

    void flushAudioBuffer ();

public:
    explicit AudioDecoder (DecoderStatus* status);

    ~AudioDecoder () override;

    void decodePacket (AVPacket* packet) override;

    void finishDecoding () override;

    void flushBuffers () override;
};
} // namespace ffmpeg
} // namespace cutscene

#endif // FREESPACE2_CUTSCENE_FFMPEG_AUDIODECODER_H
