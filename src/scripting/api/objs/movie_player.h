// -*- mode: c++; -*-

#ifndef FREESPACE2_SCRIPTING_API_OBJS_MOVIE_PLAYER_H
#define FREESPACE2_SCRIPTING_API_OBJS_MOVIE_PLAYER_H

#include "scripting/ade_api.h"

#include "cutscene/player.h"

namespace scripting {
namespace api {

class movie_player_h {
    std::unique_ptr< cutscene::Player > _player;

public:
    explicit movie_player_h (std::unique_ptr< cutscene::Player >&& player);

    cutscene::Player* player ();

    bool isValid () const;
};

DECLARE_ADE_OBJ (l_MoviePlayer, movie_player_h*);

} // namespace api
} // namespace scripting

#endif // FREESPACE2_SCRIPTING_API_OBJS_MOVIE_PLAYER_H
