// -*- mode: c++; -*-

#ifndef FREESPACE2_GRAPHICS_OPENGL_GROPENGLSYNC_H
#define FREESPACE2_GRAPHICS_OPENGL_GROPENGLSYNC_H

#include "graphics/opengl/gropengl.h"

gr_sync gr_opengl_sync_fence ();
bool gr_opengl_sync_wait (gr_sync sync, uint64_t timeoutns);
void gr_opengl_sync_delete (gr_sync sync);

#endif // FREESPACE2_GRAPHICS_OPENGL_GROPENGLSYNC_H
