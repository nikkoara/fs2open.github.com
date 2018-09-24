// -*- mode: c++; -*-

#ifndef FREESPACE2_TRACING_TRACEEVENTWRITER_H
#define FREESPACE2_TRACING_TRACEEVENTWRITER_H

#include "globalincs/pstypes.h"
#include "tracing/tracing.h"

#include "tracing/ThreadedEventProcessor.h"

#include <fstream>

/** @file
 *  @ingroup tracing
 */

namespace tracing {
class TraceEventWriter {
    std::ofstream _out;
    bool _first_line = true;

public:
    TraceEventWriter ();
    ~TraceEventWriter ();

    void processEvent (const trace_event* event);
};

typedef ThreadedEventProcessor< TraceEventWriter > ThreadedTraceEventWriter;
} // namespace tracing

#endif // FREESPACE2_TRACING_TRACEEVENTWRITER_H
