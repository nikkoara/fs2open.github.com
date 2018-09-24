// -*- mode: c++; -*-

#ifndef FREESPACE2_SCRIPTING_API_LIBS_CFILE_H
#define FREESPACE2_SCRIPTING_API_LIBS_CFILE_H

#include "scripting/ade_api.h"

namespace scripting {
namespace api {

DECLARE_ADE_LIB (l_CFile);

int l_cf_get_path_id (const char* n_path);

} // namespace api
} // namespace scripting

#endif // FREESPACE2_SCRIPTING_API_LIBS_CFILE_H
