// -*- mode: c++; -*-

#include "globalincs/pstypes.h"
#include "gamesequence/gamesequence.h"
#include "freespace2/freespace.h"
#include "parse/parselo.h"

#include <fstream>

#include <fcntl.h>
#include <sys/stat.h>

namespace {
const char* ORGANIZATION_NAME = "HardLightProductions";
const char* APPLICATION_NAME = "FreeSpaceOpen";

char* preferencesPath = nullptr;

bool checkedLegacyMode = false;
bool legacyMode = false;

std::vector< std::unique_ptr< os::Viewport > > viewports;
os::Viewport* mainViewPort = nullptr;
SDL_Window* mainSDLWindow = nullptr;

const char* getPreferencesPath () {
    // Lazily initialize the preferences path
    if (!preferencesPath) {
        preferencesPath =
            SDL_GetPrefPath (ORGANIZATION_NAME, APPLICATION_NAME);
        if (!preferencesPath) {
            ERRORF (
                LOCATION, "Failed to get preferences path from SDL: %s\n",
                SDL_GetError ());
        }
    }

    if (preferencesPath) { return preferencesPath; }
    else {
        // No preferences path, try current directory
        return "." DIR_SEPARATOR_STR;
    }
}

bool fAppActive = false;
bool window_event_handler (const SDL_Event& e) {
    ASSERTX (
        mainSDLWindow != nullptr,
        "This function may only be called with a valid SDL Window.");
    if (os::events::isWindowEvent (e, mainSDLWindow)) {
        switch (e.window.event) {
        case SDL_WINDOWEVENT_MINIMIZED:
        case SDL_WINDOWEVENT_FOCUS_LOST: {
            if (fAppActive) {
                if (!Cmdline_no_unfocus_pause) { game_pause (); }

                fAppActive = false;
            }
            break;
        }
        case SDL_WINDOWEVENT_MAXIMIZED:
        case SDL_WINDOWEVENT_RESTORED:
        case SDL_WINDOWEVENT_FOCUS_GAINED: {
            if (!fAppActive) {
                if (!Cmdline_no_unfocus_pause) { game_unpause (); }

                fAppActive = true;
            }
            break;
        }
        case SDL_WINDOWEVENT_CLOSE:
            gameseq_post_event (GS_EVENT_QUIT_GAME);
            break;
        }

        gr_activate (fAppActive);

        return true;
    }

    return false;
}

bool quit_handler (const SDL_Event& /*e*/) {
    gameseq_post_event (GS_EVENT_QUIT_GAME);
    return true;
}

} // namespace

// os-wide globals
static char szWinTitle[128];
static char szWinClass[128];
static int Os_inited = 0;

static std::vector< SDL_Event > buffered_events;

int Os_debugger_running = 0;

static bool user_dir_initialized = false;
static std::string Os_user_dir_legacy;

const char* os_get_legacy_user_dir () {
    if (user_dir_initialized) { return Os_user_dir_legacy.c_str (); }

    extern const char* Osreg_user_dir_legacy;
    sprintf (
        Os_user_dir_legacy, "%s/%s", getenv ("HOME"), Osreg_user_dir_legacy);
    user_dir_initialized = true;

    return Os_user_dir_legacy.c_str ();
}

void os_deinit ();

// If app_name is NULL or ommited, then TITLE is used
// for the app name, which is where registry keys are stored.
void os_init (const char* wclass, const char* title, const char* app_name) {
    if (app_name == nullptr || !app_name[0]) {
        os_init_registry_stuff (Osreg_company_name, title);
    }
    else {
        os_init_registry_stuff (Osreg_company_name, app_name);
    }

    strcpy_s (szWinTitle, title);
    strcpy_s (szWinClass, wclass);

    SDL_version compiled;
    SDL_version linked;

    SDL_VERSION (&compiled);
    SDL_GetVersion (&linked);

    WARNINGF (
        LOCATION, "  Initializing SDL %d.%d.%d (compiled with %d.%d.%d)...\n",
        linked.major, linked.minor, linked.patch, compiled.major,
        compiled.minor, compiled.patch);

    if (SDL_Init (SDL_INIT_EVENTS) < 0) {
        fprintf (stderr, "Couldn't init SDL: %s", SDL_GetError ());
        ERRORF (LOCATION, "Couldn't init SDL: %s", SDL_GetError ());

        exit (1);
        return;
    }

#ifdef FS2_VOICER
    SDL_EventState (
        SDL_SYSWMEVENT,
        SDL_ENABLE); // We currently only need this for voice recognition
#endif

    // initialized
    Os_inited = 1;

    os::events::addEventListener (
        SDL_WINDOWEVENT, os::events::DEFAULT_LISTENER_WEIGHT,
        window_event_handler);
    os::events::addEventListener (
        SDL_QUIT, os::events::DEFAULT_LISTENER_WEIGHT, quit_handler);
}

