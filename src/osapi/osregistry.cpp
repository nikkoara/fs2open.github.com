// -*- mode: c++; -*-

#include "globalincs/pstypes.h"
#include "osapi/osregistry.h"
#include "osapi/osapi.h"
#include "cmdline/cmdline.h"

namespace {

char szCompanyName[128] = "Volition";
char szAppName[128] = "FreeSpace2";

int Os_reg_inited = 0;

} // namespace

// Initialize path of old pilot files
const char* Osreg_user_dir_legacy = ".fs2_open";

const char* Osreg_company_name = "Volition";
const char* Osreg_class_name = "FreeSpace2Class";

const char* Osreg_app_name = "FreeSpace2";
const char* Osreg_title = "FreeSpace 2";

const char* Osreg_config_file_name = "fs2_open.ini";

#define DEFAULT_SECTION "Default"

typedef struct KeyValue {
    char* key;
    char* value;

    struct KeyValue* next;
} KeyValue;

typedef struct Section {
    char* name;

    struct KeyValue* pairs;
    struct Section* next;
} Section;

typedef struct Profile {
    struct Section* sections;
} Profile;

// For string config functions
static char tmp_string_data[1024];

// This code is needed for compatibility with the old windows registry

static char* read_line_from_file (FILE* fp) {
    char *buf, *buf_start;
    int buflen, eol;

    buflen = 80;
    buf = (char*)vm_malloc (buflen);
    buf_start = buf;
    eol = 0;

    do {
        if (buf == NULL) { return NULL; }

        if (fgets (buf_start, 80, fp) == NULL) {
            if (buf_start == buf) {
                vm_free (buf);
                return NULL;
            }
            else {
                *buf_start = 0;
                return buf;
            }
        }

        auto len = strlen (buf_start);

        if (buf_start[len - 1] == '\n') {
            buf_start[len - 1] = 0;
            eol = 1;
        }
        else {
            buflen += 80;

            buf = (char*)vm_realloc (buf, buflen);

            /* be sure to skip over the proper amount of nulls */
            buf_start = buf + (buflen - 80) - (buflen / 80) + 1;
        }
    } while (!eol);

    return buf;
}

static char* trim_string (char* str) {
    char* ptr;

    if (str == NULL) return NULL;

    /* kill any comment */
    ptr = strchr (str, ';');
    if (ptr) *ptr = 0;
    ptr = strchr (str, '#');
    if (ptr) *ptr = 0;

    ptr = str;
    auto len = strlen (str);
    if (len > 0) { ptr += len - 1; }

    while ((ptr > str) && isspace (*ptr)) { ptr--; }

    if (*ptr) {
        ptr++;
        *ptr = 0;
    }

    ptr = str;
    while (*ptr && isspace (*ptr)) { ptr++; }

    return ptr;
}

static Profile* profile_read (const char* file) {
    FILE* fp = NULL;
    char* str;

    if (os_is_legacy_mode ()) {
        // Try to use the config file at the old location
        char legacy_path[MAX_PATH_LEN];
        snprintf (
            legacy_path, MAX_PATH_LEN, "%s/%s/%s", getenv ("HOME"),
            Osreg_user_dir_legacy, file);

        fp = fopen (legacy_path, "rt");
    }
    else {
        fp = fopen (os_get_config_path (file).c_str (), "rt");
    }

    if (fp == NULL) return NULL;

    Profile* profile = (Profile*)vm_malloc (sizeof (Profile));
    profile->sections = NULL;

    Section** sp_ptr = &(profile->sections);
    Section* sp = NULL;

    KeyValue** kvp_ptr = NULL;

    while ((str = read_line_from_file (fp)) != NULL) {
        char* ptr = trim_string (str);

        if (*ptr == '[') {
            ptr++;

            char* pend = strchr (ptr, ']');
            if (pend != NULL) {
                // if (pend[1]) { /* trailing garbage! */ }

                *pend = 0;

                if (*ptr) {
                    sp = (Section*)vm_malloc (sizeof (Section));
                    sp->next = NULL;

                    sp->name = vm_strdup (ptr);
                    sp->pairs = NULL;

                    *sp_ptr = sp;
                    sp_ptr = &(sp->next);

                    kvp_ptr = &(sp->pairs);
                } // else { /* null name! */ }
            }     // else { /* incomplete section name! */ }
        }
        else {
            if (*ptr) {
                char* key = ptr;
                char* value = NULL;

                ptr = strchr (ptr, '=');
                if (ptr != NULL) {
                    *ptr = 0;
                    ptr++;

                    value = ptr;
                } // else { /* random garbage! */ }

                if (key && *key && value /* && *value */) {
                    if (sp != NULL) {
                        KeyValue* kvp =
                            (KeyValue*)vm_malloc (sizeof (KeyValue));

                        kvp->key = vm_strdup (key);
                        kvp->value = vm_strdup (value);

                        kvp->next = NULL;

                        *kvp_ptr = kvp;
                        kvp_ptr = &(kvp->next);
                    } // else { /* key/value with no section! */
                }     // else { /* malformed key/value entry! */ }
            }         // else it's just a comment or empty string
        }

        vm_free (str);
    }

    fclose (fp);

    return profile;
}

static void profile_free (Profile* profile) {
    if (profile == NULL) return;

    Section* sp = profile->sections;
    while (sp != NULL) {
        Section* st = sp;
        KeyValue* kvp = sp->pairs;

        while (kvp != NULL) {
            KeyValue* kvt = kvp;

            vm_free (kvp->key);
            vm_free (kvp->value);

            kvp = kvp->next;
            vm_free (kvt);
        }

        vm_free (sp->name);

        sp = sp->next;
        vm_free (st);
    }

    vm_free (profile);
}

