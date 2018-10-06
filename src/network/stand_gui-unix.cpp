// -*- mode: c++; -*-

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <streambuf>
#include <algorithm>
#include <memory>
#include <iterator>
#include <cstddef>

#include <json/json.h>

// Copy-Paste from
// http://www.cplusplus.com/faq/sequences/strings/split/#c-tokenizer
struct split_struct {
    enum empties_t { empties_ok, no_empties };
};

template< typename Container >
Container& split (
    Container& result, const typename Container::value_type& s,
    const typename Container::value_type& delimiters,
    split_struct::empties_t empties = split_struct::empties_ok) {
    result.clear ();
    size_t current;
    size_t next = -1;
    do {
        if (empties == split_struct::no_empties) {
            next = s.find_first_not_of (delimiters, next + 1);
            if (next == Container::value_type::npos) break;
            next -= 1;
        }
        current = next + 1;
        next = s.find_first_of (delimiters, current);
        result.push_back (s.substr (current, next - current));
    } while (next != Container::value_type::npos);
    return result;
}
// Copy-Paste end

// Copy-Paste from http://www.adp-gmbh.ch/cpp/common/base64.html
/*
 base64.cpp and base64.h

 Copyright (C) 2004-2008 René Nyffenegger

 This source code is provided 'as-is', without any express or implied
 warranty. In no event will the author be held liable for any damages
 arising from the use of this software.

 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:

 1. The origin of this source code must not be misrepresented; you must not
 claim that you wrote the original source code. If you use this source code
 in a product, an acknowledgment in the product documentation would be
 appreciated but is not required.

 2. Altered source versions must be plainly marked as such, and must not be
 misrepresented as being the original source code.

 3. This notice may not be removed or altered from any source distribution.

 René Nyffenegger rene.nyffenegger@adp-gmbh.ch

 */

static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static inline bool is_base64 (unsigned char c) {
    return (isalnum (c) || (c == '+') || (c == '/'));
}

std::string
base64_encode (unsigned char const* bytes_to_encode, unsigned int in_len) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) +
                              ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) +
                              ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++) ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++) char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] =
            ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] =
            ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++) ret += base64_chars[char_array_4[j]];

        while ((i++ < 3)) ret += '=';
    }

    return ret;
}

std::string base64_decode (std::string const& encoded_string) {
    int in_len = encoded_string.size ();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') &&
           is_base64 (encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find (char_array_4[i]);

            char_array_3[0] =
                (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) +
                              ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++) ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++) char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find (char_array_4[j]);

        char_array_3[0] =
            (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] =
            ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}
// Copy-Paste end

