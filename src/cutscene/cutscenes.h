// -*- mode: c++; -*-

#ifndef _FREESPACE_CUTSCENES_SCREEN_HEADER_FILE
#define _FREESPACE_CUTSCENES_SCREEN_HEADER_FILE

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"

typedef struct cutscene_info {
    char filename[MAX_FILENAME_LEN];
    char name[NAME_LENGTH];
    char* description;
    bool viewable;
} cutscene_info;

extern std::vector< cutscene_info > Cutscenes;

// initializa table data
void cutscene_init ();

void cutscenes_screen_init ();

void cutscenes_screen_close ();

void cutscenes_screen_do_frame ();

void cutscene_mark_viewable (const char* filename);

#endif