static Profile* profile_update (
    Profile* profile, const char* section, const char* key,
    const char* value) {
    if (profile == NULL) {
        profile = (Profile*)vm_malloc (sizeof (Profile));

        profile->sections = NULL;
    }

    KeyValue* kvp;

    Section** sp_ptr = &(profile->sections);
    Section* sp = profile->sections;

    while (sp != NULL) {
        if (strcmp (section, sp->name) == 0) {
            KeyValue** kvp_ptr = &(sp->pairs);
            kvp = sp->pairs;

            while (kvp != NULL) {
                if (strcmp (key, kvp->key) == 0) {
                    vm_free (kvp->value);

                    if (value == NULL) {
                        *kvp_ptr = kvp->next;

                        vm_free (kvp->key);
                        vm_free (kvp);
                    }
                    else {
                        kvp->value = vm_strdup (value);
                    }

                    /* all done */
                    return profile;
                }

                kvp_ptr = &(kvp->next);
                kvp = kvp->next;
            }

            if (value != NULL) {
                /* key not found */
                kvp = (KeyValue*)vm_malloc (sizeof (KeyValue));
                kvp->next = NULL;
                kvp->key = vm_strdup (key);
                kvp->value = vm_strdup (value);
            }

            *kvp_ptr = kvp;

            /* all done */
            return profile;
        }

        sp_ptr = &(sp->next);
        sp = sp->next;
    }

    /* section not found */
    sp = (Section*)vm_malloc (sizeof (Section));
    sp->next = NULL;
    sp->name = vm_strdup (section);

    kvp = (KeyValue*)vm_malloc (sizeof (KeyValue));
    kvp->next = NULL;
    kvp->key = vm_strdup (key);
    kvp->value = vm_strdup (value);

    sp->pairs = kvp;

    *sp_ptr = sp;

    return profile;
}

static char*
profile_get_value (Profile* profile, const char* section, const char* key) {
    if (profile == NULL) return NULL;

    Section* sp = profile->sections;

    while (sp != NULL) {
        if (strcasecmp (section, sp->name) == 0) {
            KeyValue* kvp = sp->pairs;

            while (kvp != NULL) {
                if (strcmp (key, kvp->key) == 0) { return kvp->value; }
                kvp = kvp->next;
            }
        }

        sp = sp->next;
    }

    /* not found */
    return NULL;
}

static void profile_save (Profile* profile, const char* file) {
    FILE* fp = NULL;
    char tmp[PATH_MAX] = "";
    char tmp2[PATH_MAX] = "";

    if (profile == NULL) return;

    fp = fopen (os_get_config_path (file).c_str (), "wt");

    if (fp == NULL) return;

    Section* sp = profile->sections;

    while (sp != NULL) {
        sprintf (tmp, NOX ("[%s]\n"), sp->name);
        fputs (tmp, fp);

        KeyValue* kvp = sp->pairs;
        while (kvp != NULL) {
            sprintf (tmp2, NOX ("%s=%s\n"), kvp->key, kvp->value);
            fputs (tmp2, fp);
            kvp = kvp->next;
        }

        fprintf (fp, "\n");

        sp = sp->next;
    }

    fclose (fp);
}

// os registry functions
// -------------------------------------------------------------

// initialize the registry. setup default keys to use
void os_init_registry_stuff (const char* company, const char* app) {
    if (company) { strcpy_s (szCompanyName, company); }
    else {
        strcpy_s (szCompanyName, Osreg_company_name);
    }

    if (app) { strcpy_s (szAppName, app); }
    else {
        strcpy_s (szAppName, Osreg_app_name);
    }

    Os_reg_inited = 1;
}

const char* os_config_read_string (
    const char* section, const char* name, const char* default_value) {
    Profile* p = profile_read (Osreg_config_file_name);

    WARNINGF (
        LOCATION,
        "os_config_read_string(): section = \"%s\", name = \"%s\", default "
        "value: \"%s\"\n",
        (section) ? section : DEFAULT_SECTION, name,
        (default_value) ? default_value : NOX ("NULL"));

    if (section == NULL) section = DEFAULT_SECTION;

    char* ptr = profile_get_value (p, section, name);

    if (ptr != NULL) {
        strncpy (tmp_string_data, ptr, 1023);
        default_value = tmp_string_data;
    }

    profile_free (p);

    return default_value;
}

unsigned int os_config_read_uint (
    const char* section, const char* name, unsigned int default_value) {
    Profile* p = profile_read (Osreg_config_file_name);

    if (section == NULL) section = DEFAULT_SECTION;

    char* ptr = profile_get_value (p, section, name);

    if (ptr != NULL) { default_value = atoi (ptr); }

    profile_free (p);

    return default_value;
}

void os_config_write_string (
    const char* section, const char* name, const char* value) {
    Profile* p = profile_read (Osreg_config_file_name);

    if (section == NULL) section = DEFAULT_SECTION;

    p = profile_update (p, section, name, value);
    profile_save (p, Osreg_config_file_name);
    profile_free (p);
}

void os_config_write_uint (
    const char* section, const char* name, unsigned int value) {
    Profile* p = profile_read (Osreg_config_file_name);

    if (section == NULL) section = DEFAULT_SECTION;

    char buf[21];

    snprintf (buf, 20, "%u", value);

    p = profile_update (p, section, name, buf);
    profile_save (p, Osreg_config_file_name);
    profile_free (p);
}