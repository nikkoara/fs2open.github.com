// -*- mode: c++; -*-

#ifndef FREESPACE2_PARSE_GENERIC_LOG_H
#define FREESPACE2_PARSE_GENERIC_LOG_H

#include "defs.hpp"

#include <string>

#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>

#define FS2_LOG(logger, channel, severity) BOOST_LOG_CHANNEL_SEV( \
        logger, channel, boost::log::trivial::severity)

namespace fs2 {
namespace log {

using logger_type = boost::log::sources::severity_channel_logger<
    boost::log::trivial::severity_level, std::string >;

void init ();

} // namespace log
} // namespace fs2

#endif // FREESPACE2_PARSE_GENERIC_LOG_H