/*
 Copyright (c) 2013 Eli2
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions: The above copyright
 notice and this permission notice shall be included in all copies or
 substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS",
 WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "network/stand_gui.h"

#include "globalincs/pstypes.h"
#include "gamesequence/gamesequence.h"
#include "playerman/player.h"
#include "mission/missiongoals.h"
#include "ship/ship.h"
#include "osapi/osapi.h"

#include "network/multi.h"
#include "network/multiutil.h"
#include "network/multimsgs.h"
#include "network/multi_pmsg.h"
#include "network/multi_kick.h"
#include "network/multi_endgame.h"

#include "fs2netd/fs2netd_client.h"

#include "mongoose/mongoose.h"

#define ARRAY_SIZE(array) (sizeof (array) / sizeof (array[0]))

class WebapiCommand {
public:
    virtual ~WebapiCommand (){};
    virtual void execute () = 0;
};

class ChangeServerInformationCommand : public WebapiCommand {
    std::string name;
    bool hasName;

    std::string passwd;
    bool hasPasswd;

    int framecap;

public:
    ChangeServerInformationCommand ()
        : hasName (false), hasPasswd (false), framecap (0) {}

    void setFrameCap (int cap) { framecap = cap; }

    void setName (const char* s) {
        hasName = true;
        name = s;
    }

    void setName (const std::string& s) {
        hasName = true;
        name = s;
    }

    void setPasswd (const char* s) {
        hasPasswd = true;
        passwd = s;
    }

    void setPasswd (const std::string& s) {
        hasPasswd = true;
        passwd = s;
    }

    void execute () override {
        if (hasName) {
            strcpy_s (Netgame.name, name.c_str ());
            strcpy_s (Multi_options_g.std_pname, name.c_str ());
            // update fs2netd with the info
            if (MULTI_IS_TRACKER_GAME) {
                fs2netd_gameserver_disconnect ();
                os_sleep (50);
                fs2netd_gameserver_start ();
            }
        }

        if (hasPasswd) {
            strcpy_s (Multi_options_g.std_passwd, passwd.c_str ());
        }

        if (framecap) { Multi_options_g.std_framecap = framecap; }
    }
};

class KickPlayerCommand : public WebapiCommand {
public:
    KickPlayerCommand (int playerId) : mPlayerId (playerId) {}

    void execute () override {
        size_t foundPlayerIndex = MAX_PLAYERS;
        for (size_t idx = 0; idx < MAX_PLAYERS; idx++) {
            if (MULTI_CONNECTED (Net_players[idx])) {
                if (Net_players[idx].player_id == mPlayerId) {
                    foundPlayerIndex = idx;
                }
            }
        }

        if (foundPlayerIndex < MAX_PLAYERS) {
            multi_kick_player (foundPlayerIndex, 0);
        }
    }

private:
    int mPlayerId;
};

class ShutdownServerCommand : public WebapiCommand {
public:
    void execute () override { gameseq_post_event (GS_EVENT_QUIT_GAME); }
};

class UpdateMissionsCommand : public WebapiCommand {
public:
    void execute () override {
        if (MULTI_IS_TRACKER_GAME) {
            // delete mvalid.cfg if it exists
            cf_delete (MULTI_VALID_MISSION_FILE, CF_TYPE_DATA);

            // refresh missions
            multi_update_valid_missions ();
        }
    }
};

class ResetGameCommand : public WebapiCommand {
public:
    void execute () override { multi_quit_game (PROMPT_NONE); }
};

class ResetFs2NetCommand : public WebapiCommand {
public:
    void execute () override { fs2netd_reset_connection (); }
};

SDL_mutex* webapiCommandQueueMutex = SDL_CreateMutex ();
std::vector< WebapiCommand* > webapiCommandQueue;

void webapiAddCommand (WebapiCommand* command) {
    SDL_mutexP (webapiCommandQueueMutex);

    webapiCommandQueue.push_back (command);

    SDL_mutexV (webapiCommandQueueMutex);
}

void webapiExecuteCommands () {
    SDL_mutexP (webapiCommandQueueMutex);

    for (auto p : webapiCommandQueue) {
        if (p) {
            p->execute ();
            delete p;
        }
    }

    webapiCommandQueue.clear ();

    SDL_mutexV (webapiCommandQueueMutex);
}

struct LogResource {
    struct LogResourceEntry {
        long timestamp;
        Json::Value data;
    };

    long currentTimestamp;
    std::list< LogResourceEntry > entries;

    void addEntity (Json::Value value) {
        entries.push_back ({ currentTimestamp++, std::move (value) });
        if (entries.size () > 500) entries.pop_front ();
    }

    Json::Value getEntriesAfter (long after) const {
        Json::Value result;

        for (const auto& entry : entries) {
            if (entry.timestamp >= after) {
                Json::Value data = entry.data;
                data["timestamp"] = Json::Value::Int64 (entry.timestamp);
                result.append (data);
            }
        }

        return result;
    }
};

SDL_mutex* webapi_dataMutex = SDL_CreateMutex ();
netgame_info webapi_netgameInfo;
std::map< short, net_player > webapiNetPlayers;
float webui_fps;
float webui_missiontime;
std::list< mission_goal > webuiMissionGoals;
LogResource webapi_chatLog;
LogResource webapi_debugLog;

enum HttpStatuscode {
    HTTP_200_OK,
    HTTP_401_UNAUTHORIZED,
    HTTP_404_NOT_FOUND,
    HTTP_500_INTERNAL_SERVER_ERROR
};

static void sendResponse (
    mg_connection* conn, std::string const& data, HttpStatuscode status) {
    std::stringstream headerStream;

    headerStream << "HTTP/1.0 ";
    switch (status) {
    case HTTP_200_OK: headerStream << "200 OK"; break;
    case HTTP_401_UNAUTHORIZED: headerStream << "401 Unauthorized"; break;
    case HTTP_404_NOT_FOUND: headerStream << "404 Not Found"; break;
    case HTTP_500_INTERNAL_SERVER_ERROR:
        headerStream << "500 Internal Server Error";
        break;
    }
    headerStream << "\r\n";

    if (data.length () > 0) {
        headerStream << "Content-Length: " << data.length () << "\r\n";
        headerStream << "Content-Type: application/json\r\n\r\n";
    }

    std::string resultString;
    resultString += headerStream.str ();
    resultString += data;

    mg_write (conn, resultString.c_str (), (int)resultString.length ());
}

static void
sendJsonResponse (mg_connection* connection, const Json::Value& response) {
    std::stringstream ss;
    ss << response;
    sendResponse (connection, ss.str (), HTTP_200_OK);
}

// =============================================================================

struct ResourceContext {
    Json::Value requestEntity;
    std::map< std::string, std::string > parameters;
};

typedef Json::Value (*resourceHandler) (ResourceContext*);

struct Resource {
    std::string path;
    std::string method;
    resourceHandler handler;
};

static inline Json::Value emptyResource (ResourceContext* /*context*/) {
    return {};
}

