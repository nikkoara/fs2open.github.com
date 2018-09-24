// -*- mode: c++; -*-

#ifndef FREESPACE2_LIBS_FFMPEG_FFMPEG_H
#define FREESPACE2_LIBS_FFMPEG_FFMPEG_H

#include "globalincs/pstypes.h"

#include <exception>
#include <stdexcept>

namespace libs {
namespace ffmpeg {

void initialize ();

class FFmpegException : public std::runtime_error {
public:
    explicit FFmpegException (const std::string& msg)
        : std::runtime_error (msg) {}
    ~FFmpegException () noexcept override {}
};

} // namespace ffmpeg
} // namespace libs

#endif // FREESPACE2_LIBS_FFMPEG_FFMPEG_H
