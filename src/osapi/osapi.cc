// -*- mode: c++; -*-

#include <fcntl.h>
#include <sys/stat.h>

#include <mutex>
#include <filesystem>
namespace fs = std::filesystem;

#include "defs.hh"
#include "shared/types.hh"
#include "gamesequence/gamesequence.hh"
#include "freespace2/freespace.hh"
#include "parse/parselo.hh"
#include "assert/assert.hh"
#include "log/log.hh"

namespace {

bool fAppActive = false;

std::vector< std::unique_ptr< os::Viewport > > viewports;
os::Viewport* mainViewPort = 0;
SDL_Window* mainSDLWindow = 0;

inline fs::path
get_config_path () {
    static fs::path config_path;

    static std::once_flag init_flag;
    std::call_once (init_flag, [&]() {
        config_path = fs::path (SDL_GetPrefPath ("Volition", "Freespace2")); });

    ASSERT (!config_path.empty ());

    return config_path;
}

bool
window_event_handler (const SDL_Event& e) {
    ASSERT (mainSDLWindow);

    if (os::events::isWindowEvent (e, mainSDLWindow)) {
        switch (e.window.event) {
        case SDL_WINDOWEVENT_MINIMIZED:
        case SDL_WINDOWEVENT_FOCUS_LOST: {
            if (fAppActive) {
                game_pause ();
                fAppActive = false;
            }
            break;
        }
        case SDL_WINDOWEVENT_MAXIMIZED:
        case SDL_WINDOWEVENT_RESTORED:
        case SDL_WINDOWEVENT_FOCUS_GAINED: {
            if (!fAppActive) {
                game_unpause ();
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

bool
quit_handler (const SDL_Event&) {
    gameseq_post_event (GS_EVENT_QUIT_GAME);
    return true;
}

} // namespace

////////////////////////////////////////////////////////////////////////

static std::vector< SDL_Event > buffered_events;

void
do_os_init (const char*, const char* title, const char* name) {
    if (0 == name || 0 == name [0]) {
        os_init_registry_stuff (Osreg_company_name, title);
    }
    else {
        os_init_registry_stuff (Osreg_company_name, name);
    }

    SDL_version compiled, linked;

    SDL_VERSION (&compiled);
    SDL_GetVersion (&linked);

    WARNINGF (LOCATION, "SDL v%d.%d.%d (compiled %d.%d.%d) ...", linked.major, linked.minor, linked.patch, compiled.major,compiled.minor, compiled.patch);

    if (SDL_Init (SDL_INIT_EVENTS) < 0) {
        EE << "SDL initialization failed : " << SDL_GetError ();
        exit (1);
    }

    os::events::addEventListener (
        SDL_WINDOWEVENT, os::events::DEFAULT_LISTENER_WEIGHT,
        window_event_handler);

    os::events::addEventListener (
        SDL_QUIT, os::events::DEFAULT_LISTENER_WEIGHT, quit_handler);
}

void os_init (const char* cls, const char* title, const char* name) {
    static std::once_flag init;
    std::call_once (init, [&]() { do_os_init (cls, title, name); });
}

void os_set_title (const char* s) {
    ASSERT (mainSDLWindow);
    SDL_SetWindowTitle (mainSDLWindow, s);
}

void os_fini () {
    os::closeAllViewports ();
    SDL_Quit ();
}

int os_foreground () { return fAppActive; }

void debug_int3 (const char* file, int line) {
    II << file << ":" << line << " : int3";

    gr_activate (0);

#ifndef NDEBUG
    SDL_TriggerBreakpoint ();
#endif

    gr_activate (1);
}

////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////

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

}}

////////////////////////////////////////////////////////////////////////

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
    for (const auto& event : buffered_events)
        handle_sdl_event (event);

    buffered_events.clear ();

    for (SDL_Event event; SDL_PollEvent (&event);)
        handle_sdl_event (event);
}

std::string os_get_config_path (const std::string& suffix) {
    return std::string (fs::path (get_config_path ()) += suffix);
}