static inline Json::Value serverGet (ResourceContext* /*context*/) {
    Json::Value result;

    result["name"] = Multi_options_g.std_pname;
    result["password"] = Multi_options_g.std_passwd;
    result["framecap"] = Multi_options_g.std_framecap;

    return result;
}

static Json::Value serverPut (ResourceContext* context) {
    auto p = std::make_unique< ChangeServerInformationCommand > ();

    const auto& req = context->requestEntity;

    if (req.isMember ("name")) p->setName (req["name"].asString ());

    if (req.isMember ("password")) p->setPasswd (req["password"].asString ());

    if (req.isMember ("password")) p->setFrameCap (req["framecap"].asInt ());

    webapiAddCommand (p.release ());

    return {};
}

Json::Value serverDelete (ResourceContext* /*context*/) {
    webapiAddCommand (new ShutdownServerCommand ());
    return {};
}

Json::Value refreshMissions (ResourceContext* /*context*/) {
    webapiAddCommand (new UpdateMissionsCommand ());
    return {};
}

Json::Value serverResetGame (ResourceContext* /*context*/) {
    webapiAddCommand (new ResetGameCommand ());
    return {};
}

Json::Value fs2netReset (ResourceContext* /*context*/) {
    webapiAddCommand (new ResetFs2NetCommand ());
    return {};
}

Json::Value netgameInfoGet (ResourceContext* /*context*/) {
    Json::Value result;

    result["name"] = webapi_netgameInfo.name;
    result["mission"] = webapi_netgameInfo.mission_name;
    result["campaign"] = webapi_netgameInfo.campaign_name;
    result["maxPlayers"] = webapi_netgameInfo.max_players;
    result["maxObservers"] = webapi_netgameInfo.options.max_observers;
    result["respawn"] = webapi_netgameInfo.respawn;
    result["gameState"] = webapi_netgameInfo.game_state;
    result["security"] = webapi_netgameInfo.security;

    return result;
}

Json::Value missionGet (ResourceContext* /*context*/) {
    Json::Value fpsEntity;

    fpsEntity["fps"] = webui_fps;
    fpsEntity["time"] = webui_missiontime;

    return fpsEntity;
}

Json::Value missionGoalsGet (ResourceContext* /*context*/) {
    Json::Value goals;

    for (const auto& goal : webuiMissionGoals) {
        Json::Value goalEntity;

        goalEntity["name"] = goal.name;
        goalEntity["message"] = goal.message;
        goalEntity["score"] = goal.score;
        goalEntity["team"] = goal.team;

        const char* typeString;

        switch (goal.type) {
        case PRIMARY_GOAL: typeString = "primary"; break;
        case SECONDARY_GOAL: typeString = "secondary"; break;
        case BONUS_GOAL: typeString = "bonus"; break;
        default: typeString = "error"; break;
        };

        goalEntity["type"] = typeString;

        const char* statusString;

        switch (goal.satisfied) {
        case GOAL_FAILED: statusString = "failed"; break;
        case GOAL_COMPLETE: statusString = "complete"; break;
        case GOAL_INCOMPLETE: statusString = "incomplete"; break;
        default: statusString = "error"; break;
        };

        goalEntity["status"] = statusString;

        goals.append (goalEntity);
    }

    return goals;
}

Json::Value playerGet (ResourceContext* /*context*/) {
    Json::Value playerList;

    for (const auto& pair : webapiNetPlayers) {
        char address[256];

        const auto& p = pair.second;

        sprintf (
            address, "%u.%u.%u.%u:%u", p.p_info.addr.addr[0],
            p.p_info.addr.addr[1], p.p_info.addr.addr[2],
            p.p_info.addr.addr[3], p.p_info.addr.port);

        Json::Value obj;

        obj["id"] = p.player_id;
        obj["address"] = address;
        obj["ping"] = p.s_info.ping.ping_avg;

        obj["host"] = bool(MULTI_HOST (p));
        obj["observer"] = bool(MULTI_OBSERVER (p));

        obj["callsign"] = p.m_player->callsign;
        obj["ship"] = Ship_info[p.p_info.ship_class].name;

        playerList.append (obj);
    }

    return playerList;
}

