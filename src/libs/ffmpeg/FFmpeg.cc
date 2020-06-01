// -*- mode: c++; -*-

#include "libs/ffmpeg/FFmpeg.hh"

#include "defs.hh"

#include "FFmpegHeaders.hh"
#include "assert/assert.hh"
#include "log/log.hh"

namespace {
const int MIN_LOG_LEVEL = AV_LOG_WARNING;

bool initialized = false;

void log_callback_report(void *ptr, int level, const char *fmt, va_list vl)
{
        if (level > MIN_LOG_LEVEL) {
                return;
        }

        char buffer[1024];
        int print_prefix = 1;
        av_log_format_line(
                ptr, level, fmt, vl, buffer, sizeof(buffer), &print_prefix);

        WARNINGF(LOCATION, "FFMPEG Log: %s", buffer);
}

void check_version(const char *libname, uint32_t current, uint32_t compiled)
{
        WARNINGF(LOCATION, "FFmpeg: Using %s with version %d.%d.%d. Compiled with version %d.%d.%d", libname, AV_VERSION_MAJOR(current), AV_VERSION_MINOR(current), AV_VERSION_MICRO(current), AV_VERSION_MAJOR(compiled), AV_VERSION_MINOR(compiled), AV_VERSION_MICRO(compiled));

        auto current_major = AV_VERSION_MAJOR(current);
        auto current_minor = AV_VERSION_MINOR(current);

        auto compiled_major = AV_VERSION_MAJOR(compiled);
        auto compiled_minor = AV_VERSION_MINOR(compiled);

        if (current_major != compiled_major) {
                ASSERTX(0, "The major version of the %s library is not the same as the one this executable was compiled with!\nCurrent major version is %" PRIu32 " but this executable was compiled with major version %" PRIu32 ".\nThis may be caused by using outdated DLLs, if you downloaded these builds then try reextracting the zip file.", libname, current_major, compiled_major);
        }

        if (current_minor < compiled_minor) {
                ASSERTX(0, "The minor version of the %s library is not the same as the one this executable was compiled with!\nCurrent minor version is %" PRIu32 " but this executable was compiled with minor version %" PRIu32 ".\nThis may be caused by using outdated DLLs, if you downloaded these builds then try reextracting the zip file.", libname, current_minor, compiled_minor);
        }
}
} // namespace

namespace libs {
namespace ffmpeg {
void initialize()
{
        if (initialized) {
                return;
        }

        // This is deprecated since 58.9.100 and not needed anymore
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)
        av_register_all();
#endif

        check_version("libavcodec", avcodec_version(), LIBAVCODEC_VERSION_INT);
        check_version(
                "libavformat", avformat_version(), LIBAVFORMAT_VERSION_INT);
        check_version("libavutil", avutil_version(), LIBAVUTIL_VERSION_INT);
        check_version(
                "libswresample", swresample_version(), LIBSWRESAMPLE_VERSION_INT);
        check_version("libswscale", swscale_version(), LIBSWSCALE_VERSION_INT);

#ifndef NDEBUG
        av_log_set_callback(&log_callback_report);
        av_log_set_level(MIN_LOG_LEVEL);
#else
        av_log_set_level(AV_LOG_QUIET);
#endif

        WARNINGF(LOCATION, "FFmpeg library initialized!");
        WARNINGF(LOCATION, "FFmpeg: License: %s", avformat_license());

        initialized = true;
}
} // namespace ffmpeg
} // namespace libs
