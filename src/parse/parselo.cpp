// -*- mode: c++; -*-

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdarg>
#include <csetjmp>

#include <cctype>
#include "globalincs/version.h"
#include "localization/fhash.h"
#include "localization/localize.h"
#include "mission/missionparse.h"
#include "parse/encrypt.h"
#include "parse/parselo.h"
#include "parse/sexp.h"
#include "ship/ship.h"
#include "weapon/weapon.h"
#include "mod_table/mod_table.h"

#include "util/encoding.h"
#include "util/unicode.h"

#define ERROR_LENGTH 64
#define RS_MAX_TRIES 5
#define SHARP_S (char)-33

// to know that a modular table is currently being parsed
bool Parsing_modular_table = false;

char Current_filename[MAX_PATH_LEN];
char Current_filename_save[MAX_PATH_LEN];
char Current_filename_sub[MAX_PATH_LEN]; // Last attempted file to load, don't
                                         // know if ex or not.
char Error_str[ERROR_LENGTH];
int Warning_count, Error_count;
int Warning_count_save = 0, Error_count_save = 0;
int fred_parse_flag = 0;
int Token_found_flag;

char* Parse_text = nullptr;
char* Parse_text_raw = nullptr;
char *Mp = NULL, *Mp_save = NULL;
const char* token_found;

static int Parsing_paused = 0;

// text allocation stuff
void allocate_parse_text (size_t size);
static size_t Parse_text_size = 0;

// Return true if this character is white space, else false.
int is_white_space (char ch) {
    return ((ch == ' ') || (ch == '\t') || (ch == EOLN));
}
int is_white_space (unicode::codepoint_t cp) {
    return (
        (cp == U' ') || (cp == U'\t') || (cp == (unicode::codepoint_t)EOLN));
}

// Returns true if this character is gray space, else false (gray space is
// white space except for EOLN).
int is_gray_space (char ch) { return ((ch == ' ') || (ch == '\t')); }

bool is_gray_space (unicode::codepoint_t cp) {
    return cp == U' ' || cp == U'\t';
}

int is_parenthesis (char ch) { return ((ch == '(') || (ch == ')')); }

// Advance global Mp (mission pointer) past all current white space.
// Leaves Mp pointing at first non white space character.
void ignore_white_space () {
    while ((*Mp != '\0') && is_white_space (*Mp)) Mp++;
}

void ignore_gray_space () {
    while ((*Mp != '\0') && is_gray_space (*Mp)) Mp++;
}

// Truncate *str, eliminating all trailing white space.
// Eg: "abc   "   becomes "abc"
// "abc abc " becomes "abc abc"
// "abc \t"   becomes "abc"
void drop_trailing_white_space (char* str) {
    auto len = strlen (str);
    if (len == 0) {
        // Nothing to do here
        return;
    }
    auto i = len - 1;
    while (i != INVALID_SIZE && is_white_space (str[i])) { --i; }
    str[i + 1] = '\0';
}

// Ditto for std::string
void drop_trailing_white_space (std::string& str) {
    if (str.empty ()) {
        // Nothing to do here
        return;
    }
    auto i = str.size () - 1;
    while (i != INVALID_SIZE && is_white_space (str[i])) { --i; }
    str.resize (i + 1);
}

// Eliminate any leading whitespace in str
void drop_leading_white_space (char* str) {
    auto len = strlen (str);
    size_t first = 0;

    // find first non-whitespace
    while ((first < len) && is_white_space (str[first])) first++;

    // quick out
    if (first == 0) return;

    memmove (str, str + first, len - first);
    str[len - first] = 0;
}

// Ditto for std::string
void drop_leading_white_space (std::string& str) {
    auto len = str.length ();
    size_t first = 0;

    // find first non-whitespace
    while ((first < len) && is_white_space (str[first])) first++;

    // quick out
    if (first == 0) return;

    // Assign the found substring to the string
    str = str.substr (first, len - first);
}

// eliminates all leading and trailing white space from a string.  Returns
// pointer passed in.
char* drop_white_space (char* str) {
    drop_trailing_white_space (str);
    drop_leading_white_space (str);

    return str;
}

// ditto for std::string
void drop_white_space (std::string& str) {
    drop_trailing_white_space (str);
    drop_leading_white_space (str);
}

// Advances Mp past current token.
void skip_token () {
    ignore_white_space ();

    while ((*Mp != '\0') && !is_white_space (*Mp)) Mp++;
}

// Display a diagnostic message if Verbose is set.
// (Verbose is set if -v command line switch is present.)
void diag_printf (const char* format, ...) {
#ifndef NDEBUG
    std::string buffer;
    va_list args;

    va_start (args, format);
    vsprintf (buffer, format, args);
    va_end (args);

    WARNINGF (LOCATION, "%s", buffer.c_str ());
#endif
}

// Grab and return (a pointer to) a bunch of tokens, terminating at
// ERROR_LENGTH chars, or end of line.
char* next_tokens () {
    int count = 0;
    char* pstr = Mp;
    char ch;

    while (((ch = *pstr++) != EOLN) && (ch != '\0') &&
           (count < ERROR_LENGTH - 1))
        Error_str[count++] = ch;

    Error_str[count] = 0;
    return Error_str;
}

// Return the line number given by the current mission pointer, ie Mp.
// A very slow function (scans all processed text), but who cares how long
// an error reporting function takes?
int get_line_num () {
    int count = 1;
    int incomment = 0;
    int multiline = 0;
    char* stoploc;
    char* p;

    p = Parse_text;
    stoploc = Mp;

    while (p < stoploc) {
        if (*p == '\0') ASSERT (0);

        if (!incomment && (*p == COMMENT_CHAR)) incomment = 1;

        if (!incomment && (*p == '/') && (*(p + 1) == '*')) {
            multiline = 1;
            incomment = 1;
        }

        if (incomment) stoploc++;

        if (multiline && (*(p - 1) == '*') && (*p == '/')) {
            multiline = 0;
            incomment = 0;
        }

        if (*p++ == EOLN) {
            if (!multiline && incomment) incomment = 0;
            count++;
        }
    }

    return count;
}

// Call this function to display an error message.
// error_level == 0 means this is just a warning.
// !0 means it's an error message.
// Prints line number and other useful information.
extern int Cmdline_noparseerrors;
void error_display (int error_level, const char* format, ...) {
    char type[8];
    std::string error_text;
    va_list args;

    if (error_level == 0) {
        strcpy_s (type, "Warning");
        Warning_count++;
    }
    else {
        strcpy_s (type, "Error");
        Error_count++;
    }

    va_start (args, format);
    vsprintf (error_text, format, args);
    va_end (args);

    WARNINGF (
        LOCATION,
        "%s(line %i): %s: %s\n", Current_filename, get_line_num (), type,
        error_text.c_str ());

    if (error_level == 0 || Cmdline_noparseerrors)
        WARNINGF (
            LOCATION, "%s(line %i):\n%s: %s", Current_filename,
            get_line_num (), type, error_text.c_str ());
    else
        ASSERTX (0, "%s(line %i):\n%s: %s", Current_filename,get_line_num (), type, error_text.c_str ());
}

// Advance Mp to the next eoln character.
void advance_to_eoln (const char* more_terminators) {
    char terminators[128];

    ASSERT ((more_terminators == NULL) || (strlen (more_terminators) < 125));

    terminators[0] = EOLN;
    terminators[1] = 0;
    if (more_terminators != NULL)
        strcat_s (terminators, more_terminators);
    else
        terminators[1] = 0;

    while (strchr (terminators, *Mp) == NULL) Mp++;
}

// Advance Mp to the next white space (ignoring white space inside of " marks)
void advance_to_next_white () {
    int in_quotes = 0;

    while ((*Mp != EOLN) && (*Mp != '\0')) {
        if (*Mp == '\"') in_quotes = !in_quotes;

        if (!in_quotes && is_white_space (*Mp)) break;

        if (!in_quotes && is_parenthesis (*Mp)) break;

        Mp++;
    }
}

// Search for specified string, skipping everything up to that point.  Returns
// 1 if found, 0 if string wasn't found (and hit end of file), or -1 if not
// found, but end of checking block was reached.
int skip_to_string (const char* pstr, const char* end) {
    ignore_white_space ();
    auto len = strlen (pstr);
    size_t len2 = 0;

    if (end) len2 = strlen (end);

    while ((*Mp != '\0') && strncasecmp (pstr, Mp, len) != 0) {
        if (end && *Mp == '#') return 0;

        if (end && !strncasecmp (end, Mp, len2)) return -1;

        advance_to_eoln (NULL);
        ignore_white_space ();
    }

    if (!Mp || *Mp == '\0') return 0;

    Mp += strlen (pstr);
    return 1;
}

// Goober5000
// Advance to start of pstr.  Return 0 is successful, otherwise return !0
int skip_to_start_of_string (const char* pstr, const char* end) {
    ignore_white_space ();
    auto len = strlen (pstr);
    size_t endlen;
    if (end)
        endlen = strlen (end);
    else
        endlen = 0;

    while ((*Mp != '\0') && strncasecmp (pstr, Mp, len) != 0) {
        if (end && *Mp == '#') return 0;

        if (end && !strncasecmp (end, Mp, endlen)) return 0;

        advance_to_eoln (NULL);
        ignore_white_space ();
    }

    if (!Mp || *Mp == '\0') return 0;

    return 1;
}

// Advance to start of either pstr1 or pstr2.  Return 0 is successful,
// otherwise return !0
int skip_to_start_of_string_either (
    const char* pstr1, const char* pstr2, const char* end) {
    size_t len1, len2, endlen;

    ignore_white_space ();
    len1 = strlen (pstr1);
    len2 = strlen (pstr2);
    if (end)
        endlen = strlen (end);
    else
        endlen = 0;

    while ((*Mp != '\0') && strncasecmp (pstr1, Mp, len1) != 0 &&
           strncasecmp (pstr2, Mp, len2) != 0) {
        if (end && *Mp == '#') return 0;

        if (end && !strncasecmp (end, Mp, endlen)) return 0;

        advance_to_eoln (NULL);
        ignore_white_space ();
    }

    if (!Mp || *Mp == '\0') return 0;

    return 1;
}

// Find a required string.
// If not found, display an error message, but try up to RS_MAX_TRIES times
// to find the string.  (This is the groundwork for ignoring non-understood
// lines.
// If unable to find the required string after RS_MAX_TRIES tries, then
// abort using longjmp to parse_abort.
int required_string (const char* pstr) {
    int count = 0;

    ignore_white_space ();

    while (strncasecmp (pstr, Mp, strlen (pstr)) != 0 &&
           (count < RS_MAX_TRIES)) {
        error_display (
            1, "Missing required token: [%s]. Found [%.32s] instead.\n", pstr,
            next_tokens ());
        advance_to_eoln (NULL);
        ignore_white_space ();
        count++;
    }

    if (count == RS_MAX_TRIES) {
        WARNINGF (
            LOCATION, "Error: Unable to find required token [%s]\n", pstr);
        WARNINGF (
            LOCATION, "Error: Unable to find required token [%s]\n", pstr);
        throw parse::ParseException ("Required string not found");
    }

    Mp += strlen (pstr);
    diag_printf ("Found required string [%s]", token_found = pstr);
    return 1;
}

int check_for_eof_raw () {
    if (*Mp == '\0') return 1;

    return 0;
}

int check_for_eof () {
    ignore_white_space ();

    return check_for_eof_raw ();
}

/**
Returns 1 if it finds a newline character precded by any amount of grayspace.
*/
int check_for_eoln () {
    ignore_gray_space ();

    if (*Mp == EOLN)
        return 1;
    else
        return 0;
}

// similar to optional_string, but just checks if next token is a match.
// It doesn't advance Mp except to skip past white space.
int check_for_string (const char* pstr) {
    ignore_white_space ();

    if (!strncasecmp (pstr, Mp, strlen (pstr))) return 1;

    return 0;
}

// like check for string, but doesn't skip past any whitespace
int check_for_string_raw (const char* pstr) {
    if (!strncasecmp (pstr, Mp, strlen (pstr))) return 1;

    return 0;
}

// Find an optional string.
// If found, return 1, else return 0.
// If found, point past string, else don't update pointer.
int optional_string (const char* pstr) {
    ignore_white_space ();

    if (!strncasecmp (pstr, Mp, strlen (pstr))) {
        Mp += strlen (pstr);
        return 1;
    }

    return 0;
}

int optional_string_either (const char* str1, const char* str2) {
    ignore_white_space ();

    if (!strncasecmp (str1, Mp, strlen (str1))) {
        Mp += strlen (str1);
        return 0;
    }
    else if (!strncasecmp (str2, Mp, strlen (str2))) {
        Mp += strlen (str2);
        return 1;
    }

    return -1;
}

// generic parallel to required_string_one_of
int optional_string_one_of (int arg_count, ...) {
    ASSERTX (
        arg_count > 0,
        "optional_string_one_of() called with arg_count of %d; get a coder!\n",
        arg_count);
    int idx, found = -1;
    char* pstr;
    va_list vl;

    ignore_white_space ();

    va_start (vl, arg_count);
    for (idx = 0; idx < arg_count; idx++) {
        pstr = va_arg (vl, char*);

        if (!strncasecmp (pstr, Mp, strlen (pstr))) {
            Mp += strlen (pstr);
            found = idx;
            break;
        }
    }
    va_end (vl);

    return found;
}

int required_string_fred (const char* pstr, const char* end) {
    char* backup = Mp;

    token_found = pstr;
    if (fred_parse_flag) return 0;

    ignore_white_space ();
    while (*Mp != '\0' && strncasecmp (pstr, Mp, strlen (pstr)) != 0) {
        if ((*Mp == '#') || (end && !strncasecmp (end, Mp, strlen (end)))) {
            Mp = NULL;
            break;
        }

        advance_to_eoln (NULL);
        ignore_white_space ();
    }

    if (!Mp || *Mp == '\0') {
        diag_printf ("Required string [%s] not found", pstr);
        Mp = backup;
        Token_found_flag = 0;
        return 0;
    }

    Mp += strlen (pstr);
    diag_printf ("Found required string [%s]", pstr);
    Token_found_flag = 1;
    return 1;
}