// set the main window title
void os_set_title (const char* title) {
    ASSERTX (
        mainSDLWindow != nullptr,
        "This function may only be called with a valid SDL Window.");
    strcpy_s (szWinTitle, title);

    SDL_SetWindowTitle (mainSDLWindow, szWinTitle);
}

// call at program end
void os_cleanup () {
    os_deinit ();
}

// window management
// -----------------------------------------------------------------

// Returns 1 if app is not the foreground app.
int os_foreground () { return fAppActive; }

// process management
// -----------------------------------------------------------------

// Sleeps for n milliseconds or until app becomes active.
void os_sleep (uint ms) { SDL_Delay (ms); }

static bool file_exists (const std::string& path) {
    std::ofstream str (path, std::ios::in);
    return str.good ();
}

static time_t get_file_modification_time (const std::string& path) {
    struct stat file_stats {};
    if (stat (path.c_str (), &file_stats) < 0) { return 0; }
    return file_stats.st_mtime;
}

const char* Osapi_legacy_mode_reason = nullptr;

bool os_is_legacy_mode () {
    // Make this check a little faster by caching the result
    if (checkedLegacyMode) { return legacyMode; }

    if (Cmdline_portable_mode) {
        // When the portable mode option is given, non-legacy is implied
        legacyMode = false;
        checkedLegacyMode = true;

        Osapi_legacy_mode_reason =
            "Legacy mode disabled since portable mode was enabled.";
    }
    else {
        std::stringstream path_stream;
        path_stream << getPreferencesPath () << Osreg_config_file_name;

        auto new_config_exists = file_exists (path_stream.str ());
        time_t new_config_time = 0;
        if (new_config_exists) {
            new_config_time = get_file_modification_time (path_stream.str ());
        }

        // Also check the modification times of the command line files in case
        // the launcher did not change the settings file
        path_stream.str ("");
        path_stream << getPreferencesPath () << "data" << DIR_SEPARATOR_CHAR
                    << "cmdline_fso.cfg";
        new_config_time = std::max (
            new_config_time, get_file_modification_time (path_stream.str ()));
        path_stream.str ("");
        path_stream << os_get_legacy_user_dir () << DIR_SEPARATOR_CHAR
                    << Osreg_config_file_name;

        auto old_config_exists = file_exists (path_stream.str ());
        time_t old_config_time = 0;
        if (old_config_exists) {
            old_config_time = get_file_modification_time (path_stream.str ());
        }

        path_stream.str ("");
        path_stream << os_get_legacy_user_dir () << DIR_SEPARATOR_CHAR
                    << "data" << DIR_SEPARATOR_CHAR << "cmdline_fso.cfg";
        old_config_time = std::max (
            old_config_time, get_file_modification_time (path_stream.str ()));

        if (new_config_exists && old_config_exists) {
            // Both config files exists so we need to decide which to use based
            // on their last modification times if the old config was modified
            // more recently than the new config then we use the legacy mode
            // since the user probably used an outdated launcher after using a
            // more recent one
            legacyMode = old_config_time > new_config_time;

            if (legacyMode) {
                Osapi_legacy_mode_reason =
                    "Legacy mode enabled since the old config location was "
                    "used more recently than the new location.";
            }
            else {
                Osapi_legacy_mode_reason =
                    "Legacy mode disabled since the new config location was "
                    "used more recently than the old location.";
            }
        }
        else if (new_config_exists) {
            // If the new config exists and the old one doesn't then we can
            // safely disable the legacy mode
            legacyMode = false;

            Osapi_legacy_mode_reason =
                "Legacy mode disabled since the old config does not exist "
                "while the new config exists.";
        }
        else if (old_config_exists) {
            // Old config exists but new doesn't -> use legacy mode
            legacyMode = true;

            Osapi_legacy_mode_reason =
                "Legacy mode enabled since the old config exists while the "
                "new config does not exist.";
        }
        else {
            // Neither old nor new config exists -> this is a new install
            legacyMode = false;

            Osapi_legacy_mode_reason =
                "Legacy mode disabled since no existing config was detected.";
        }
    }

    if (legacyMode) {
        // Print a message for the people running it from the terminal
        fprintf (
            stdout,
            "FSO is running in legacy config mode. Please either update your "
            "launcher or"
            " copy the configuration and pilot files to '%s' for better "
            "future compatibility.\n",
            getPreferencesPath ());
    }

    checkedLegacyMode = true;
    return legacyMode;
}

// ----------------------------------------------------------------------------------------------------
// OSAPI FORWARD DECLARATIONS
//

// called at shutdown. Makes sure all thread processing terminates.
void os_deinit () {
    // Free the view ports
    os::closeAllViewports ();

    if (preferencesPath) {
        SDL_free (preferencesPath);
        preferencesPath = nullptr;
    }

    SDL_Quit ();
}

void debug_int3 (const char* file, int line) {
    WARNINGF (LOCATION, "Int3(): From %s at line %d\n", file, line);

    gr_activate (0);

#ifndef NDEBUG
    SDL_TriggerBreakpoint ();
#endif

    gr_activate (1);
}

