// -*- mode: c++; -*-

#ifndef FREESPACE2_CUTSCENE_FFMPEG_SUBTITLEDECODER_H
#define FREESPACE2_CUTSCENE_FFMPEG_SUBTITLEDECODER_H

#include "cutscene/ffmpeg/internal.h"
#include "cutscene/ffmpeg/FFMPEGDecoder.h"

namespace cutscene {
namespace ffmpeg {

class SubtitleDecoder : public FFMPEGStreamDecoder< SubtitleFrame > {
public:
    explicit SubtitleDecoder (DecoderStatus* status);

    ~SubtitleDecoder () override;

    void decodePacket (AVPacket* packet) override;

    void finishDecoding () override;
    void pushSubtitleFrame (AVPacket* subtitle, AVSubtitle* pSubtitle);
    void flushBuffers () override;
};

} // namespace ffmpeg
} // namespace cutscene

#endif // FREESPACE2_CUTSCENE_FFMPEG_SUBTITLEDECODER_H
