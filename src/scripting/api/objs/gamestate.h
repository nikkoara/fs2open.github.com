// -*- mode: c++; -*-

#ifndef FREESPACE2_SCRIPTING_API_OBJS_GAMESTATE_H
#define FREESPACE2_SCRIPTING_API_OBJS_GAMESTATE_H

#include "scripting/ade_api.h"
#include "gamesequence/gamesequence.h"

namespace scripting {
namespace api {

class gamestate_h {
private:
    int sdx;

public:
    gamestate_h ();
    gamestate_h (int n_state);

    bool IsValid ();

    int Get ();
};

DECLARE_ADE_OBJ (l_GameState, gamestate_h);

} // namespace api
} // namespace scripting

#endif // FREESPACE2_SCRIPTING_API_OBJS_GAMESTATE_H
