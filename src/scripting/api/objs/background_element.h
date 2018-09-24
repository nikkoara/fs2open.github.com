// -*- mode: c++; -*-

#ifndef FREESPACE2_SCRIPTING_API_OBJS_BACKGROUND_ELEMENT_H
#define FREESPACE2_SCRIPTING_API_OBJS_BACKGROUND_ELEMENT_H

#include "object/object.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

enum class BackgroundType { Invalid, Bitmap, Sun };

struct background_el_h {
    background_el_h () = default;
    explicit background_el_h (BackgroundType in_type, int in_id);

    BackgroundType type = BackgroundType::Invalid;
    int id = -1;

    bool isValid () const;
};

DECLARE_ADE_OBJ (l_BackgroundElement, background_el_h);

} // namespace api
} // namespace scripting

#endif // FREESPACE2_SCRIPTING_API_OBJS_BACKGROUND_ELEMENT_H