// attempt to find token in buffer.  It might not exist, however, in which case
// we don't need to do anything.  If it is found, then we advance the pointer
// to just after the token.  To further complicate things, we should only
// search to a certain point, since we don't want a token that belongs to
// another section which might match the token we want.  Thus, we also pass in
// an ending token, which marks the point we should stop looking at.
int optional_string_fred (
    const char* pstr, const char* end, const char* end2) {
    char* mp_save = Mp;

    token_found = pstr;
    if (fred_parse_flag) return 0;

    ignore_white_space ();
    while ((*Mp != '\0') && strncasecmp (pstr, Mp, strlen (pstr)) != 0) {
        if ((*Mp == '#') || (end && !strncasecmp (end, Mp, strlen (end))) ||
            (end2 && !strncasecmp (end2, Mp, strlen (end2)))) {
            Mp = NULL;
            break;
        }

        advance_to_eoln (NULL);
        ignore_white_space ();
    }

    if (!Mp || *Mp == '\0') {
        diag_printf ("Optional string [%s] not found", pstr);
        Mp = mp_save;
        Token_found_flag = 0;
        return 0;
    }

    Mp += strlen (pstr);
    diag_printf ("Found optional string [%s]", pstr);
    Token_found_flag = 1;
    return 1;
}

/**
 * @brief Checks for one of two required strings
 *
 * @retval 0 for str1 match
 * @retval 1 for str2 match
 * @throws parse::ParseException If neither strings were found
 *
 * @details Advances the Mp until a string is found or exceeds RS_MAX_TRIES.
 * Once a string is found, Mp is located at the start of the found string.
 */
int required_string_either (const char* str1, const char* str2) {
    ignore_white_space ();

    for (int count = 0; count < RS_MAX_TRIES; ++count) {
        if (strncasecmp (str1, Mp, strlen (str1)) == 0) {
            // Mp += strlen(str1);
            diag_printf ("Found required string [%s]", token_found = str1);
            return 0;
        }
        else if (strncasecmp (str2, Mp, strlen (str2)) == 0) {
            // Mp += strlen(str2);
            diag_printf ("Found required string [%s]", token_found = str2);
            return 1;
        }

        error_display (
            1, "Required token = [%s] or [%s], found [%.32s].\n", str1, str2,
            next_tokens ());

        advance_to_eoln (NULL);
        ignore_white_space ();
    }

    WARNINGF (
        LOCATION, "Error: Unable to find either required token [%s] or [%s]\n",
        str1, str2);
    WARNINGF (
        LOCATION, "Error: Unable to find either required token [%s] or [%s]\n",
        str1, str2);
    throw parse::ParseException ("Required string not found");
}

/**
 * @brief Checks for one of any of the given required strings.
 *
 * @returns The index number of the found string, if it was found
 * @returns -1 if a string was not found
 *
 * @details By ngld, with some tweaks by MageKing17.
 */
int required_string_one_of (int arg_count, ...) {
    ASSERTX (
        arg_count > 0,
        "required_string_one_of() called with arg_count of %d; get a coder!\n",
        arg_count);
    int count = 0;
    int idx;
    char* expected;
    std::string message = "";
    va_list vl;

    ignore_white_space ();

    while (count < RS_MAX_TRIES) {
        va_start (vl, arg_count);
        for (idx = 0; idx < arg_count; idx++) {
            expected = va_arg (vl, char*);
            if (strncasecmp (expected, Mp, strlen (expected)) == 0) {
                diag_printf (
                    "Found required string [%s]", token_found = expected);
                va_end (vl);
                return idx;
            }
        }
        va_end (vl);

        if (message.empty ()) {
            va_start (vl, arg_count);
            message = "Required token = ";
            for (idx = 0; idx < arg_count; idx++) {
                message += "[";
                message += va_arg (vl, char*);
                message += "]";
                if (arg_count == 2 && idx == 0) { message += " or "; }
                else if (idx == arg_count - 2) {
                    message += ", or ";
                }
                else if (idx < arg_count - 2) {
                    message += ", ";
                }
            }
            va_end (vl);
        }

        error_display (
            1, "%s, found [%.32s]\n", message.c_str (), next_tokens ());
        advance_to_eoln (NULL);
        ignore_white_space ();
        count++;
    }

    return -1;
}

int required_string_either_fred (const char* str1, const char* str2) {
    ignore_white_space ();

    while (*Mp != '\0') {
        if (!strncasecmp (str1, Mp, strlen (str1))) {
            // Mp += strlen(str1);
            diag_printf ("Found required string [%s]", token_found = str1);
            return fred_parse_flag = 0;
        }
        else if (!strncasecmp (str2, Mp, strlen (str2))) {
            // Mp += strlen(str2);
            diag_printf ("Found required string [%s]", token_found = str2);
            return fred_parse_flag = 1;
        }

        advance_to_eoln (NULL);
        ignore_white_space ();
    }

    if (*Mp == '\0')
        diag_printf (
            "Unable to find either required token [%s] or [%s]\n", str1, str2);

    return -1;
}

// Copy characters from instr to outstr until eoln is found, or until max
// characters have been copied (including terminator).
void copy_to_eoln (
    char* outstr, const char* more_terminators, const char* instr, int max) {
    int count = 0;
    char ch;
    char terminators[128];

    ASSERT ((more_terminators == NULL) || (strlen (more_terminators) < 125));

    terminators[0] = EOLN;
    terminators[1] = 0;
    if (more_terminators != NULL)
        strcat_s (terminators, more_terminators);
    else
        terminators[1] = 0;

    while (((ch = *instr++) != 0) && (strchr (terminators, ch) == NULL) &&
           (count < max)) {
        *outstr++ = ch;
        count++;
    }

    if (count >= max)
        error_display (
            0, "Token too long: [%s].  Length = %zu.  Max is %i.\n",
            next_tokens (), strlen (next_tokens ()), max);

    *outstr = 0;
}

// Ditto for std::string.
void copy_to_eoln (
    std::string& outstr, const char* more_terminators, const char* instr) {
    char ch;
    char terminators[128];

    ASSERT ((more_terminators == NULL) || (strlen (more_terminators) < 125));

    terminators[0] = EOLN;
    terminators[1] = 0;
    if (more_terminators != NULL)
        strcat_s (terminators, more_terminators);
    else
        terminators[1] = 0;

    outstr = "";
    while (((ch = *instr++) != 0) && (strchr (terminators, ch) == NULL)) {
        outstr.append (1, ch);
    }
}

// Copy characters from instr to outstr until next white space is found, or
// until max    characters have been copied (including terminator).
void copy_to_next_white (char* outstr, char* instr, int max) {
    int count = 0;
    int in_quotes = 0;
    char ch;

    while (((ch = *instr++) > 0) && (ch != EOLN) && (ch != '\0') &&
           (count < max)) {
        if (ch == '\"') {
            in_quotes = !in_quotes;
            continue;
        }

        if (!in_quotes &&
            is_white_space (
                ch)) // not in quotes, white space terminates string
            break;

        if (!in_quotes &&
            is_parenthesis (ch)) // not in quotes, parentheses are important
                                 // for parsing so we don't want to copy them
            break;

        *outstr++ = ch;
        count++;
    }

    if (count >= max)
        error_display (
            0, "Token too long: [%s].  Length = %zu.  Max is %i.\n",
            next_tokens (), strlen (next_tokens ()), max);

    *outstr = 0;
}

// Ditto for std::string.
void copy_to_next_white (std::string& outstr, char* instr) {
    int in_quotes = 0;
    char ch;

    outstr = "";
    while (((ch = *instr++) > 0) && (ch != EOLN) && (ch != '\0')) {
        if (ch == '\"') {
            in_quotes = !in_quotes;
            continue;
        }

        if (!in_quotes &&
            is_white_space (
                ch)) // not in quotes, white space terminates string
            break;

        if (!in_quotes &&
            is_parenthesis (ch)) // not in quotes, parentheses are important
                                 // for parsing so we don't want to copy them
            break;

        outstr.append (1, ch);
    }
}

// Returns a null-terminated character string allocated with vm_malloc() with
// the data
char* alloc_text_until (char* instr, char* endstr) {
    ASSERT (instr && endstr);
    char* foundstr = stristr (instr, endstr);

    if (foundstr == NULL) {
        ASSERTX (0, "Missing [%s] in file", endstr);
        throw parse::ParseException ("End string not found");
    }
    else {
        if ((foundstr - instr) <= 0) {
            Int3 (); // since this really shouldn't ever happen
            return NULL;
        }

        char* rstr = NULL;
        rstr = (char*)vm_malloc ((foundstr - instr + 1) * sizeof (char));

        if (rstr != NULL) {
            strncpy (rstr, instr, foundstr - instr);
            rstr[foundstr - instr] = '\0';
        }
        else {
            ASSERTX (0, "Could not allocate enough memory in alloc_text_until");
        }

        return rstr;
    }
}

// Copy text until a certain string is matched.
// For example, this is used to copy mission notes, scanning until $END NOTES:
// is found.
void copy_text_until (
    char* outstr, char* instr, const char* endstr, int max_chars) {
    char* foundstr;
    ASSERT (outstr && instr && endstr);

    foundstr = stristr (instr, endstr);

    if (foundstr == NULL) {
        WARNINGF (
            LOCATION, "Error.  Looking for [%s], but never found it.\n",
            endstr);
        throw parse::ParseException ("End string not found");
    }

    if (foundstr - instr + strlen (endstr) < (uint)max_chars) {
        strncpy (outstr, instr, foundstr - instr);
        outstr[foundstr - instr] = 0;
    }
    else {
        WARNINGF (
            LOCATION,
            "Error.  Too much text (%zu chars, %i allowed) before %s\n",
            foundstr - instr - strlen (endstr), max_chars, endstr);

        throw parse::ParseException ("Too much text found");
    }
}

// Ditto for std::string.
void copy_text_until (std::string& outstr, char* instr, const char* endstr) {
    char* foundstr;
    ASSERT (instr && endstr);

    foundstr = stristr (instr, endstr);

    if (foundstr == NULL) {
        WARNINGF (
            LOCATION, "Error.  Looking for [%s], but never found it.\n",
            endstr);
        throw parse::ParseException ("End string not found");
    }

    outstr.assign (instr, foundstr - instr);
}

// stuffs a string into a buffer.  Can get a string between " marks and stops
// when whitespace is encounted -- not to end of line
void stuff_string_white (char* outstr, int len) {
    if (!len) len = NAME_LENGTH - 1;

    ignore_white_space ();
    copy_to_next_white (outstr, Mp, len);
    advance_to_next_white ();
}

// ditto for std::string
void stuff_string_white (std::string& outstr) {
    ignore_white_space ();
    copy_to_next_white (outstr, Mp);
    advance_to_next_white ();
}

// Goober5000
void stuff_string_until (char* outstr, const char* endstr, int len) {
    if (!len) len = NAME_LENGTH - 1;

    ignore_gray_space ();
    copy_text_until (outstr, Mp, endstr, len);
    Mp += strlen (outstr);
    drop_trailing_white_space (outstr);
}

// Goober5000
void stuff_string_until (std::string& outstr, const char* endstr) {
    ignore_gray_space ();
    copy_text_until (outstr, Mp, endstr);
    Mp += outstr.length ();
    drop_trailing_white_space (outstr);
}

// WMC
// Used for allocating large blocks, eg of Python code
// Returns a null-terminated string allocated with vm_malloc(),
// or NULL on failure
// Does depth checks for the start and end strings
// extra_chars indicates extra malloc space that should be allocated.
char* alloc_block (const char* startstr, const char* endstr, int extra_chars) {
    ASSERT (startstr != NULL && endstr != NULL);
    ASSERT (strcasecmp (startstr, endstr));

    char* rval = NULL;
    auto elen = strlen (endstr);
    auto slen = strlen (startstr);
    size_t flen = 0;

    // Skip the opening thing and any extra stuff
    required_string (startstr);
    ignore_white_space ();

    // Allocate it
    char* pos = Mp;

    // Depth checking
    int level = 1;
    while (*pos != '\0') {
        if (!strncasecmp (pos, startstr, slen)) { level++; }
        else if (!strncasecmp (pos, endstr, elen)) {
            level--;
        }

        if (level <= 0) { break; }

        pos++;
    }

    // Check that we left the file
    if (level > 0) {
        ASSERTX (0, "Unclosed pair of \"%s\" and \"%s\" on line %d in file",startstr, endstr, get_line_num ());
        throw parse::ParseException ("End string not found");
    }
    else {
        // Set final length for faster calcs
        flen = pos - Mp;

        // Allocate the memory
        // WMC - Don't forget the null character that's added later on.
        rval = (char*)vm_malloc ((flen + extra_chars + 1) * sizeof (char));

        // Copy the text (if memory was allocated)
        if (rval != NULL) {
            strncpy (rval, Mp, flen);
            rval[flen] = '\0';
        }
        else {
            return NULL;
        }
    }

    // Skip the copied stuff
    Mp += flen;
    required_string (endstr);
    return rval;
}

// Karajorma - Stuffs the provided char array with either the contents of a
// quoted string or the name of a string variable. Returns PARSING_FOUND_STRING
// if a string was found or PARSING_FOUND_VARIABLE if a variable was present.
int get_string_or_variable (char* str) {
    int result = -1;

    ignore_white_space ();

    // Variable
    if (*Mp == '@') {
        Mp++;
        stuff_string_white (str);
        int sexp_variable_index = get_index_sexp_variable_name (str);

        // We only want String variables
        ASSERTX (
            sexp_variable_index != -1, "Didn't find variable name \"%s\"",
            str);
        ASSERT (
            Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_STRING);

        result = PARSING_FOUND_VARIABLE;
    }
    // Quoted string
    else if (*Mp == '"') {
        get_string (str);
        result = PARSING_FOUND_STRING;
    }
    else {
        get_string (str);
        ASSERTX (0, "Invalid entry \"%s\"  found in get_string_or_variable. Must be a quoted string or a string variable name.",str);
    }

    return result;
}

