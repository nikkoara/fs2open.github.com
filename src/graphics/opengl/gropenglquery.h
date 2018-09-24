// -*- mode: c++; -*-

#ifndef FREESPACE2_GRAPHICS_OPENGL_GROPENGLQUERY_H
#define FREESPACE2_GRAPHICS_OPENGL_GROPENGLQUERY_H

#include "graphics/2d.h"

int gr_opengl_create_query_object ();

void gr_opengl_query_value (int obj, QueryType type);

bool gr_opengl_query_value_available (int obj);

std::uint64_t gr_opengl_get_query_value (int obj);

void gr_opengl_delete_query_object (int obj);

#endif // FREESPACE2_GRAPHICS_OPENGL_GROPENGLQUERY_H
