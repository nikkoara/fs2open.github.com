// -*- mode: c++; -*-

#include "osapi/dialogs.h"
#include "osapi/osapi.h"
#include "parse/parselo.h"
#include "cmdline/cmdline.h"
#include "graphics/2d.h"

#include <SDL_messagebox.h>
#include <SDL_clipboard.h>

#include <string>
#include <algorithm>
#include <filesystem>
namespace fs = std::filesystem;

namespace {

const size_t max_msgbox_lines = 30;

std::stringstream
clip (std::stringstream& ss) {
    size_t i = 0;

    std::string line;
    std::stringstream out;
    
    for (; i < max_msgbox_lines && std::getline (ss, line); ++i) {
        out << line << "\n";
    }

    if (i >= max_msgbox_lines)
        out << "[...]";

    return out;
}

inline void
clipboard_copy (const std::string& s) {
    if (0 == SDL_InitSubSystem (SDL_INIT_VIDEO))
        SDL_SetClipboardText (s.c_str ());
}

} // namespace anonymous

////////////////////////////////////////////////////////////////////////

int Global_warning_count = 0;
int Global_error_count = 0;

namespace fs2 {
namespace dialog {

void
assert_msg (
    const char* s, const char* filepath, int linenum,
    const char* fmt, ...) {
    
    auto filename = fs::path (filepath).filename ();

    std::stringstream ss;
    
    ss << "Assert: \"" << s << "\"\nFile: " << filename << "\nLine: "
       << linenum << "\n";

    if (fmt) {
        std::string buf;
        va_list args;

        va_start (args, fmt);
        vsprintf (buf, fmt, args);
        va_end (args);

        ss << buf << "\n";
    }

    ss << "\n";
    ss << dump_stacktrace ();

    mprintf (
        ("ASSERTION: \"%s\" at %s:%d\n %s\n", s, filename.c_str (), linenum,
         ss.str ().c_str ()));    

    clipboard_copy (ss.str ());

    ss = clip (ss);
    ss  << "\n(copied to clipboard)\n\n\nDebug breaks into Debugger, "
        "Exit closes the program.\n";

    error (ss.str ().c_str ());
}

static void
do_error_log (const char* s) {
    EE ("general") << s;
}

static void
do_error_box (const char* s) {
    std::stringstream ss;
    ss << s << "\n" << dump_stacktrace ();

    clipboard_copy (ss.str ());

    ss = clip (ss);    
    ss  << "\n(copied to clipboard)\n\n\nDebug breaks into Debugger, "
        "Exit closes the program.\n";

    static const SDL_MessageBoxButtonData buttons [] = {
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Exit" },
        { /* .flags, .buttonid, .text */ 0, 0, "Debug" },
    };

    SDL_MessageBoxData boxData;
    memset (&boxData, 0, sizeof (boxData));

    boxData.buttons = buttons;
    boxData.numbuttons = 2;
    boxData.colorScheme = 0;
    boxData.flags = SDL_MESSAGEBOX_ERROR;
    boxData.message = ss.str ().c_str ();
    boxData.title = "Error!";
    boxData.window = os::getSDLMainWindow ();

    gr_activate (0);

    int buttonId;
    
    if (SDL_ShowMessageBox (&boxData, &buttonId) < 0)
        exit (1);

    switch (buttonId) {
    case 1:
        exit (1);

    default:
        Int3 (); break;
    }
    
    gr_activate (1);
}

void
error (const char* s) {
    do_error_log (s);

    if (Cmdline_noninteractive)
        std::terminate ();

    do_error_box (s);
}

void
error (const char* filename, int line, const char* fmt, ...) {
    std::string s;
    
    va_list args;
    va_start (args, fmt);
    vsprintf (s, fmt, args);
    va_end (args);

    std::stringstream ss;
    
    ss << "Error: " << s << "\nFile: "
       << fs::path (filename).filename () << "\nLine: "
       << line << "\n";

    error (ss.str ().c_str ());
}

static void
do_warning_log (const fs::path& filename, int line, const std::string& text) {
    //
    // Output to debug log
    //
    std::string s = text;

    std::transform (s.begin (), s.end (), s.begin (), [](auto c) {
        return c == '\n' ? ' ' : c; });

    WW ("general") << " file: " << filename << " ,line: " << line << " :" << s;
}

static void
do_warning_box (const fs::path& filename, int line, const std::string& text) {
    std::stringstream ss;
    
    ss << "Warning: " << text << "\n" << "File: " << filename << "\n"
       << "Line: " << line << "\n";

    clipboard_copy (ss.str ());

    ss = clip (ss);
    ss  << "\n(copied to clipboard)\n\n\nDebug breaks into Debugger\n";

    static const SDL_MessageBoxButtonData buttons[] = {
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 2, "Exit" },
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "Continue" },
        { /* .flags, .buttonid, .text */ 0, 0, "Debug" },
    };

    SDL_MessageBoxData boxData;
    memset (&boxData, 0, sizeof (boxData));

    boxData.buttons = buttons;
    boxData.numbuttons = 3;
    boxData.colorScheme = 0;
    boxData.flags = SDL_MESSAGEBOX_WARNING;
    boxData.message = ss.str ().c_str ();
    boxData.title = "Warning!";
    boxData.window = os::getSDLMainWindow ();

    gr_activate (0);

    int buttonId;
    
    if (0 > SDL_ShowMessageBox (&boxData, &buttonId)) {
        exit (1);
    }

    switch (buttonId) {
    case 2:
        exit (1);

    case 0:
        Int3 (); break;

    default:
        break;
    }

    gr_activate (1);
}

static void
do_warning (const char* filepath, int line, const std::string& text) {
    auto filename = fs::path (filepath).filename ();

    do_warning_log (filename, line, text);

    if (Cmdline_noninteractive)
        return;

    do_warning_box (filename, line, text);
}

void
release_warning (const char* file, int line, const char* fmt, ...) {
    ++Global_warning_count;

    std::string s;
    va_list args;

    va_start (args, fmt);
    vsprintf (s, fmt, args);
    va_end (args);

    do_warning (file, line, s);
}

void
warning (const char* file, int line, const char* fmt, ...) {
    ++Global_warning_count;

#ifndef NDEBUG
    std::string s;
    va_list args;

    va_start (args, fmt);
    vsprintf (s, fmt, args);
    va_end (args);

    do_warning (file, line, s);
#endif
}

void
warning_ex (const char* file, int line, const char* fmt, ...) {
#ifndef NDEBUG
    if (Cmdline_extra_warn) {
        std::string s;
        va_list args;

        va_start (args, fmt);
        vsprintf (s, fmt, args);
        va_end (args);

        warning (file, line, "%s", s.c_str ());
    }
#endif
}

void
message (dialog_type type, const char* s, const char* t) {
    int flags = 1;

    switch (type) {
    case dialog_type::error:
        flags = SDL_MESSAGEBOX_ERROR;
        if (0 == t) t = "Error";
        break;
        
    case dialog_type::info:
        flags = SDL_MESSAGEBOX_INFORMATION;
        if (0 == t) t = "Information";
        break;
        
    case dialog_type::warning:
        flags = SDL_MESSAGEBOX_WARNING;
        if (0 == t) t = "Warning";
        break;
        
    default:
        Int3 ();
        t = "";
        break;
    }

    SDL_ShowSimpleMessageBox (flags, t, s, os::getSDLMainWindow ());
}

} // namespace dialogs
} // namespace os
