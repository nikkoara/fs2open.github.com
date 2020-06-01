// -*- mode: c++; -*-

#ifndef FREESPACE2_TRACING_MAINFRAMETIMER_HH
#define FREESPACE2_TRACING_MAINFRAMETIMER_HH

#include "defs.hh"

#include <fstream>

#include "tracing/ThreadedEventProcessor.hh"
#include "tracing/tracing.hh"

/** @file
 *  @ingroup tracing
 */

namespace tracing {
class MainFrameTimer {
        std::ofstream _out;

        std::uint64_t _begin_time = 0;

public:
        MainFrameTimer();
        ~MainFrameTimer();

        void processEvent(const trace_event *event);
};

typedef ThreadedEventProcessor< MainFrameTimer > ThreadedMainFrameTimer;
} // namespace tracing

#endif // FREESPACE2_TRACING_MAINFRAMETIMER_HH