namespace os {
Viewport* addViewport (std::unique_ptr< Viewport >&& viewport) {
    auto port = viewport.get ();
    viewports.push_back (std::move (viewport));
    return port;
}
void setMainViewPort (Viewport* mainView) {
    mainViewPort = mainView;
    mainSDLWindow = mainView->toSDLWindow ();
}
SDL_Window* getSDLMainWindow () { return mainSDLWindow; }
Viewport* getMainViewport () { return mainViewPort; }
void closeAllViewports () { viewports.clear (); }

namespace events {
namespace {
ListenerIdentifier nextListenerIdentifier;

struct EventListenerData {
    ListenerIdentifier identifier;
    Listener listener;

    uint32_t type;
    int weight;

    bool operator< (const EventListenerData& other) const {
        if (type < other.type) { return true; }
        if (type > other.type) { return false; }

        // Type is the same
        return weight < other.weight;
    }
};

bool compare_type (
    const EventListenerData& left, const EventListenerData& right) {
    return left.type < right.type;
}

std::vector< EventListenerData > eventListeners;
} // namespace

ListenerIdentifier
addEventListener (SDL_EventType type, int weight, const Listener& listener) {
    ASSERTX (listener, "Invalid event handler passed!");

    EventListenerData data;
    data.identifier = ++nextListenerIdentifier;
    data.listener = listener;

    data.weight = weight;
    data.type = static_cast< uint32_t > (type);

    eventListeners.push_back (data);
    // This is suboptimal for runtime but we will iterate that vector often so
    // cache hits are more important
    std::sort (eventListeners.begin (), eventListeners.end ());

    return data.identifier;
}

bool removeEventListener (ListenerIdentifier identifier) {
    auto endIter = end (eventListeners);
    for (auto iter = begin (eventListeners); iter != endIter; ++iter) {
        if (iter->identifier == identifier) {
            eventListeners.erase (iter);
            return true; // Identifiers are unique
        }
    }

    return false;
}

bool isWindowEvent (const SDL_Event& e, SDL_Window* window) {
    auto mainId = SDL_GetWindowID (window);
    switch (e.type) {
    case SDL_WINDOWEVENT: return mainId == e.window.windowID;
    case SDL_KEYDOWN:
    case SDL_KEYUP: return mainId == e.key.windowID;
    case SDL_TEXTEDITING: return mainId == e.edit.windowID;
    case SDL_TEXTINPUT: return mainId == e.text.windowID;
    case SDL_MOUSEMOTION: return mainId == e.motion.windowID;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP: return mainId == e.button.windowID;
    case SDL_MOUSEWHEEL: return mainId == e.wheel.windowID;
    default:
        // Event doesn't have a window ID
        return true;
    }
}
} // namespace events
} // namespace os

void os_ignore_events () {
    SDL_Event event;
    while (SDL_PollEvent (&event)) {
        // Add event to buffer
        buffered_events.push_back (event);
    }
}

static void handle_sdl_event (const SDL_Event& event) {
    using namespace os::events;

    EventListenerData data;
    data.type = event.type;

    auto iter = std::lower_bound (
        eventListeners.begin (), eventListeners.end (), data, compare_type);

    if (iter != eventListeners.end ()) {
        // The vector contains all event listeners, the listeners are sorted
        // for type and weight
        // -> iterating through all listeners will yield them in increasing
        // weight order but we can only do this until we have reached the end
        // of the vector or the type has changed
        for (; iter != eventListeners.end () && iter->type == event.type;
             ++iter) {
            if (iter->listener (event)) {
                // Listener has handled the event
                break;
            }
        }
    }
}

void os_poll () {
    // Replay the buffered events
    auto end = buffered_events.end ();
    for (auto it = buffered_events.begin (); it != end; ++it) {
        handle_sdl_event (*it);
    }
    buffered_events.clear ();

    SDL_Event event;

    while (SDL_PollEvent (&event)) { handle_sdl_event (event); }
}

std::string os_get_config_path (const std::string& subpath) {
    // Make path platform compatible
    std::string compatiblePath (subpath);
    std::replace (
        compatiblePath.begin (), compatiblePath.end (), '/',
        DIR_SEPARATOR_CHAR);

    std::stringstream ss;

    if (Cmdline_portable_mode) {
        // Use the current directory
        ss << "." << DIR_SEPARATOR_CHAR << compatiblePath;
        return ss.str ();
    }

    // Avoid infinite recursion when checking legacy mode
    if (os_is_legacy_mode ()) {
        extern const char* Osreg_user_dir_legacy;

        // Use the home directory
        ss << getenv ("HOME") << DIR_SEPARATOR_CHAR << Osreg_user_dir_legacy;
        ss << DIR_SEPARATOR_CHAR << compatiblePath;

        return ss.str ();
    }

    ss << getPreferencesPath () << compatiblePath;

    return ss.str ();
}
