// -*- mode: c++; -*-

#ifndef FREESPACE2_SCRIPTING_API_OBJS_TIME_OBJ_H
#define FREESPACE2_SCRIPTING_API_OBJS_TIME_OBJ_H

#include "scripting/ade.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

DECLARE_ADE_OBJ (l_Timestamp, uint64_t);

DECLARE_ADE_OBJ (l_TimeSpan, int64_t);

} // namespace api
} // namespace scripting

#endif // FREESPACE2_SCRIPTING_API_OBJS_TIME_OBJ_H
