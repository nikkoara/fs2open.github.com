// -*- mode: c++; -*-

#ifndef FREESPACE2_LOG_LOG_HH
#define FREESPACE2_LOG_LOG_HH

#include "defs.hh"

#include "util/fmt.hh"

#include <string>

#include <boost/log/trivial.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>

using fs2_logger_type = boost::log::sources::severity_channel_logger_mt<
    boost::log::trivial::severity_level, std::string >;

BOOST_LOG_GLOBAL_LOGGER(fs2_logger, fs2_logger_type)

#define FS2_LOG(channel, severity) BOOST_LOG_CHANNEL_SEV( \
        fs2_logger::get (), channel, boost::log::trivial::severity)

#define FS2_EE(channel) FS2_LOG (channel, error)
#define FS2_WW(channel) FS2_LOG (channel, warning)
#define FS2_II(channel) FS2_LOG (channel, info)

#define ERROR FS2_EE
#define WARN  FS2_WW
#define INFO  FS2_II

#define EE  ERROR
#define WW  WARN
#define II  INFO

#define FS2_ERROR ERROR
#define FS2_WARN  WARN
#define FS2_INFO  INFO

#ifndef NDEBUG
#  define ERRORF   EE ("general") << fs2_fmt
#  define WARNINGF WW ("general") << fs2_fmt
#else
#  define ERRORF(...)   ((void)0)
#  define WARNINGF(...) ((void)0)
#endif // NDEBUG

#define WWF WARNINGF
#define FS2_WARNINGF WARNINGF

#define RELEASE_WARNINGF WARNINGF

#endif // FREESPACE2_LOG_LOG_HH