Json::Value playerDelete (ResourceContext* context) {
    int playerId = atoi (context->parameters["playerId"].c_str ());
    webapiAddCommand (new KickPlayerCommand (playerId));
    return {};
}

net_player* playerForId (int playerId) {
    net_player* foundPlayer = NULL;
    for (size_t idx = 0; idx < MAX_PLAYERS; idx++) {
        if (MULTI_CONNECTED (Net_players[idx])) {
            if (Net_players[idx].player_id == playerId) {
                foundPlayer = &Net_players[idx];
            }
        }
    }
    return foundPlayer;
}

Json::Value playerMissionScoreAlltimeGet (ResourceContext* context) {
    net_player* p =
        playerForId (atoi (context->parameters["playerId"].c_str ()));

    Json::Value obj2;
    if (p == NULL || p->m_player == NULL) return obj2;

    scoring_struct* scores = &(p->m_player->stats);

    obj2["score"] = scores->m_score;
    obj2["kills"] = scores->m_kill_count;
    obj2["kills-enemy"] = scores->m_kill_count_ok;
    obj2["kills-friendly"] = scores->m_bonehead_kills;
    obj2["assists"] = scores->m_assists;
    obj2["shots-primary"] = scores->mp_shots_fired;
    obj2["shots-secondary"] = scores->ms_shots_fired;
    obj2["hits-primary"] = scores->mp_shots_hit;
    obj2["hits-secondary"] = scores->ms_shots_hit;
    obj2["hits-friendly-primary"] = scores->mp_bonehead_hits;
    obj2["hits-friendly-secondary"] = scores->ms_bonehead_hits;

    return obj2;
}

Json::Value playerMissionScoreMissionGet (ResourceContext* context) {
    net_player* p =
        playerForId (atoi (context->parameters["playerId"].c_str ()));

    Json::Value obj;

    if (0 == p || 0 == p->m_player) return obj;

    scoring_struct* scores = &(p->m_player->stats);

    obj["score"] = scores->score;
    obj["kills"] = scores->kill_count;
    obj["kills-enemy"] = scores->kill_count_ok;
    obj["kills-friendly"] = scores->bonehead_kills;
    obj["assists"] = scores->assists;
    obj["shots-primary"] = scores->p_shots_fired;
    obj["shots-secondary"] = scores->s_shots_fired;
    obj["hits-primary"] = scores->p_shots_hit;
    obj["hits-secondary"] = scores->s_shots_hit;
    obj["hits-friendly-primary"] = scores->p_bonehead_hits;
    obj["hits-friendly-secondary"] = scores->s_bonehead_hits;

    return obj;
}

int afterTimestamp (ResourceContext* context) {
    std::map< std::string, std::string >::iterator iter =
        context->parameters.find ("after");
    if (iter != context->parameters.end ()) {
        return atoi (iter->second.c_str ());
    }

    return 0;
}

Json::Value chatGet (ResourceContext* context) {
    int after = afterTimestamp (context);
    return webapi_chatLog.getEntriesAfter (after);
}

Json::Value chatPost (ResourceContext* context) {
    const auto& req = context->requestEntity;

    if (req.isMember ("message")) {
        const auto s = req["message"].asString ();
        send_game_chat_packet (Net_player, s.c_str (), MULTI_MSG_ALL, 0);
        std_add_chat_text (s.c_str (), 0, 1);
    }

    return emptyResource (context);
}

Json::Value debugGet (ResourceContext* context) {
    int after = afterTimestamp (context);
    return webapi_debugLog.getEntriesAfter (after);
}

struct Resource resources[] = {
    { "api/1/auth", "GET", &emptyResource },
    { "api/1/server", "GET", &serverGet },
    { "api/1/server", "PUT", &serverPut },
    { "api/1/server", "DELETE", &serverDelete },
    { "api/1/server/refreshMissions", "GET", &refreshMissions },
    { "api/1/server/resetGame", "GET", &serverResetGame },
    { "api/1/server/fs2net/reset", "GET", &fs2netReset },
    { "api/1/netgameInfo", "GET", &netgameInfoGet },
    { "api/1/mission", "GET", &missionGet },
    { "api/1/mission/goals", "GET", &missionGoalsGet },
    { "api/1/player", "GET", &playerGet },
    { "api/1/player/*", "DELETE", &playerDelete },
    { "api/1/player/*/score/mission", "GET", &playerMissionScoreMissionGet },
    { "api/1/player/*/score/alltime", "GET", &playerMissionScoreAlltimeGet },
    { "api/1/chat", "GET", &chatGet },
    { "api/1/chat", "POST", &chatPost },
    { "api/1/debug", "GET", &debugGet }
};