// ditto for std::string
int get_string_or_variable (std::string& str) {
    int result = -1;

    ignore_white_space ();

    // Variable
    if (*Mp == '@') {
        Mp++;
        stuff_string_white (str);
        int sexp_variable_index = get_index_sexp_variable_name (str);

        // We only want String variables
        ASSERTX (
            sexp_variable_index != -1, "Didn't find variable name \"%s\"",
            str.c_str ());
        ASSERT (
            Sexp_variables[sexp_variable_index].type & SEXP_VARIABLE_STRING);

        result = PARSING_FOUND_VARIABLE;
    }
    // Quoted string
    else if (*Mp == '"') {
        get_string (str);
        result = PARSING_FOUND_STRING;
    }
    else {
        get_string (str);
        ASSERTX (0, "Invalid entry \"%s\"  found in get_string_or_variable. Must be a quoted string or a string variable name.",str.c_str ());
    }

    return result;
}

/**
 * Stuff a string (" chars ") into *str, return length.
 * Accepts an optional max length parameter. If it is omitted or negative, then
 * no max length is enforced.
 */
int get_string (char* str, int max) {
    auto len = strcspn (Mp + 1, "\"");

    if (max >= 0 && len >= (size_t)max)
        error_display (
            0, "String too long.  Length = %zu.  Max is %i.\n", len, max);

    strncpy (str, Mp + 1, len);
    str[len] = 0;

    Mp += len + 2;
    return (int)len;
}

/**
 * Stuff a string (" chars ") into str.
 */
void get_string (std::string& str) {
    auto len = strcspn (Mp + 1, "\"");
    str.assign (Mp + 1, len);

    Mp += len + 2;
}

// Stuff a string into a string buffer.
// Supports various FreeSpace primitive types.  If 'len' is supplied, it will
// override
// the default string length if using the F_NAME case.
void stuff_string (char* outstr, int type, int len, const char* terminators) {
    char read_str[PARSE_BUF_SIZE] = "";
    int read_len = PARSE_BUF_SIZE;
    int final_len = len - 1;
    int tag_id;

    // make sure we have enough room
    ASSERT (final_len > 0);

    // make sure it's zero'd out
    memset (outstr, 0, len);

    switch (type) {
    case F_RAW:
    case F_LNAME:
    case F_NAME:
    case F_DATE:
    case F_FILESPEC:
    case F_PATHNAME:
    case F_MESSAGE:
        ignore_gray_space ();
        copy_to_eoln (read_str, terminators, Mp, read_len);
        drop_trailing_white_space (read_str);
        advance_to_eoln (terminators);
        break;

    case F_NOTES:
        ignore_white_space ();
        copy_text_until (read_str, Mp, "$End Notes:", read_len);
        Mp += strlen (read_str);
        required_string ("$End Notes:");
        break;

        // F_MULTITEXTOLD keeping for backwards compatability with old missions
        // can be deleted once all missions are using new briefing format

    case F_MULTITEXTOLD:
        ignore_white_space ();
        copy_text_until (read_str, Mp, "$End Briefing Text:", read_len);
        Mp += strlen (read_str);
        required_string ("$End Briefing Text:");
        break;

    case F_MULTITEXT:
        ignore_white_space ();
        copy_text_until (read_str, Mp, "$end_multi_text", read_len);
        Mp += strlen (read_str);
        drop_trailing_white_space (read_str);
        required_string ("$end_multi_text");
        break;

    default:
        ASSERTX (0, "Unhandled string type %d in stuff_string!", type);
    }

    if (type == F_FILESPEC) {
        // Make sure that the passed string looks like a good filename
        if (strlen (read_str) == 0) {
            // Empty file name is probably not valid!
            error_display (
                0,
                "A file name was expected but no name was supplied! This is "
                "probably a mistake.");
        }
    }

    // now we want to do any final localization
    if (type != F_RAW && type != F_LNAME) {
        lcl_ext_localize (read_str, outstr, final_len, &tag_id);

        // if the hash localized text hash table is active and we have a valid
        // external string - hash it
        if (fhash_active () && (tag_id > -2)) {
            fhash_add_str (outstr, tag_id);
        }
    }
    else {
        if (strlen (read_str) > (uint)final_len)
            error_display (
                0,
                "Token too long: [%s].  Length = %zu"
                ".  Max is %i.\n",
                read_str, strlen (read_str), final_len);

        strncpy (outstr, read_str, final_len);
    }

    diag_printf ("Stuffed string = [%.30s]", outstr);
}

// Stuff a string into a string buffer.
// Supports various FreeSpace primitive types.
void stuff_string (std::string& outstr, int type, const char* terminators) {
    std::string read_str;
    int tag_id;

    // make sure it's zero'd out
    outstr = "";

    switch (type) {
    case F_RAW:
    case F_LNAME:
    case F_NAME:
    case F_DATE:
    case F_FILESPEC:
    case F_PATHNAME:
    case F_MESSAGE:
        ignore_gray_space ();
        copy_to_eoln (read_str, terminators, Mp);
        drop_trailing_white_space (read_str);
        advance_to_eoln (terminators);
        break;

    case F_NOTES:
        ignore_white_space ();
        copy_text_until (read_str, Mp, "$End Notes:");
        Mp += read_str.length ();
        required_string ("$End Notes:");
        break;

        // F_MULTITEXTOLD keeping for backwards compatability with old missions
        // can be deleted once all missions are using new briefing format

    case F_MULTITEXTOLD:
        ignore_white_space ();
        copy_text_until (read_str, Mp, "$End Briefing Text:");
        Mp += read_str.length ();
        required_string ("$End Briefing Text:");
        break;

    case F_MULTITEXT:
        ignore_white_space ();
        copy_text_until (read_str, Mp, "$end_multi_text");
        Mp += read_str.length ();
        drop_trailing_white_space (read_str);
        required_string ("$end_multi_text");
        break;

    default:
        ASSERTX (0, "Unhandled string type %d in stuff_string!", type);
    }

    if (type == F_FILESPEC) {
        // Make sure that the passed string looks like a good filename
        if (read_str.empty ()) {
            // Empty file name is not valid!
            error_display (
                1, "A file name was expected but no name was supplied!\n");
        }
    }

    // now we want to do any final localization
    if (type != F_RAW && type != F_LNAME) {
        lcl_ext_localize (read_str, outstr, &tag_id);

        // if the hash localized text hash table is active and we have a valid
        // external string - hash it
        if (fhash_active () && (tag_id > -2)) {
            fhash_add_str (outstr.c_str (), tag_id);
        }
    }
    else {
        outstr = read_str;
    }

    diag_printf ("Stuffed string = [%.30s]", outstr.c_str ());
}

// stuff a string, but only until the end of a line. don't ignore leading
// whitespace. close analog of fgets()/cfgets()
void stuff_string_line (char* outstr, int len) {
    char read_str[PARSE_BUF_SIZE] = "";
    int read_len = PARSE_BUF_SIZE;
    int final_len = len - 1;
    int tag_id;

    ASSERT (final_len > 0);

    // read in a line
    copy_to_eoln (read_str, "\n", Mp, read_len);
    drop_trailing_white_space (read_str);
    advance_to_eoln ("");
    Mp++;

    // now we want to do any final localization
    lcl_ext_localize (read_str, outstr, final_len, &tag_id);

    // if the hash localized text hash table is active and we have a valid
    // external string - hash it
    if (fhash_active () && (tag_id > -2)) { fhash_add_str (outstr, tag_id); }

    diag_printf ("Stuffed string = [%.30s]", outstr);
}

// ditto for std::string
void stuff_string_line (std::string& outstr) {
    std::string read_str;
    int tag_id;

    // read in a line
    copy_to_eoln (read_str, "\n", Mp);
    drop_trailing_white_space (read_str);
    advance_to_eoln ("");
    Mp++;

    // now we want to do any final localization
    lcl_ext_localize (read_str, outstr, &tag_id);

    // if the hash localized text hash table is active and we have a valid
    // external string - hash it
    if (fhash_active () && (tag_id > -2)) {
        fhash_add_str (outstr.c_str (), tag_id);
    }

    diag_printf ("Stuffed string = [%.30s]", outstr.c_str ());
}

// Exactly the same as stuff string only Malloc's the buffer.
// Supports various FreeSpace primitive types.  If 'len' is supplied, it will
// override
// the default string length if using the F_NAME case.
char* stuff_and_malloc_string (int type, char* terminators) {
    std::string tmp_result;

    stuff_string (tmp_result, type, terminators);
    drop_white_space (tmp_result);

    if (tmp_result.empty ()) return NULL;

    return vm_strdup (tmp_result.c_str ());
}

void stuff_malloc_string (char** dest, int type, char* terminators) {
    ASSERT (dest != NULL); // wtf?

    char* new_val = stuff_and_malloc_string (type, terminators);

    if (new_val != NULL) {
        if ((*dest) != NULL) { vm_free (*dest); }

        (*dest) = new_val;
    }
}

// After reading a multitext string, you can call this function to convert any
// newlines into spaces, so it's a one paragraph string (I.e. as in MS-Word).
//
void compact_multitext_string (char* str) {
    size_t i;
    size_t len = strlen (str);
    size_t num_cr = 0;

    for (i = 0; i < len; i++) {
        // skip CR
        // convert LF to space
        // copy characters backwards if any CRs previously encountered
        if (str[i] == '\r')
            num_cr++;
        else if (str[i] == '\n')
            str[i - num_cr] = ' ';
        else if (num_cr > 0)
            str[i - num_cr] = str[i];
    }

    if (num_cr > 0) str[len - num_cr] = 0;
}

// ditto for std::string
void compact_multitext_string (std::string& str) {
    size_t i;
    size_t len = str.length ();
    size_t num_cr = 0;

    for (i = 0; i < len; i++) {
        // skip CR
        // convert LF to space
        // copy characters backwards if any CRs previously encountered
        if (str[i] == '\r')
            num_cr++;
        else if (str[i] == '\n')
            str[i - num_cr] = ' ';
        else if (num_cr > 0)
            str[i - num_cr] = str[i];
    }

    if (num_cr > 0) str.resize (len - num_cr);
}

// Converts a character from Windows-1252 to CP437.
int maybe_convert_foreign_character (int ch) {
    // time to do some special foreign character conversion
    switch (ch) {
    case -57: ch = 128; break;

    case -4: ch = 129; break;

    case -23: ch = 130; break;

    case -30: ch = 131; break;

    case -28: ch = 132; break;

    case -32: ch = 133; break;

    case -27: ch = 134; break;

    case -25: ch = 135; break;

    case -22: ch = 136; break;

    case -21: ch = 137; break;

    case -24: ch = 138; break;

    case -17: ch = 139; break;

    case -18: ch = 140; break;

    case -20: ch = 141; break;

    case -60: ch = 142; break;

    case -59: ch = 143; break;

    case -55: ch = 144; break;

    case -26: ch = 145; break;

    case -58: ch = 146; break;

    case -12: ch = 147; break;

    case -10: ch = 148; break;

    case -14: ch = 149; break;

    case -5: ch = 150; break;

    case -7: ch = 151; break;

    case -1: ch = 152; break;

    case -42: ch = 153; break;

    case -36: ch = 154; break;

    case -94: ch = 155; break;

    case -93: ch = 156; break;

    case -91: ch = 157; break;

    case -125: ch = 159; break;

    case -31: ch = 160; break;

    case -19: ch = 161; break;

    case -13: ch = 162; break;

    case -6: ch = 163; break;

    case -15: ch = 164; break;

    case -47: ch = 165; break;

    case -86: ch = 166; break;

    case -70: ch = 167; break;

    case -65: ch = 168; break;

    case -84: ch = 170; break;

    case -67: ch = 171; break;

    case -68: ch = 172; break;

    case -95: ch = 173; break;

    case -85: ch = 174; break;

    case -69: ch = 175; break;

    case -33: ch = 225; break;

    case -75: ch = 230; break;

    case -79: ch = 241; break;

    case -9: ch = 246; break;

    case -80: ch = 248; break;

    case -73: ch = 250; break;

    case -78: ch = 253; break;

    case -96: ch = 255; break;
    }

    return ch;
}

// Goober5000
// Yarn - The capacity of out must be at least the value returned by
// get_converted_string_length(in) (plus one if add_null is true).
// Returns the number of characters written to out.
size_t
maybe_convert_foreign_characters (const char* in, char* out, bool add_null) {
    if (Fred_running) {
        size_t len = strlen (in);

        if (add_null) {
            strcpy (out, in);
            return len + 1;
        }
        else {
            strncpy (out, in, len);
            return len;
        }
    }
    else {
        auto inp = in;
        auto outp = out;

        while (*inp != '\0') {
            if (*inp == SHARP_S) {
                *outp++ = 's';
                *outp++ = 's';
            }
            else if (Lcl_pl) {
                *outp++ = *inp;
            }
            else {
                *outp++ = (char)maybe_convert_foreign_character (*inp);
            }
            inp++;
        }

        if (add_null) { *outp++ = '\0'; }

        return outp - out;
    }
}

// Goober5000
void maybe_convert_foreign_characters (std::string& text) {
    if (!Fred_running) {
        for (std::string::iterator ii = text.begin (); ii != text.end ();
             ++ii) {
            text.reserve (get_converted_string_length (text));

            if (*ii == SHARP_S) {
                text.replace (ii, ii + 1, "ss");
                ++ii;
            }
            else if (!Lcl_pl) {
                *ii = (char)maybe_convert_foreign_character (*ii);
            }
        }
    }
}

