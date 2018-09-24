// -*- mode: c++; -*-

#ifndef FREESPACE2_TRACING_MAINFRAMETIMER_H
#define FREESPACE2_TRACING_MAINFRAMETIMER_H

#include "globalincs/pstypes.h"
#include "tracing/tracing.h"

#include "tracing/ThreadedEventProcessor.h"

#include <fstream>

/** @file
 *  @ingroup tracing
 */

namespace tracing {
class MainFrameTimer {
    std::ofstream _out;

    std::uint64_t _begin_time = 0;

public:
    MainFrameTimer ();
    ~MainFrameTimer ();

    void processEvent (const trace_event* event);
};

typedef ThreadedEventProcessor< MainFrameTimer > ThreadedMainFrameTimer;
} // namespace tracing

#endif // FREESPACE2_TRACING_MAINFRAMETIMER_H