static bool
webserverApiRequest (mg_connection* conn, const mg_request_info* ri) {
    std::string resourcePath (ri->uri);

    resourcePath.erase (0, 1);
    std::vector< std::string > pathParts;
    split (pathParts, resourcePath, "/", split_struct::no_empties);

    Json::Value result;

    std::string method (ri->request_method);

    for (size_t i = 0; i < ARRAY_SIZE (resources); i++) {
        Resource* r = &resources[i];
        std::vector< std::string > resourcePathParts;
        split (resourcePathParts, r->path, "/", split_struct::no_empties);

        if (resourcePathParts.size () == pathParts.size ()) {
            ResourceContext context;

            bool pathMatch = true;
            for (size_t u = 0; u < resourcePathParts.size (); u++) {
                // TODO this is kind of a hack
                if (resourcePathParts.at (u) == "*") {
                    context.parameters["playerId"] = pathParts.at (u);
                }
                else if (resourcePathParts.at (u) != pathParts.at (u)) {
                    pathMatch = false;
                    break;
                }
            }

            if (pathMatch && r->method == method) {
                std::string userNameAndPassword;

                userNameAndPassword += Multi_options_g.webapiUsername.c_str ();
                userNameAndPassword += ":";
                userNameAndPassword += Multi_options_g.webapiPassword.c_str ();

                std::string basicAuthValue = "Basic ";

                basicAuthValue += base64_encode (
                    reinterpret_cast< const unsigned char* > (
                        userNameAndPassword.c_str ()),
                    userNameAndPassword.length ());

                const char* authValue = mg_get_header (conn, "Authorization");
                if (authValue == NULL ||
                    strcmp (authValue, basicAuthValue.c_str ()) != 0) {
                    sendResponse (conn, std::string (), HTTP_401_UNAUTHORIZED);
                    return true;
                }

                if (ri->query_string) {
                    std::string query (ri->query_string);
                    std::vector< std::string > queryPairs;
                    split (queryPairs, query, "&", split_struct::no_empties);

                    for (std::vector< std::string >::const_iterator iter =
                             queryPairs.begin ();
                         iter != queryPairs.end (); ++iter) {
                        std::vector< std::string > temp;

                        split (temp, *iter, "=", split_struct::no_empties);

                        if (temp.size () == 2) {
                            context.parameters[temp[0]] = temp[1];
                        }
                    }
                }

                char entityBuffer[1024];
                memset (entityBuffer, 0, sizeof (entityBuffer));

                /*int readBytes = */ mg_read (
                    conn, &entityBuffer, sizeof (entityBuffer));

                {
                    Json::CharReaderBuilder parser;
                    parser["collectComments"] = false;

                    std::stringstream ss (entityBuffer);

                    std::string ignore;
                    Json::Value root;

                    /* bool ignore2 = */
                    Json::parseFromStream (parser, ss, &root, &ignore);
                }

                SDL_mutexP (webapi_dataMutex);
                result = r->handler (&context);
                SDL_mutexV (webapi_dataMutex);

                break;
            }
        }
    }

    if (result) {
        sendJsonResponse (conn, result);
        return true;
    }

    return false;
}

static void*
webserverCallback (enum mg_event event, struct mg_connection* conn) {
    const struct mg_request_info* ri = mg_get_request_info (conn);

    switch (event) {
    case MG_EVENT_LOG: {
        const char* msg = (const char*)ri->ev_data;
        ERRORF (LOCATION, "Webapi error: %s\n", msg);
        break;
    }
    case MG_NEW_REQUEST: {
        bool requestHandled = webserverApiRequest (conn, ri);
        return requestHandled ? const_cast< char* > ("") : NULL;
    }
    default: break;
    };
    return NULL;
}

struct mg_context* webserverContext = NULL;

void webapi_shutdown () {
    if (webserverContext) {
        WARNINGF (LOCATION, "Webapi shutting down\n");
        mg_stop (webserverContext);
    }
}

void std_init_standalone () { atexit (webapi_shutdown); }