// Yarn - Returns what the length of the text will be after it's processed by
// maybe_convert_foreign_characters, not including the null terminator.
size_t get_converted_string_length (const char* text) {
    if (Fred_running) { return strlen (text); }
    else {
        size_t count = 0;
        auto s = strchr (text, SHARP_S);
        while (s != nullptr) {
            count++;
            s = strchr (s + 1, SHARP_S);
        }
        return strlen (text) + count;
    }
}

// Yarn - Returns what the length of the text will be after it's processed by
// maybe_convert_foreign_characters.
size_t get_converted_string_length (const std::string& text) {
    if (Fred_running) { return text.size (); }
    else {
        size_t count = 0;
        for (auto ii = text.begin (); ii != text.end (); ++ii) {
            if (*ii == SHARP_S) { count++; }
        }
        return text.size () + count;
    }
}

// Goober5000
bool get_number_before_separator (
    int& number, int& number_chars, const char* text, char separator) {
    char buf[8];
    const char* ch = text;
    int len = 0;

    while (true) {
        // didn't find separator
        if (*ch == '\0' || len == 8) return false;

        // found separator
        if (*ch == separator) break;

        // found nondigit
        if (!isdigit (*ch)) return false;

        // copying in progress
        buf[len] = *ch;
        len++;
        ch++;
    }

    // got an integer
    buf[len] = '\0';
    number = atoi (buf);
    number_chars = len;
    return true;
}

// Goober5000
bool get_number_before_separator (
    int& number, int& number_chars, const std::string& text,
    std::string::iterator text_pos, char separator) {
    char buf[8];
    std::string::iterator ch = text_pos;
    int len = 0;

    while (true) {
        // didn't find separator
        if (ch == text.end () || len == 8) return false;

        // found separator
        if (*ch == separator) break;

        // found nondigit
        if (!isdigit (*ch)) return false;

        // copying in progress
        buf[len] = *ch;
        len++;
        ++ch;
    }

    // got an integer
    buf[len] = '\0';
    number = atoi (buf);
    number_chars = len;
    return true;
}

bool matches_version_specific_tag (
    const char* line_start, bool& compatible_version, int& tag_len) {
    // special version-specific comment
    // formatted like e.g. ;;FSO 3.7.0;;
    // Should now support anything from ;;FSO 3;; to ;;FSO 3.7.3.20151106;;
    // -MageKing17
    if (strncasecmp (line_start, ";;FSO ", 6) != 0) return false;

    int major, minor, build, revis;
    int s_num =
        scan_fso_version_string (line_start, &major, &minor, &build, &revis);

    if (s_num == 0) return false;

    // hack for releases
    if (s_num == 4 && FS_VERSION_REVISION < 1000) { s_num = 3; }

    const char* ch = line_start + 6;
    while ((*ch) != ';') {
        ASSERTX (
            (*ch) != '\0',
            "String that was already guaranteed to end with semicolons did "
            "not end with semicolons; it's possible we have fallen into an "
            "alternate universe. Failing string: [%s]\n",
            line_start);
        ch++;
    }
    ch++;
    ASSERTX (
        (*ch) == ';',
        "String that was guaranteed to have double semicolons did not; it's "
        "possible we have fallen into an alternate universe. Failing string: "
        "[%s]\n",
        line_start);
    ch++;

    tag_len = (int)(ch - line_start);
    compatible_version = true;

    // check whether major, minor, and build line up with this version
    if (major > FS_VERSION_MAJOR) { compatible_version = false; }
    else if (major == FS_VERSION_MAJOR && s_num > 1) {
        if (minor > FS_VERSION_MINOR) { compatible_version = false; }
        else if (minor == FS_VERSION_MINOR && s_num > 2) {
            if (build > FS_VERSION_BUILD) { compatible_version = false; }
            else if (build == FS_VERSION_BUILD && s_num > 3) {
                if (revis > FS_VERSION_REVISION) {
                    compatible_version = false;
                }
            }
        }
    }

    // true for tag match
    return true;
}

// Strip comments from a line of input.
// Goober5000 - rewritten for the second time
void strip_comments (
    char* line, bool& in_quote, bool& in_multiline_comment_a,
    bool& in_multiline_comment_b) {
    char* writep = line;
    char* readp = line;

    // copy all characters from read to write, unless they're commented
    while (*readp != '\r' && *readp != '\n' && *readp != '\0') {
        // only check for comments if not quoting
        if (!in_quote) {
            bool compatible_version;
            int tag_len;

            // see what sort of comment characters we recognize
            if (!strncmp (readp, "/*", 2)) {
                // comment styles are mutually exclusive
                if (!in_multiline_comment_b) in_multiline_comment_a = true;
            }
            else if (!strncmp (readp, "!*", 2)) {
                // comment styles are mutually exclusive
                if (!in_multiline_comment_a) in_multiline_comment_b = true;
            }
            else if (!strncmp (readp, "*/", 2)) {
                if (in_multiline_comment_a) {
                    in_multiline_comment_a = false;
                    readp += 2;
                    continue;
                }
            }
            else if (!strncmp (readp, "*!", 2)) {
                if (in_multiline_comment_b) {
                    in_multiline_comment_b = false;
                    readp += 2;
                    continue;
                }
            }
            // special version-specific comment
            // formatted like e.g. ;;FSO 3.7.0;;
            else if (matches_version_specific_tag (
                         readp, compatible_version, tag_len)) {
                // comment passes, so advance pass the tag and keep reading
                if (compatible_version) {
                    readp += tag_len;
                    continue;
                }
                // comment does not pass, so ignore the line
                else {
                    break;
                }
            }
            // standard comment
            else if (*readp == ';') {
                break;
            }
        }

        // maybe toggle quoting
        if (*readp == '\"') in_quote = !in_quote;

        // if not inside a comment, copy the characters
        if (!in_multiline_comment_a && !in_multiline_comment_b) {
            if (writep != readp) *writep = *readp;

            writep++;
        }

        // read the next character
        readp++;
    }

    // if we moved any characters, or if we haven't reached the end of the
    // string, then mark end-of-line and terminate string
    if (writep != readp || *readp != '\0') {
        writep[0] = EOLN;
        writep[1] = '\0';
    }
}

int parse_get_line (
    char* lineout, int max_line_len, char* start, int max_size, char* cur) {
    char* t = lineout;
    int i, num_chars_read = 0;
    char c;

    for (i = 0; i < max_line_len - 1; i++) {
        do {
            if ((cur - start) >= max_size) {
                *lineout = 0;
                if (lineout > t) { return num_chars_read; }
                else {
                    return 0;
                }
            }
            c = *cur++;
            num_chars_read++;
        } while (c == 13);

        *lineout++ = c;
        if (c == '\n') break;
    }

    *lineout++ = 0;
    return num_chars_read;
}

// Read mission text, stripping comments.
// When a comment is found, it is removed.  If an entire line
// consisted of a comment, a blank line is left in the input file.
// Goober5000 - added ability to read somewhere other than Mission_text
void read_file_text (
    const char* filename, int mode, char* processed_text, char* raw_text) {
    // copy the filename
    if (!filename) throw parse::ParseException ("Invalid filename");

    strcpy_s (Current_filename_sub, filename);

    // if we are paused then processed_text and raw_text must not be NULL!!
    if (Parsing_paused && ((processed_text == NULL) || (raw_text == NULL))) {
        ASSERTX (0, "ERROR: Neither processed_text nor raw_text may be NULL when parsing is paused!!\n");
    }

    // read the raw text
    read_raw_file_text (filename, mode, raw_text);

    if (processed_text == NULL) processed_text = Parse_text;

    if (raw_text == NULL) raw_text = Parse_text_raw;

    // process it (strip comments)
    process_raw_file_text (processed_text, raw_text);
}

void stop_parse () {
    ASSERT (!Parsing_paused);

    if (Parse_text != nullptr) {
        vm_free (Parse_text);
        Parse_text = nullptr;
    }

    if (Parse_text_raw != nullptr) {
        vm_free (Parse_text_raw);
        Parse_text_raw = nullptr;
    }

    Parse_text_size = 0;
}

void allocate_parse_text (size_t size) {
    ASSERT (size > 0);

    // Make sure that there is space for the terminating null character
    size += 1;

    if (size <= Parse_text_size) {
        // Make sure that a new parsing session does not use uninitialized
        // data.
        memset (Parse_text, 0, sizeof (char) * Parse_text_size);
        memset (Parse_text_raw, 0, sizeof (char) * Parse_text_size);
        return;
    }

    static ubyte parse_atexit = 0;

    if (!parse_atexit) {
        atexit (stop_parse);
        parse_atexit = 1;
    }

    if (Parse_text != nullptr) {
        vm_free (Parse_text);
        Parse_text = nullptr;
    }

    if (Parse_text_raw != nullptr) {
        vm_free (Parse_text_raw);
        Parse_text_raw = nullptr;
    }

    Parse_text = (char*)vm_malloc (sizeof (char) * size, memory::quiet_alloc);
    Parse_text_raw =
        (char*)vm_malloc (sizeof (char) * size, memory::quiet_alloc);

    if ((Parse_text == nullptr) || (Parse_text_raw == nullptr)) {
        ASSERTX (0, "Unable to allocate enough memory for Mission_text!  Aborting...\n");
    }

    memset (Parse_text, 0, sizeof (char) * size);
    memset (Parse_text_raw, 0, sizeof (char) * size);

    Parse_text_size = size;
}

// Goober5000
void read_raw_file_text (const char* filename, int mode, char* raw_text) {
    CFILE* mf;
    int file_is_encrypted;

    ASSERT (filename);

    mf = cfopen (filename, "rb", CFILE_NORMAL, mode);
    if (mf == NULL) {
        WARNINGF (LOCATION, "Wokka!  Error opening file (%s)!", filename);
        throw parse::ParseException ("Failed to open file");
    }

    // read the entire file in
    int file_len = cfilelength (mf);

    if (!file_len) {
        WARNINGF (LOCATION, "Oh noes!!  File is empty! (%s)!", filename);
        throw parse::ParseException ("Failed to open file");
    }

    // For the possible Latin1 -> UTF-8 conversion we need to reallocate the
    // raw_text at some point and we can only do that if we have control over
    // the raw_text pointer which is only the case if it's null.
    auto can_reallocate = raw_text == nullptr;
    if (raw_text == nullptr) {
        // allocate, or reallocate, memory for Mission_text and
        // Mission_text_raw based on size we need now
        allocate_parse_text ((size_t) (file_len + 1));
        // NOTE: this always has to be done *after* the allocate_mission_text()
        // call!!
        raw_text = Parse_text_raw;
    }

    // read first 10 bytes to determine if file is encrypted
    cfread (raw_text, MIN (file_len, 10), 1, mf);
    file_is_encrypted = is_encrypted (raw_text);
    cfseek (mf, 0, CF_SEEK_SET);

    file_len = util::check_encoding_and_skip_bom (mf, filename);

    if (file_is_encrypted) {
        int unscrambled_len;
        char* scrambled_text;
        scrambled_text = (char*)vm_malloc (file_len + 1);
        ASSERT (scrambled_text);
        cfread (scrambled_text, file_len, 1, mf);
        // unscramble text
        unencrypt (scrambled_text, file_len, raw_text, &unscrambled_len);
        file_len = unscrambled_len;
        vm_free (scrambled_text);
    }
    else {
        cfread (raw_text, file_len, 1, mf);
    }

    // WMC - Slap a NULL character on here for the odd error where we forgot a
    // #End
    raw_text[file_len] = '\0';

    if (Unicode_text_mode) {
        // Validate the UTF-8 encoding
        auto invalid =
            unicode::detail::find_invalid (raw_text, raw_text + file_len);

        if (invalid != raw_text + file_len) {
            auto isLatin1 =
                util::guessLatin1Encoding (raw_text, (size_t)file_len);

            // We do the additional can_reallocate check here since we need
            // control over raw_text to reencode the file
            if (isLatin1 && can_reallocate) {
                // Latin1 is the encoding of retail data and for legacy reasons
                // we convert that to UTF-8. We still output a warning
                // though...
                WARNINGF (
                    LOCATION,
                    "Found Latin-1 encoded file %s. This file will be "
                    "automatically converted to UTF-8 but "
                    "it may cause parsing issues with retail FS2 files since "
                    "those contained invalid data.\n"
                    "To silence this warning you must convert the files to "
                    "UTF-8, e.g. by using a program like iconv.",
                    filename);

                // SDL2 has iconv functionality so we use that to convert from
                // Latin1 to UTF-8

                // We need the raw_text as the output buffer so we first need
                // to copy the current
                std::string input_str = raw_text;

                do {
                    auto in_str = input_str.c_str ();
                    auto in_size = input_str.size ();
                    auto out_str = Parse_text_raw;
                    auto out_size = Parse_text_size;

                    auto iconv = SDL_iconv_open ("UTF-8", "ISO-8859-1");
                    auto err = SDL_iconv (
                        iconv, &in_str, &in_size, &out_str, &out_size);
                    SDL_iconv_close (iconv);

                    if (err == 0) { break; }
                    else if (err == SDL_ICONV_E2BIG) {
                        // buffer is not big enough, try again with a bigger
                        // buffer. Use a rather conservative size increment
                        // since the additional size required is probably
                        // pretty small
                        allocate_parse_text (Parse_text_size + 300);
                    }
                    else {
                        WARNINGF (
                            LOCATION,
                            "File reencoding failed (error code %zu"
                            ")!\n"
                            "You will probably encounter encoding issues.",
                            err);

                        // Copy the original data back to the mission text
                        // pointer so that we don't loose any data here
                        strcpy (Parse_text_raw, input_str.c_str ());
                        break;
                    }
                } while (true);
            }
            else {
                WARNINGF (
                    LOCATION,
                    "Found invalid UTF-8 encoding in file %s at "
                    "position %zd"
                    "!\n"
                    "This may cause parsing errors and should be fixed!",
                    filename, invalid - raw_text);
            }
        }
    }

    cfclose (mf);
}

