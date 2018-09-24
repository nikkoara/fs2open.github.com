// -*- mode: c++; -*-

#ifndef FREESPACE2_SCRIPTING_API_OBJS_EYE_H
#define FREESPACE2_SCRIPTING_API_OBJS_EYE_H

#include "scripting/ade_api.h"
#include "model/model.h"

namespace scripting {
namespace api {

class eye_h {
public:
    int model;
    int eye_idx;

    eye_h ();
    eye_h (int n_m, int n_e);
    bool IsValid ();
};

DECLARE_ADE_OBJ (l_Eyepoint, eye_h);

} // namespace api
} // namespace scripting

#endif // FREESPACE2_SCRIPTING_API_OBJS_EYE_H