void std_configLoaded (multi_global_options* options) {
    webapi_shutdown ();

    char buffer[16];
    sprintf (buffer, "%d", options->webapiPort);

    const char* mgOptions[] = { "listening_ports",
                                buffer,
                                "document_root",
                                options->webuiRootDirectory.c_str (),
                                "num_threads",
                                "4",
                                0 };

    WARNINGF (
        LOCATION, "Webapi starting on port: %d with document root at: %s\n",
        options->webapiPort, options->webuiRootDirectory.c_str ());

    webserverContext = mg_start (&webserverCallback, NULL, mgOptions);
}

void std_add_chat_text (
    const char* text, int /*player_index*/, int /*add_id*/) {
    Json::Value msg;
    msg["message"] = text;

    webapi_chatLog.addEntity (msg);
}

void std_debug_multilog_add_line (const char* str) {
    std::vector< std::string > debugMessages;
    split (debugMessages, std::string (str), "\n", split_struct::no_empties);

    for (const auto& s : debugMessages) {
        Json::Value msg;
        msg["message"] = s;
        webapi_debugLog.addEntity (msg);
    }
}

// =============================================================================

std::vector< std::string > bannedPlayers;

void std_add_ban (const char* name) {
    bannedPlayers.push_back (std::string (name));
}

int std_player_is_banned (const char* name) {
    return std::find (
               bannedPlayers.begin (), bannedPlayers.end (),
               std::string (name)) != bannedPlayers.end ()
               ? 1
               : 0;
}

/**
 * return 1 if the standalone is host password protected, otherwise 0
 */
int std_is_host_passwd () {
    return (Multi_options_g.std_passwd[0] != '\0') ? 1 : 0;
}

void std_set_standalone_fps (float fps) { webui_fps = fps; }

void std_do_gui_frame () {
    SDL_mutexP (webapi_dataMutex);

    webapi_netgameInfo = Netgame;

    // Update player data
    webapiNetPlayers.clear ();

    for (size_t idx = 0; idx < MAX_PLAYERS; idx++) {
        if (MULTI_CONNECTED (Net_players[idx]) &&
            (Net_player != &Net_players[idx])) {
            net_player* p = &Net_players[idx];

            webapiNetPlayers[p->player_id] = *p;
        }
    }

    // Update mission data
    webui_missiontime = f2fl (Missiontime);

    webuiMissionGoals.clear ();
    for (int idx = 0; idx < Num_goals; idx++) {
        webuiMissionGoals.push_back (Mission_goals[idx]);
    }

    SDL_mutexV (webapi_dataMutex);

    webapiExecuteCommands ();
}

// set the game name for the standalone. passing NULL uses the default
void std_connect_set_gamename (char* name) {
    // use the default name for now
    if (name == NULL) {
        // if a permanent name exists, use that instead of the default
        if (strlen (Multi_options_g.std_pname)) {
            strcpy_s (Netgame.name, Multi_options_g.std_pname);
        }
        else {
            strcpy_s (Netgame.name, XSTR ("Standalone Server", 916));
        }
    }
    else {
        strcpy_s (Netgame.name, name);

        // update fs2netd
        if (MULTI_IS_TRACKER_GAME) {
            fs2netd_gameserver_disconnect ();
            os_sleep (50);
            fs2netd_gameserver_start ();
        }
    }
}

/**
 * Unused methods from the original API below,
 * most of this stuff is now done in std_do_gui_frame
 */
void std_add_player (net_player* /*p*/) {}
/* The return value does nothing, except cause the two functions below to be
 * called*/
int std_remove_player (net_player* /*p*/) { return 0; }
void std_connect_set_host_connect_status () {}
int std_connect_set_connect_count () { return 0; }
void std_update_player_ping (net_player* /*p*/) {}
void std_multi_setup_goal_tree () {}
void std_multi_add_goals () {}
void std_multi_update_goals () {}
void std_multi_update_netgame_info_controls () {}
void std_multi_set_standalone_mission_name (char* /*mission_name*/) {}
void std_gen_set_text (const char* /*str*/, int /*field_num*/) {}
void std_create_gen_dialog (const char* /*title*/) {}
void std_destroy_gen_dialog () {}
int std_gen_is_active () { return 0; }
void std_debug_set_standalone_state_string (const char* /*str*/) {}
void std_reset_standalone_gui () {}
void std_reset_timestamps () {}
void std_multi_set_standalone_missiontime (float /*mission_time*/) {}

// stub - not required for *nix standalone
void std_init_os () {}