// Goober5000
void process_raw_file_text (char* processed_text, char* raw_text) {
    char* mp;
    char* mp_raw;
    char outbuf[PARSE_BUF_SIZE];
    bool in_quote = false;
    bool in_multiline_comment_a = false;
    bool in_multiline_comment_b = false;
    int raw_text_len = (int)strlen (raw_text);

    if (processed_text == NULL) processed_text = Parse_text;

    if (raw_text == NULL) raw_text = Parse_text_raw;

    ASSERT (processed_text != NULL);
    ASSERT (raw_text != NULL);

    mp = processed_text;
    mp_raw = raw_text;

    // strip comments from raw text, reading into file_text
    int num_chars_read = 0;
    while ((num_chars_read = parse_get_line (
                outbuf, PARSE_BUF_SIZE, raw_text, raw_text_len, mp_raw)) !=
           0) {
        mp_raw += num_chars_read;

        // stupid hacks to make retail data work with fixed parser, per Mantis
        // #3072
        if (!strcmp (
                outbuf,
                "1402, \"Sie haben IPX-Protokoll als Protokoll ausgew\xE4hlt, "
                "aber dieses Protokoll ist auf Ihrer Maschine nicht "
                "installiert.\".\"\n")) {
            outbuf[121] = ' ';
            outbuf[122] = ' ';
        }
        else if (!strcmp (
                     outbuf,
                     "1117, \"\\r\\n\"Aucun web browser trouva. Del\xE0 isn't "
                     "on emm\xE9nagea ou if \\r\\non est emm\xE9nagea, ca "
                     "isn't set pour soient la default "
                     "browser.\\r\\n\\r\\n\"\n")) {
            char* ch = &outbuf[11];
            do {
                *ch = *(ch + 1);
                ++ch;
            } while (*ch);
        }
        else if (!strcmp (outbuf, "1337, \"(fr)Loading\"\n")) {
            outbuf[3] = '6';
        }
        else if (!strcmp (
                     outbuf,
                     "3966, \"Es sieht so aus, als habe Staffel Kappa Zugriff "
                     "auf die GTVA-Zugangscodes f\xFCr das System gehabt. Das "
                     "ist ein ernstes Sicherheitsleck. Ihre IFF-Kennung "
                     "erschien als \"verb\xFCndet\", so da\xDF sie sich dem "
                     "Konvoi ungehindert n\xE4hern konnten. Zum Gl\xFC\x63k "
                     "flogen Sie und  Alpha 2 Geleitschutz und lie\xDF\x65n "
                     "den Schwindel auffliegen, bevor Kappa ihren Befehl "
                     "ausf\xFChren konnte.\"\n")) {
            outbuf[171] = '\'';
            outbuf[181] = '\'';
        }

        strip_comments (
            outbuf, in_quote, in_multiline_comment_a, in_multiline_comment_b);

        if (Unicode_text_mode) {
            // In unicode mode we simply assume that the text is already
            // properly encoded in UTF-8 Also, since we don't know how big mp
            // actually is since we get the pointer from the outside we can't
            // use one of the "safe" strcpy variants here...
            strcpy (mp, outbuf);
            mp += strlen (outbuf);
        }
        else {
            mp += maybe_convert_foreign_characters (outbuf, mp, false);
        }
    }

    // Make sure the string is terminated properly
    *mp = *mp_raw = '\0';
    /*
        while (cfgets(outbuf, PARSE_BUF_SIZE, mf) != NULL) {
            if (strlen(outbuf) >= PARSE_BUF_SIZE-1)
                error_display(0, "Input string too long.  Max is %i
       characters.\n%.256s\n", PARSE_BUF_SIZE, outbuf);

            // If you hit this assert, it is probably telling you the obvious.
       The file
            // you are trying to read is truly too large.  Look at *filename
       to see the file name. Assert(mp_raw - file_text_raw + strlen(outbuf) <
       PARSE_TEXT_SIZE); strcpy_s(mp_raw, outbuf); mp_raw += strlen(outbuf);

            in_comment = strip_comments(outbuf, in_comment);
            strcpy_s(mp, outbuf);
            mp += strlen(outbuf);
        }

        *mp = *mp_raw = EOF_CHAR;
    */
}

void debug_show_mission_text () {
    char* mp = Parse_text;
    char ch;

    while ((ch = *mp++) != '\0') printf ("%c", ch);
}

static bool atof2 (float* out) {
    ignore_white_space ();
    char ch = *Mp;

    if ((ch != '.') && (ch != '-') && (ch != '+') &&
        ((ch < '0') || (ch > '9'))) {
        error_display (1, "Expecting float, found [%.32s].\n", next_tokens ());
        *out = 0.0f;
        return false;
    }

    *out = (float)atof (Mp);
    return true;
}

static bool atoi2 (int* out) {
    ignore_white_space ();
    char ch = *Mp;

    if ((ch != '-') && (ch != '+') && ((ch < '0') || (ch > '9'))) {
        error_display (1, "Expecting int, found [%.32s].\n", next_tokens ());
        *out = 0;
        return false;
    }

    *out = atoi (Mp);
    return true;
}

bool atol2 (long* out) {
    ignore_white_space ();
    char ch = *Mp;

    if ((ch != '-') && (ch != '+') && ((ch < '0') || (ch > '9'))) {
        error_display (1, "Expecting long, found [%.32s].\n", next_tokens ());
        *out = 0;
        return false;
    }

    *out = atol (Mp);
    return true;
}

// Stuff a floating point value pointed at by Mp.
// Advances past float characters.
void stuff_float (float* f) {
    bool success = atof2 (f);

    if (!success)
        skip_token ();
    else
        Mp += strspn (Mp, "+-0123456789.");

    if (*Mp == ',') Mp++;

    diag_printf ("Stuffed float: %f", *f);
}

int stuff_float_optional (float* f, bool raw) {
    bool comma = false;

    if (!raw) ignore_white_space ();

    auto skip_len = strspn (Mp, "+-0123456789.");
    if (*(Mp + skip_len) == ',') { comma = true; }

    if (skip_len == 0) {
        if (comma) {
            Mp++;
            return 1;
        }
        else {
            return 0;
        }
    }

    stuff_float (f);
    return 2;
}

// Stuff an integer value pointed at by Mp.
// Advances past integer characters.
void stuff_int (int* i) {
    bool success = atoi2 (i);

    if (!success)
        skip_token ();
    else
        Mp += strspn (Mp, "+-0123456789");

    if (*Mp == ',') Mp++;

    diag_printf ("Stuffed int: %i", *i);
}

int stuff_int_optional (int* i, bool raw) {
    bool comma = false;

    if (!raw) ignore_white_space ();

    auto skip_len = strspn (Mp, "+-0123456789");
    if (*(Mp + skip_len) == ',') { comma = true; }

    if (skip_len == 0) {
        if (comma) {
            Mp++;
            return 1;
        }
        else {
            return 0;
        }
    }

    stuff_int (i);
    return 2;
}

int stuff_int_or_variable (int& i, bool positive_value = false);
int stuff_int_or_variable (int* ilp, int count, bool positive_value = false);

// Stuffs an int value or the value of a number variable. Returns the index of
// the variable or NOT_SET_BY_SEXP_VARIABLE.
int stuff_int_or_variable (int& i, bool positive_value) {
    int index = NOT_SET_BY_SEXP_VARIABLE;

    if (*Mp == '@') {
        Mp++;
        int value = -1;
        char str[128];
        stuff_string (str, F_NAME, sizeof (str));

        index = get_index_sexp_variable_name (str);

        if (index > -1 && index < MAX_SEXP_VARIABLES) {
            if (Sexp_variables[index].type & SEXP_VARIABLE_NUMBER) {
                value = atoi (Sexp_variables[index].text);
            }
            else {
                ASSERTX (0, "Invalid variable type \"%s\" found in mission. Variable must be a number variable!",str);
            }
        }
        else {
            ASSERTX (0, "Invalid variable name \"%s\" found.", str);
        }

        // zero negative values if requested
        if (positive_value && value < 0) { value = 0; }

        // Record the value of the index for FreeSpace
        i = value;
    }
    else {
        stuff_int (&i);
    }
    return index;
}

// Stuff an integer value pointed at by Mp.If a variable is found instead stuff
// the value of that variable and record the index of the variable in the
// following slot.
int stuff_int_or_variable (int* ilp, int count, bool positive_value) {
    if (*Mp == '@') {
        Mp++;
        int value = -1;
        char str[128];
        stuff_string (str, F_NAME, sizeof (str));

        int index = get_index_sexp_variable_name (str);

        if (index > -1 && index < MAX_SEXP_VARIABLES) {
            if (Sexp_variables[index].type & SEXP_VARIABLE_NUMBER) {
                value = atoi (Sexp_variables[index].text);
            }
            else {
                ASSERTX (0, "Invalid variable type \"%s\" found in mission. Variable must be a number variable!",str);
            }
        }
        else {
            ASSERTX (0, "Invalid variable name \"%s\" found.", str);
        }

        // zero negative values if requested
        if (positive_value && value < 0) { value = 0; }

        // Record the value of the index for FreeSpace
        ilp[count++] = value;
        // Record the index itself because we may need it later.
        ilp[count++] = index;
    }
    else {
        stuff_int (&ilp[count++]);
        // Since we have a numerical value we don't have a SEXP variable index
        // to add for next slot.
        ilp[count++] = NOT_SET_BY_SEXP_VARIABLE;
    }
    return count;
}

// Stuffs boolean value.
// Passes things off to stuff_boolean(bool)
void stuff_boolean (int* i, bool a_to_eol) {
    bool tempb;
    stuff_boolean (&tempb, a_to_eol);
    if (tempb)
        *i = 1;
    else
        *i = 0;
}

void stuff_boolean_flag (int* i, int flag, bool a_to_eol) {
    bool temp;
    stuff_boolean (&temp, a_to_eol);
    if (temp)
        *i |= flag;
    else
        *i &= ~(flag);
}

// Stuffs a boolean value pointed at by Mp.
// YES/NO (supporting 1/0 now as well)
// Now supports localization :) -WMC

void stuff_boolean (bool* b, bool a_to_eol) {
    char token[32];
    stuff_string_white (token, sizeof (token) / sizeof (char));
    if (a_to_eol) advance_to_eoln (NULL);

    if (isdigit (token[0])) {
        if (token[0] != '0')
            *b = true;
        else
            *b = false;
    }
    else {
        if (!strcasecmp (token, "yes") || !strcasecmp (token, "true") ||
            !strcasecmp (token, "ja")          // German
            || !strcasecmp (token, "Oui")      // French
            || !strcasecmp (token, "si")       // Spanish
            || !strcasecmp (token, "ita vero") // Latin
            || !strcasecmp (token, "HIja'") ||
            !strcasecmp (token, "HISlaH")) // Klingon
        {
            *b = true;
        }
        else if (
            !strcasecmp (token, "no") || !strcasecmp (token, "false") ||
            !strcasecmp (token, "nein")   // German
            || !strcasecmp (token, "Non") // French
            // I don't know spanish for "no"
            // But according to altavista, spanish for "No" is "no"
            // Go figure.
            || !strcasecmp (token, "minime")  // Latin
            || !strcasecmp (token, "ghobe'")) // Klingon
        {
            *b = false;
        }
        else {
            *b = false;
            WARNINGF (
                LOCATION, "Boolean '%s' type unknown; assuming 'no/false'",
                token);
        }
    }

    diag_printf ("Stuffed bool: %s", (b) ? NOX ("true") : NOX ("false"));
}

int stuff_bool_list (bool* blp, int max_bools) {
    int count = 0;
    bool trash_buf = false;

    ignore_white_space ();

    if (*Mp != '(') {
        error_display (
            1, "Reading boolean list.  Found [%c].  Expecting '('.\n", *Mp);
        throw parse::ParseException ("Syntax error");
    }

    Mp++;

    ignore_white_space ();

    while (*Mp != ')') {
        if (count < max_bools) {
            stuff_boolean (&blp[count++], false);
            ignore_white_space ();

            // Since Bobb has set a precedent, allow commas for bool lists -WMC
            if (*Mp == ',') {
                Mp++;
                ignore_white_space ();
            }
        }
        else {
            trash_buf = true;
            break;
        }
    }

    if (trash_buf) {
        error_display (
            0,
            "Boolean list has more than allowed arguments; max is %d. "
            "Arguments over max will be ignored.",
            max_bools);
        while (*Mp != ')') {
            stuff_boolean (&trash_buf, false);
            ignore_white_space ();
        }
    }

    Mp++;

    return count;
}

// Stuff an integer value pointed at by Mp.
// Advances past integer characters.
void stuff_ubyte (ubyte* i) {
    int temp;
    bool success = atoi2 (&temp);

    *i = (ubyte)temp;

    if (!success)
        skip_token ();
    else
        Mp += strspn (Mp, "+-0123456789");

    if (*Mp == ',') Mp++;

    diag_printf ("Stuffed byte: %i", *i);
}

int parse_string_flag_list (int* dest, flag_def_list defs[], int defs_size) {
    ASSERT (dest != NULL); // wtf?

    char(*slp)[NAME_LENGTH] = (char(*)[32]) new char[defs_size * NAME_LENGTH];
    int num_strings = stuff_string_list (slp, defs_size);
    int i, j;

    for (i = 0; i < num_strings; i++) {
        for (j = 0; j < defs_size; j++) {
            if (!strcasecmp (slp[i], defs[j].name)) { (*dest) |= defs[j].def; }
        }
    }

    delete[] slp; //>_>
    // nobody saw that right

    return num_strings;
}

int stuff_string_list (std::vector< std::string >& slp) {
    //_asm int 3;
    slp.clear ();

    ignore_white_space ();

    if (*Mp != '(') {
        error_display (
            1, "Reading string list.  Found [%c].  Expecting '('.\n", *Mp);
        throw parse::ParseException ("Syntax error");
    }

    Mp++;

    ignore_white_space ();

    std::string buf;

    while (*Mp != ')') {
        if (*Mp != '\"') {
            error_display (0, "Missing quotation marks in string list.");
        }
        // ASSERT ( *Mp == '\"' );                                      // should always be
        // enclosed in quotes

        buf = "";
        get_string (buf);
        slp.push_back (buf);
        ignore_white_space ();
    }

    Mp++;

    return (int)(slp.size ());
}

// Stuffs a list of strings
int stuff_string_list (char slp[][NAME_LENGTH], int max_strings) {
    int count = 0;
    ignore_white_space ();

    if (*Mp != '(') {
        error_display (
            1, "Reading string list.  Found [%c].  Expecting '('.\n", *Mp);
        throw parse::ParseException ("Syntax error");
    }

    Mp++;

    ignore_white_space ();

    while (*Mp != ')') {
        ASSERT (count < max_strings);
        if (*Mp != '\"') {
            error_display (0, "Missing quotation marks in string list.");
        }
        // ASSERT ( *Mp == '\"' );                                      // should always be
        // enclosed in quotes

        if (count < max_strings) { get_string (slp[count++]); }
        else {
            char trash[NAME_LENGTH];
            get_string (trash);
        }
        ignore_white_space ();
    }

    Mp++;

    return count;
}

const char* get_lookup_type_name (int lookup_type) {
    switch (lookup_type) {
    case SHIP_TYPE: return "Ships";
    case SHIP_INFO_TYPE: return "Ship Classes";
    case WEAPON_POOL_TYPE: return "Weapon Pool";
    case WEAPON_LIST_TYPE: return "Weapon Types";
    case RAW_INTEGER_TYPE: return "Untyped integer list";
    }

    return "Unknown lookup type, tell a coder!";
}

// Stuffs an integer list.
// This is of the form ( i* )
// where i is an integer.
// For example, (1) () (1 2 3) ( 1 ) are legal integer lists.
int stuff_int_list (int* ilp, int max_ints, int lookup_type) {
    ASSERTX (
        max_ints > 0,
        "Requested to parse an integer list with a maximum of 0 entries!");

    int count = 0, ok_flag = 1, dummy;
    ignore_white_space ();

    if (*Mp != '(') {
        error_display (
            1, "Reading integer list.  Found [%c].  Expecting '('.\n", *Mp);
        throw parse::ParseException ("Syntax error");
    }

    Mp++;
    ignore_white_space ();

    while (*Mp != ')') {
        if (*Mp == '"') {
            int num = 0;
            char str[128];

            get_string (str);
            switch (lookup_type) {
            case SHIP_TYPE:
                num = ship_name_lookup (
                    str); // returns index of Ship[] entry with name
                break;

            case SHIP_INFO_TYPE:
                ok_flag = 1;
                num = ship_info_lookup (
                    str); // returns index of Ship_info[] entry with name
                if (num < 0) ok_flag = 0;
                break;

            case WEAPON_POOL_TYPE:
                ok_flag = 1;
                num = weapon_info_lookup (str);
                if (num < 0) ok_flag = 0;
                break;

            case WEAPON_LIST_TYPE:
                num = weapon_info_lookup (str);
                if (num < 0) num = -2;
                break;

            case RAW_INTEGER_TYPE: num = atoi (str); break;

            default:
                ASSERTX (0, "Unknown lookup_type %d in stuff_int_list",lookup_type);
                break;
            }

            if (ok_flag) {
                if (num == -1) {
                    error_display (
                        1,
                        "Unable to find string \"%s\" in "
                        "stuff_int_list\n\nMany possible sources for this "
                        "error.  Get a programmer!\n",
                        str);
                }
                else if (num == -2) {
                    if (str[0] != '\0') {
                        error_display (
                            0,
                            "Unable to find WEAPON_LIST_TYPE string \"%s\" in "
                            "stuff_int_list\n\nMany possible sources for this "
                            "error.  Get a programmer!\n",
                            str);
                    }
                }

                if (num < 0) // other negatives used to bypass the above error
                             // trap, but should be -1
                    num = -1;

                if (count < max_ints) { ilp[count] = num; }
                ++count;
            }
        }
        else {
            if (ok_flag && (count < max_ints))
                stuff_int (&ilp[count]);
            else
                stuff_int (&dummy);
            ++count;
        }

        ignore_white_space ();
    }

    Mp++;

    if (count > max_ints) {
        error_display (
            0,
            "Too many entries in integer list. Expected %d, found %d.\nList "
            "type was %s",
            max_ints, count, get_lookup_type_name (lookup_type));
    }

    return count;
}

// helper for the next function. Removes a broken entry from ship or weapon
// lists and advances to the next one
void clean_loadout_list_entry () {
    int dummy;

    // clean out the broken entry
    ignore_white_space ();
    stuff_int_or_variable (dummy);
    ignore_white_space ();
}

// Karajorma - Stuffs an int list by parsing a list of ship or weapon choices.
// Unlike stuff_int_list it can deal with variables and it also has better
// error reporting.
int stuff_loadout_list (int* ilp, int max_ints, int lookup_type) {
    int count = 0;
    int index, sexp_variable_index, variable_found;
    char str[128];

    ignore_white_space ();

    if (*Mp != '(') {
        error_display (
            1, "Reading loadout list.  Found [%c].  Expecting '('.\n", *Mp);
        throw parse::ParseException ("Syntax error");
    }

    Mp++;
    ignore_white_space ();

    while (*Mp != ')') {
        if (count >= max_ints) {
            ASSERTX (0, "Loadout contains too many entries.\n");
        }

        index = -1;
        sexp_variable_index = NOT_SET_BY_SEXP_VARIABLE;
        variable_found = get_string_or_variable (str);

        // if we've got a variable get the variable index and copy it's value
        // into str so that regardless of whether we found a variable or not it
        // now holds the name of the ship or weapon we're interested in.
        if (variable_found) {
            ASSERT (lookup_type != CAMPAIGN_LOADOUT_SHIP_LIST);
            sexp_variable_index = get_index_sexp_variable_name (str);

            if (sexp_variable_index < 0) {
                ASSERTX (0, "Invalid SEXP variable name \"%s\" found in stuff_loadout_list.",str);
            }

            strcpy_s (str, Sexp_variables[sexp_variable_index].text);
        }

        switch (lookup_type) {
        case MISSION_LOADOUT_SHIP_LIST:
        case CAMPAIGN_LOADOUT_SHIP_LIST: index = ship_info_lookup (str); break;

        case MISSION_LOADOUT_WEAPON_LIST:
        case CAMPAIGN_LOADOUT_WEAPON_LIST:
            index = weapon_info_lookup (str);
            break;

        default: Int3 ();
        }

        // Complain if this isn't a valid ship or weapon and we are loading a
        // mission. Campaign files can be loading containing no ships from the
        // current tables (when swapping mods) so don't report that as an
        // error.
        if (index < 0 && (lookup_type == MISSION_LOADOUT_SHIP_LIST ||
                          lookup_type == MISSION_LOADOUT_WEAPON_LIST)) {
            // print a warning in debug mode
            WARNINGF (
                LOCATION,
                "Invalid type \"%s\" found in loadout of mission "
                "file...skipping",
                str);
            // increment counter for release FRED builds.
            Num_unknown_loadout_classes++;

            clean_loadout_list_entry ();
            continue;
        }

        // similarly, complain if this is a valid ship or weapon class that the
        // player can't use
        if ((lookup_type == MISSION_LOADOUT_SHIP_LIST) &&
            (!(Ship_info[index].flags[Ship::Info_Flags::Player_ship]))) {
            clean_loadout_list_entry ();
            WARNINGF (
                LOCATION,
                "Ship type \"%s\" found in loadout of mission file. This "
                "class is not marked as a player ship...skipping",
                str);
            continue;
        }
        else if (
            (lookup_type == MISSION_LOADOUT_WEAPON_LIST) &&
            (!(Weapon_info[index]
                   .wi_flags[Weapon::Info_Flags::Player_allowed]))) {
            clean_loadout_list_entry ();
            WARNINGF (
                LOCATION,
                "Weapon type %s found in loadout of mission file. This class "
                "is not marked as a player allowed weapon...skipping\n",
                str);
            if (!Is_standalone)
                WARNINGF (
                    LOCATION,
                    "Weapon type \"%s\" found in loadout of mission file. "
                    "This class is not marked as a player allowed "
                    "weapon...skipping",
                    str);
            continue;
        }

        // we've found a real item. Add its index to the list.
        if (count < max_ints) { ilp[count++] = index; }

        ignore_white_space ();

        // Campaign lists need go no further
        if (lookup_type == CAMPAIGN_LOADOUT_SHIP_LIST ||
            lookup_type == CAMPAIGN_LOADOUT_WEAPON_LIST) {
            continue;
        }

        // record the index of the variable that gave us this item if any
        if (count < max_ints) { ilp[count++] = sexp_variable_index; }

        // Now read in the number of this type available. The number must be
        // positive
        count = stuff_int_or_variable (ilp, count, true);

        ignore_white_space ();
    }

    Mp++;
    return count;
}

// Stuffs an integer list like stuff_int_list.
size_t stuff_float_list (float* flp, size_t max_floats) {
    size_t count = 0;
    ignore_white_space ();

    if (*Mp != '(') {
        error_display (
            1, "Reading float list.  Found [%c].  Expecting '('.\n", *Mp);
        throw parse::ParseException ("Syntax error");
    }

    Mp++;
    ignore_white_space ();
    while (*Mp != ')') {
        ASSERT (count < max_floats);
        if (count < max_floats) { stuff_float (&flp[count++]); }
        else {
            float dummy;
            stuff_float (&dummy);
        }
        ignore_white_space ();
    }

    Mp++;

    return count;
}

// Marks an integer list.
// This is of the form ( i* )
// where i is an integer.
// If a specified string is found in the lookup and its value is 7, then the
// 7th value    in the array is set.
void mark_int_list (int* ilp, int max_ints, int lookup_type) {
    ignore_white_space ();

    if (*Mp != '(') {
        error_display (
            1, "Marking integer list.  Found [%c].  Expecting '('.\n", *Mp);
        throw parse::ParseException ("Syntax error");
    }

    Mp++;
    ignore_white_space ();

    while (*Mp != ')') {
        if (*Mp == '"') {
            int num = 0;
            char str[128];

            get_string (str);
            switch (lookup_type) {
            case SHIP_TYPE:
                num = ship_name_lookup (
                    str); // returns index of Ship[] entry with name
                break;

            case SHIP_INFO_TYPE:
                num = ship_info_lookup (
                    str); // returns index of Ship_info[] entry with name
                break;

            case WEAPON_LIST_TYPE: num = weapon_info_lookup (str); break;

            default:
                ASSERTX (0, "Unknown lookup_type %d in mark_int_list",lookup_type);
                break;
            }

            if ((num < 0) || (num >= max_ints))
                ASSERTX (0, "Unable to find string \"%s\" in mark_int_list.\n", str);

            // ilp[num] = 1;
        }
        else {
            int tval;

            stuff_int (&tval);
            ASSERT ((tval >= 0) && (tval < max_ints));
            if (tval >= 0 && tval < max_ints) { ilp[tval] = 1; }
        }

        ignore_white_space ();
    }

    Mp++;
}

// Stuff a vec3d struct, which is 3 floats.
void stuff_vec3d (vec3d* vp) {
    stuff_float (&vp->xyz.x);
    stuff_float (&vp->xyz.y);
    stuff_float (&vp->xyz.z);
}

void stuff_parenthesized_vec3d (vec3d* vp) {
    ignore_white_space ();

    if (*Mp != '(') {
        error_display (
            1, "Reading parenthesized vec3d.  Found [%c].  Expecting '('.\n",
            *Mp);
        throw parse::ParseException ("Syntax error");
    }
    else {
        Mp++;
        stuff_vec3d (vp);
        ignore_white_space ();
        if (*Mp != ')') {
            error_display (
                1,
                "Reading parenthesized vec3d.  Found [%c].  Expecting ')'.\n",
                *Mp);
            throw parse::ParseException ("Syntax error");
        }
        Mp++;
    }
}

// Stuffs vec3d list.  *vlp is an array of vec3ds.
// This is of the form ( (vec3d)* )
// (where * is a kleene star, not a pointer indirection)
// For example, ( (1 2 3) (2 3 4) (2 3 5) )
// is a list of three vec3ds.
int stuff_vec3d_list (vec3d* vlp, int max_vecs) {
    int count = 0;

    ignore_white_space ();

    if (*Mp != '(') {
        error_display (
            1, "Reading vec3d list.  Found [%c].  Expecting '('.\n", *Mp);
        throw parse::ParseException ("Syntax error");
    }

    Mp++;

    ignore_white_space ();

    while (*Mp != ')') {
        ASSERT (count < max_vecs);
        if (count < max_vecs) { stuff_parenthesized_vec3d (&vlp[count++]); }
        else {
            vec3d temp;
            stuff_parenthesized_vec3d (&temp);
        }

        ignore_white_space ();
    }

    Mp++;

    return count;
}

// ditto the above, but a vector of vec3ds...
int stuff_vec3d_list (std::vector< vec3d >& vec_list) {
    ignore_white_space ();

    if (*Mp != '(') {
        error_display (
            1, "Reading vec3d list.  Found [%c].  Expecting '('.\n", *Mp);
        throw parse::ParseException ("Syntax error");
    }

    Mp++;

    ignore_white_space ();

    while (*Mp != ')') {
        vec3d temp;
        stuff_parenthesized_vec3d (&temp);
        vec_list.push_back (temp);

        ignore_white_space ();
    }

    Mp++;

    return (int)vec_list.size ();
}

// Stuff a matrix, which is 3 vec3ds.
void stuff_matrix (matrix* mp) {
    stuff_vec3d (&mp->vec.rvec);
    stuff_vec3d (&mp->vec.uvec);
    stuff_vec3d (&mp->vec.fvec);
}

/**
 * @brief Given a string, find it in a string array.
 *
 * @param str1 is the string to be found.
 * @param strlist is the list of strings to search.
 * @param max is the number of entries in *strlist to scan.
 * @param description is only used for diagnostics in case it can't be found.
 * @param say_errors @c true if errors should be reported
 * @return
 */
int string_lookup (
    const char* str1, const char* const* strlist, size_t max,
    const char* description, bool say_errors) {
    for (size_t i = 0; i < max; i++) {
        ASSERT (strlen (strlist[i]) != 0); //-V805

        if (!strcasecmp (str1, strlist[i])) return (int)i;
    }

    if (say_errors)
        error_display (
            0, "Unable to find [%s] in %s list.\n", str1, description);

    return -1;
}

// Find a required string (*id), then stuff the text of type f_type that
// follows it at *addr.  *strlist[] contains the strings it should try to
// match.
void find_and_stuff (
    const char* id, int* addr, int f_type, const char* strlist[], size_t max,
    const char* description) {
    char token[128];
    int checking_ship_classes = (strcasecmp (id, "$class:") == 0);

    // Goober5000 - don't say errors when we're checking classes because 1) we
    // have more checking to do; and 2) we will say a redundant error later
    required_string (id);
    stuff_string (token, f_type, sizeof (token));
    *addr = string_lookup (
        token, strlist, max, description, !checking_ship_classes);

    // Goober5000 - handle certain FSPort idiosyncracies with ship classes
    if (*addr < 0 && checking_ship_classes) {
        int idx = ship_info_lookup (token);

        if (idx >= 0)
            *addr = string_lookup (
                Ship_info[idx].name, strlist, max, description, 0);
        else
            *addr = -1;
    }
}

void find_and_stuff_optional (
    const char* id, int* addr, int f_type, const char* const* strlist,
    size_t max, const char* description) {
    char token[128];

    if (optional_string (id)) {
        stuff_string (token, f_type, sizeof (token));
        *addr = string_lookup (token, strlist, max, description, 1);
    }
}

// Mp points at a string.
// Find the string in the list of strings *strlist[].
// Returns the index of the match, -1 if none.
int match_and_stuff (
    int f_type, const char* const* strlist, int max, const char* description) {
    char token[128];

    stuff_string (token, f_type, sizeof (token));
    return string_lookup (token, strlist, max, description, 0);
}

void find_and_stuff_or_add (
    const char* id, int* addr, int f_type, char* strlist[], int* total,
    int max, const char* description) {
    char token[128];

    *addr = -1;
    required_string (id);
    stuff_string (token, f_type, sizeof (token));
    if (*total) *addr = string_lookup (token, strlist, *total, description, 0);

    if (*addr == -1) // not in list, so lets try and add it.
    {
        ASSERT (*total < max);
        strcpy (strlist[*total], token);
        *addr = (*total)++;
    }
}

// pause current parsing so that some else can be parsed without interfering
// with the currently parsing file
void pause_parse () {
    ASSERT (!Parsing_paused);
    if (Parsing_paused) return;

    Mp_save = Mp;

    Warning_count_save = Warning_count;
    Error_count_save = Error_count;

    strcpy_s (Current_filename_save, Current_filename);

    Parsing_paused = 1;
}

// unpause parsing to continue with previously parsing file
void unpause_parse () {
    ASSERT (Parsing_paused);
    if (!Parsing_paused) return;

    Mp = Mp_save;

    Warning_count = Warning_count_save;
    Error_count = Error_count_save;

    strcpy_s (Current_filename, Current_filename_save);

    Parsing_paused = 0;
}

void reset_parse (char* text) {
    if (text != NULL) { Mp = text; }
    else {
        Mp = Parse_text;
    }

    Warning_count = 0;
    Error_count = 0;

    strcpy_s (Current_filename, Current_filename_sub);
}

// Display number of warnings and errors at the end of a parse.
void display_parse_diagnostics () {
    WARNINGF (LOCATION, "\nParse complete.");
    WARNINGF (
        LOCATION, "%i errors.  %i warnings.\n", Error_count, Warning_count);
}

// Splits a string into 2 lines if the string is wider than max_pixel_w pixels.
// A null terminator is placed where required to make the first line <=
// max_pixel_w.  The remaining text is returned (leading whitespace removed).
// If the line doesn't need to be split, NULL is returned.
char* split_str_once (char* src, int max_pixel_w) {
    char* brk = NULL;
    int i, w, len, last_was_white = 0;

    ASSERT (src);
    ASSERT (max_pixel_w > 0);

    gr_get_string_size (&w, NULL, src);
    if ((w <= max_pixel_w) && !strstr (src, "\n")) {
        return NULL; // string doesn't require a cut
    }

    len = (int)strlen (src);
    for (i = 0; i < len; i++) {
        gr_get_string_size (&w, NULL, src, i);
        if (w > max_pixel_w) break;

        if (src[i] == '\n') { // reached natural end of line
            src[i] = 0;
            return src + i + 1;
        }

        if (is_white_space (src[i])) {
            if (!last_was_white) brk = src + i;

            last_was_white = 1;
        }
        else {
            last_was_white = 0;
        }
    }

    // if we are over max pixel width and weren't able to come up with a good
    // non-word split then just return the original src text and the calling
    // function should have to handle the result
    if ((w > max_pixel_w) && ((i == 0) || !brk)) { return src; }

    if (!brk) { brk = src + i; }

    *brk = 0;
    src = brk + 1;
    while (is_white_space (*src)) src++;

    if (!*src) return NULL; // end of the string anyway

    if (*src == '\n') src++;

    return src;
}

#define SPLIT_STR_BUFFER_SIZE 512

// --------------------------------------------------------------------------------------
// split_str()
//
// A general function that will split a string into several lines.  Lines are
// allowed up to max_pixel_w pixels.  Breaks are found in white space.
//
// Supports \n's in the strings!
//
// parameters:          src                     =>              source string to be broken up
// max_pixel_w     =>              max width of line in pixels
// n_chars         =>              output array that will hold number
// of characters in each line                                           p_str                   =>              output array of pointers
// to start of
// lines within src                                             max_lines       =>              limit of number
// of
// lines to break src up into                                           ignore_char     =>              OPTIONAL
// parameter (default val -1).  Ignore words starting with this character
// This is useful when you want to ignore embedded control information that
// starts with a specific character, like $ or
//#
//
// returns:                        number of lines src is broken into
// -1 is returned when an error occurs
//
int split_str (
    const char* src, int max_pixel_w, int* n_chars, const char** p_str,
    int max_lines, unicode::codepoint_t ignore_char,
    bool strip_leading_whitespace) {
    char buffer[SPLIT_STR_BUFFER_SIZE];
    const char* breakpoint = NULL;
    int sw, new_line = 1, line_num = 0, last_was_white = 0;
    int ignore_until_whitespace, buf_index;

    // check our assumptions..
    ASSERT (src != NULL);
    ASSERT (n_chars != NULL);
    ASSERT (p_str != NULL);
    ASSERT (max_lines > 0);
    ASSERT (max_pixel_w > 0);

    memset (buffer, 0, sizeof (buffer));
    buf_index = 0;
    ignore_until_whitespace = 0;

    // get rid of any leading whitespace
    while (strip_leading_whitespace && is_white_space (*src)) src++;

    new_line = 1;
    p_str[0] = NULL;

    // iterate through chars in line, keeping track of most recent "white
    // space" location that can be used as a line splitting point if necessary
    unicode::codepoint_range range (src);
    auto end_iter = std::end (range);
    auto iter = std::begin (range);
    for (; iter != end_iter; ++iter) {
        auto cp = *iter;

        if (line_num >= max_lines) return line_num; // time to bail out

        // starting a new line of text, init stuff for that
        if (new_line) {
            p_str[line_num] = NULL;
            if (strip_leading_whitespace && is_gray_space (cp)) continue;

            p_str[line_num] = iter.pos ();
            breakpoint = NULL;
            new_line = 0;
        }

        // maybe skip leading whitespace
        if (ignore_until_whitespace) {
            if (is_white_space (cp)) ignore_until_whitespace = 0;

            continue;
        }

        // if we have a newline, split the line here
        if (cp == U'\n') {
            n_chars[line_num] =
                (int)(iter.pos () - p_str[line_num]); // track length of line
            line_num++;
            if (line_num < max_lines) { p_str[line_num] = NULL; }
            new_line = 1;

            memset (buffer, 0, SPLIT_STR_BUFFER_SIZE);
            buf_index = 0;
            continue;
        }

        if (cp == ignore_char) {
            ignore_until_whitespace = 1;
            continue;
        }

        if (is_gray_space (cp)) {
            if (!last_was_white) // track at first whitespace in a series of
                                 // whitespace
                breakpoint = iter.pos ();

            last_was_white = 1;
        }
        else {
            // indicate next time around that this wasn't a whitespace
            // character
            last_was_white = 0;
        }

        auto encoded_width = unicode::encoded_size (cp);
        ASSERTX (
            buf_index + encoded_width < SPLIT_STR_BUFFER_SIZE,
            "buffer overflow in split_str: screen width causes this text to "
            "be longer than %d characters!",
            SPLIT_STR_BUFFER_SIZE - 1);

        // throw it in our buffer
        unicode::encode (cp, &buffer[buf_index]);
        buf_index += (int)encoded_width;
        buffer[buf_index] = 0; // null terminate it

        gr_get_string_size (&sw, NULL, buffer);
        if (sw >= max_pixel_w) {
            const char* end;

            if (breakpoint) {
                end = breakpoint;
                iter = unicode::text_iterator (
                    breakpoint, src, src + strlen (src));
            }
            else {
                end = iter.pos (); // force a split here since to whitespace
                --iter;            // reuse this character in next line
            }

            n_chars[line_num] =
                (int)(end - p_str[line_num]); // track length of line
            ASSERT (n_chars[line_num]);
            line_num++;
            if (line_num < max_lines) { p_str[line_num] = NULL; }
            new_line = 1;

            memset (buffer, 0, sizeof (buffer));
            buf_index = 0;
            continue;
        }
    } // end for

    if (!new_line && p_str[line_num]) {
        n_chars[line_num] =
            (int)(iter.pos () - p_str[line_num]); // track length of line
        ASSERT (n_chars[line_num]);
        line_num++;
    }

    return line_num;
}

int split_str (
    const char* src, int max_pixel_w, std::vector< int >& n_chars,
    std::vector< const char* >& p_str, unicode::codepoint_t ignore_char,
    bool strip_leading_whitespace) {
    char buffer[SPLIT_STR_BUFFER_SIZE];
    const char* breakpoint = NULL;
    int sw, new_line = 1, line_num = 0, last_was_white = 0;
    int ignore_until_whitespace = 0, buf_index = 0;

    // check our assumptions..
    ASSERT (src != NULL);
    ASSERT (max_pixel_w > 0);

    memset (buffer, 0, sizeof (buffer));

    // get rid of any leading whitespace
    while (strip_leading_whitespace && is_white_space (*src)) src++;

    p_str.clear ();

    // iterate through chars in line, keeping track of most recent "white
    // space" location that can be used as a line splitting point if necessary
    unicode::codepoint_range range (src);
    auto end_iter = std::end (range);
    auto iter = std::begin (range);
    for (; iter != end_iter; ++iter) {
        auto cp = *iter;

        // starting a new line of text, init stuff for that
        if (new_line) {
            if (strip_leading_whitespace && is_gray_space (cp)) continue;

            p_str.push_back (iter.pos ());
            breakpoint = NULL;
            new_line = 0;
        }

        // maybe skip leading whitespace
        if (ignore_until_whitespace) {
            if (is_white_space (cp)) {
                ignore_until_whitespace = 0;

                // don't eat the newline
                if (cp == EOLN) --iter;
            }

            continue;
        }

        // if we have a newline, split the line here
        if (cp == U'\n') {
            n_chars.push_back (
                (int)(iter.pos () - p_str[line_num])); // track length of line
            line_num++;
            new_line = 1;

            memset (buffer, 0, SPLIT_STR_BUFFER_SIZE);
            buf_index = 0;
            continue;
        }

        if (cp == ignore_char) {
            ignore_until_whitespace = 1;
            continue;
        }

        if (is_gray_space (cp)) {
            if (!last_was_white) // track at first whitespace in a series of
                                 // whitespace
                breakpoint = iter.pos ();

            last_was_white = 1;
        }
        else {
            // indicate next time around that this wasn't a whitespace
            // character
            last_was_white = 0;
        }

        auto encoded_width = unicode::encoded_size (cp);
        ASSERTX (
            buf_index + encoded_width < SPLIT_STR_BUFFER_SIZE,
            "buffer overflow in split_str: screen width causes this text to "
            "be longer than %d characters!",
            SPLIT_STR_BUFFER_SIZE - 1);

        // throw it in our buffer
        unicode::encode (cp, &buffer[buf_index]);
        buf_index += (int)encoded_width;
        buffer[buf_index] = 0; // null terminate it

        gr_get_string_size (&sw, NULL, buffer);
        if (sw >= max_pixel_w) {
            const char* end;

            if (breakpoint) {
                end = breakpoint;
                iter = unicode::text_iterator (
                    breakpoint, src, src + strlen (src));
            }
            else {
                end = iter.pos (); // force a split here since to whitespace
                --iter;            // reuse this character in next line
            }

            n_chars.push_back (
                (int)(end - p_str[line_num])); // track length of line
            ASSERT (n_chars[line_num]);
            line_num++;
            new_line = 1;

            memset (buffer, 0, sizeof (buffer));
            buf_index = 0;
            continue;
        }
    } // end for

    if (!new_line && p_str[line_num]) {
        n_chars.push_back (
            (int)(iter.pos () - p_str[line_num])); // track length of line
        ASSERT (n_chars[line_num]);
        line_num++;
    }

    return line_num;
}

// Goober5000
// accounts for the dumb communications != communication, etc.
int subsystem_strcasecmp (const char* str1, const char* str2) {
    ASSERT (str1 && str2);

    // ensure len-1 will be valid
    if (!*str1 || !*str2) return strcasecmp (str1, str2);

    // calc lengths
    auto len1 = (int)strlen (str1);
    auto len2 = (int)strlen (str2);

    // get rid of trailing s on s1?
    if (tolower (*(str1 + len1 - 1) == 's')) len1--;

    // get rid of trailing s on s2?
    if (tolower (*(str2 + len2 - 1) == 's')) len2--;

    // once we remove the trailing s on both names, they should be the same
    // length
    if (len1 > len2) return 1;
    if (len1 < len2) return -1;

    // now do the comparison
    return strncasecmp (str1, str2, len1);
}

// Goober5000
// current algorithm adapted from
// http://www.codeproject.com/string/stringsearch.asp
const char* stristr (const char* str, const char* substr) {
    // check for null and insanity
    ASSERT (str);
    ASSERT (substr);
    if (str == NULL || substr == NULL || *substr == '\0') return NULL;

    // save both a lowercase and an uppercase version of the first character of
    // substr
    char substr_ch_lower = (char)tolower (*substr);
    char substr_ch_upper = (char)toupper (*substr);

    // find the maximum distance to search
    const char* upper_bound = str + strlen (str) - strlen (substr);

    // loop through every character of str
    for (const char* start = str; start <= upper_bound; start++) {
        // check first character of substr
        if ((*start == substr_ch_upper) || (*start == substr_ch_lower)) {
            // first character matched, so check the rest
            for (const char *str_ch = start + 1, *substr_ch = substr + 1;
                 *substr_ch != '\0'; str_ch++, substr_ch++) {
                // character match?
                if (*str_ch == *substr_ch) continue;

                // converted character match?
                if (tolower (*str_ch) == tolower (*substr_ch)) continue;

                // mismatch
                goto stristr_continue_outer_loop;
            }

            // finished inner loop with success!
            return start;
        }

    stristr_continue_outer_loop:
        /* NO-OP */;
    }

    // no match
    return NULL;
}

// non-const version
char* stristr (char* str, const char* substr) {
    // check for null and insanity
    ASSERT (str);
    ASSERT (substr);
    if (str == NULL || substr == NULL || *substr == '\0') return NULL;

    // save both a lowercase and an uppercase version of the first character of
    // substr
    char substr_ch_lower = (char)tolower (*substr);
    char substr_ch_upper = (char)toupper (*substr);

    // find the maximum distance to search
    const char* upper_bound = str + strlen (str) - strlen (substr);

    // loop through every character of str
    for (char* start = str; start <= upper_bound; start++) {
        // check first character of substr
        if ((*start == substr_ch_upper) || (*start == substr_ch_lower)) {
            // first character matched, so check the rest
            for (const char *str_ch = start + 1, *substr_ch = substr + 1;
                 *substr_ch != '\0'; str_ch++, substr_ch++) {
                // character match?
                if (*str_ch == *substr_ch) continue;

                // converted character match?
                if (tolower (*str_ch) == tolower (*substr_ch)) continue;

                // mismatch
                goto stristr_continue_outer_loop;
            }

            // finished inner loop with success!
            return start;
        }

    stristr_continue_outer_loop:
        /* NO-OP */;
    }

    // no match
    return NULL;
}

// Goober5000
bool can_construe_as_integer (const char* text) {
    // trivial case; evaluates to 0
    if (*text == '\0') return true;

    // number sign or digit for first char
    if ((*text != '+') && (*text != '-') && !isdigit (*text)) return false;

    // check digits for rest
    for (const char* p = text + 1; *p != '\0'; p++) {
        if (!isdigit (*p)) return false;
    }

    return true;
}

// Goober5000
// yoinked gratefully from dbugfile.cpp
void vsprintf (std::string& dest, const char* format, va_list ap) {
    va_list copy;
    va_copy (copy, ap);

    int needed_length = vsnprintf (nullptr, 0, format, copy);
    va_end (copy);

    if (needed_length < 0) {
        // Error
        return;
    }

    dest.resize (static_cast< size_t > (needed_length));
    vsnprintf (&dest[0], dest.size () + 1, format, ap);
}

void sprintf (std::string& dest, const char* format, ...) {
    va_list args;
    va_start (args, format);
    vsprintf (dest, format, args);
    va_end (args);
}

// Goober5000
bool end_string_at_first_hash_symbol (char* src) {
    char* p;
    ASSERT (src);

    p = get_pointer_to_first_hash_symbol (src);
    if (p) {
        while ((p != src) && (*(p - 1) == ' ')) p--;

        *p = '\0';
        return true;
    }

    return false;
}

// Goober5000
bool end_string_at_first_hash_symbol (std::string& src) {
    int index = get_index_of_first_hash_symbol (src);
    if (index >= 0) {
        while (index > 0 && src[index - 1] == ' ') index--;

        src.resize (index);
        return true;
    }

    return false;
}

// Goober5000
char* get_pointer_to_first_hash_symbol (char* src) {
    ASSERT (src);
    return strchr (src, '#');
}

// Goober5000
const char* get_pointer_to_first_hash_symbol (const char* src) {
    ASSERT (src);
    return strchr (src, '#');
}

// Goober5000
int get_index_of_first_hash_symbol (std::string& src) {
    size_t pos = src.find ('#');
    return (pos == std::string::npos) ? -1 : (int)pos;
}

// Goober5000
ptrdiff_t replace_one (
    char* str, const char* oldstr, const char* newstr, size_t max_len,
    ptrdiff_t range) {
    ASSERT (str && oldstr && newstr);

    // search
    char* ch = stristr (str, oldstr);

    // found?
    if (ch) {
        // not found within bounds?
        if ((range > 0) && ((ch - str) > range)) { return 0; }

        // determine if replacement will exceed max len
        if (strlen (str) + strlen (newstr) - strlen (oldstr) > max_len) {
            return -1;
        }

        // allocate temp string to hold extra stuff
        char* temp = (char*)vm_malloc (sizeof (char) * max_len);

        // ensure allocation was successful
        if (temp) {
            // save remainder of string
            strcpy_s (temp, sizeof (char) * max_len, ch + strlen (oldstr));

            // replace
            strcpy (ch, newstr);

            // append rest of string
            strcpy (ch + strlen (newstr), temp);
        }

        // free temp string
        vm_free (temp);
    }
    // not found
    else {
        return 0;
    }

    // return pos of replacement
    return (ch - str);
}

// Goober5000
ptrdiff_t replace_all (
    char* str, const char* oldstr, const char* newstr, size_t max_len,
    ptrdiff_t range) {
    ptrdiff_t val, tally = 0;

    while ((val = replace_one (str, oldstr, newstr, max_len, range)) > 0) {
        tally++;

        // adjust range (if we have one), because the text length might have
        // changed
        if (range) { range += strlen (newstr) - strlen (oldstr); }
    }

    return (val < 0) ? val : tally;
}

std::string& replace_one (
    std::string& context, const std::string& from, const std::string& to) {
    size_t foundHere;
    if ((foundHere = context.find (from, 0)) != std::string::npos) {
        context.replace (foundHere, from.length (), to);
    }
    return context;
}

std::string&
replace_one (std::string& context, const char* from, const char* to) {
    size_t foundHere;
    if ((foundHere = context.find (from, 0)) != std::string::npos) {
        context.replace (foundHere, strlen (from), to);
    }
    return context;
}

// http://www.cppreference.com/wiki/string/replace
std::string& replace_all (
    std::string& context, const std::string& from, const std::string& to) {
    size_t from_len = from.length ();
    size_t to_len = to.length ();

    size_t lookHere = 0;
    size_t foundHere;
    while ((foundHere = context.find (from, lookHere)) != std::string::npos) {
        context.replace (foundHere, from_len, to);
        lookHere = foundHere + to_len;
    }
    return context;
}

// http://www.cppreference.com/wiki/string/replace
std::string&
replace_all (std::string& context, const char* from, const char* to) {
    size_t from_len = strlen (from);
    size_t to_len = strlen (to);

    size_t lookHere = 0;
    size_t foundHere;
    while ((foundHere = context.find (from, lookHere)) != std::string::npos) {
        context.replace (foundHere, from_len, to);
        lookHere = foundHere + to_len;
    }
    return context;
}

// WMC
// Compares two strings, ignoring (last) extension
// Returns 0 if equal, nonzero if not
int strextcmp (const char* s1, const char* s2) {
    // sanity check
    ASSERT ((s1 != NULL) && (s2 != NULL));

    // find last '.' in both strings
    char* s1_end = (char*)strrchr (s1, '.');
    char* s2_end = (char*)strrchr (s2, '.');

    // get length
    size_t s1_len, s2_len;

    if (s1_end != NULL)
        s1_len = (s1_end - s1);
    else
        s1_len = strlen (s1);

    if (s2_end != NULL)
        s2_len = (s2_end - s2);
    else
        s2_len = strlen (s2);

    // if the lengths aren't the same then it's deffinitely not the same name
    if (s2_len != s1_len) return 1;

    return strncasecmp (s1, s2, s1_len);
}

// Goober5000
bool drop_extension (char* str) {
    char* p = strrchr (str, '.');
    if (p != NULL) {
        *p = 0;
        return true;
    }

    return false;
}

// Goober5000
bool drop_extension (std::string& str) {
    size_t pos = str.rfind ('.');
    if (pos != std::string::npos) {
        str.resize (pos);
        return true;
    }

    return false;
}

// WMC
void backspace (char* src) {
    ASSERT (src != NULL); // this would be bad

    char* dest = src;
    src++;

    while (*src != '\0') { *dest++ = *src++; }

    *dest = '\0';
}

// Goober5000
void format_integer_with_commas (
    char* buf, int integer, bool use_comma_with_four_digits) {
    int old_pos, new_pos, triad_count;
    char backward_buf[32];

    // print an initial string of just the digits
    sprintf (buf, "%d", integer);

    // no commas needed?
    if ((integer < 1000) || (integer < 10000 && !use_comma_with_four_digits))
        return;

    // scan the string backwards, writing commas after every third digit
    new_pos = 0;
    triad_count = 0;
    for (old_pos = (int)strlen (buf) - 1; old_pos >= 0; old_pos--) {
        backward_buf[new_pos] = buf[old_pos];
        new_pos++;
        triad_count++;

        if (triad_count == 3 && old_pos > 0) {
            backward_buf[new_pos] = ',';
            new_pos++;
            triad_count = 0;
        }
    }
    backward_buf[new_pos] = '\0';

    // now reverse the string
    new_pos = 0;
    for (old_pos = (int)strlen (backward_buf) - 1; old_pos >= 0; old_pos--) {
        buf[new_pos] = backward_buf[old_pos];
        new_pos++;
    }
    buf[new_pos] = '\0';
}

// Goober5000
// there's probably a better way to do this, but this way works and is clear
// and short
int scan_fso_version_string (
    const char* text, int* major, int* minor, int* build, int* revis) {
    int val;

    val = sscanf (text, ";;FSO %i.%i.%i.%i;;", major, minor, build, revis);
    if (val == 4) return val;

    *revis = 0;
    val = sscanf (text, ";;FSO %i.%i.%i;;", major, minor, build);
    if (val == 3) return val;

    *build = *revis = 0;
    val = sscanf (text, ";;FSO %i.%i;;", major, minor);
    if (val == 2) return val;

    *minor = *build = *revis = 0;
    val = sscanf (text, ";;FSO %i;;", major);
    if (val == 1) return val;

    *major = *minor = *build = *revis = 0;
    return 0;
}

// Goober5000 - used for long Warnings, Errors, and FRED error messages with
// SEXPs
void truncate_message_lines (std::string& text, int num_allowed_lines) {
    ASSERT (num_allowed_lines > 0);
    size_t find_from = 0;

    while (find_from < text.size ()) {
        if (num_allowed_lines <= 0) {
            text.resize (find_from);
            text.append ("[...]");
            break;
        }

        size_t pos = text.find ('\n', find_from);
        if (pos == std::string::npos) break;

        num_allowed_lines--;
        find_from = pos + 1;
    }
}

// Goober5000 - ugh, I can't see why they didn't just use stuff_*_list for
// these; the only differece is the lack of parentheses

// from aicode.cpp
// Stuff a list of floats at *plist.
void parse_float_list (float* plist, int size) {
    int i;

    for (i = 0; i < size; i++) { stuff_float (&plist[i]); }
}

// from aicode.cpp
// Stuff a list of ints at *plist.
void parse_int_list (int* ilist, int size) {
    int i;

    for (i = 0; i < size; i++) { stuff_int (&ilist[i]); }
}

// parse a modular table of type "name_check" and parse it using the specified
// function callback
int parse_modular_table (
    const char* name_check, void (*parse_callback) (const char* filename),
    int path_type, int sort_type) {
    std::vector< std::string > tbl_file_names;
    int i, num_files = 0;

    if ((name_check == NULL) || (parse_callback == NULL) ||
        ((*name_check) != '*')) {
        ASSERT (0);
        return 0;
    }

    num_files =
        cf_get_file_list (tbl_file_names, path_type, name_check, sort_type);

    Parsing_modular_table = true;

    for (i = 0; i < num_files; i++) {
        tbl_file_names[i] += ".tbm";
        WARNINGF (
            LOCATION, "TBM  =>  Starting parse of '%s' ...\n",
            tbl_file_names[i].c_str ());
        (*parse_callback) (tbl_file_names[i].c_str ());
    }

    Parsing_modular_table = false;

    return num_files;
}