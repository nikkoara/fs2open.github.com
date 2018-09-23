/*
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "globalincs/pstypes.h"
#include "resource/resource.hpp"

#include <iterator>
#include <mutex>

struct def_file {
    const char* path_type;
    const char* filename;
    const void* contents;
    const size_t size;
};

#include "resource/effects/batched-f.sdr.cpp"
#include "resource/effects/batched-v.sdr.cpp"
#include "resource/effects/bloom-comp-f.sdr.cpp"
#include "resource/effects/blur-f.sdr.cpp"
#include "resource/effects/brightpass-f.sdr.cpp"
#include "resource/effects/decal-f.sdr.cpp"
#include "resource/effects/decal-v.sdr.cpp"
#include "resource/effects/default-material-f.sdr.cpp"
#include "resource/effects/deferred-clear-f.sdr.cpp"
#include "resource/effects/deferred-clear-v.sdr.cpp"
#include "resource/effects/deferred-f.sdr.cpp"
#include "resource/effects/deferred-v.sdr.cpp"
#include "resource/effects/effect-distort-f.sdr.cpp"
#include "resource/effects/effect-distort-v.sdr.cpp"
#include "resource/effects/effect-particle-f.sdr.cpp"
#include "resource/effects/effect-screen-g.sdr.cpp"
#include "resource/effects/effect-v.sdr.cpp"
#include "resource/effects/fog-f.sdr.cpp"
#include "resource/effects/fxaa-f.sdr.cpp"
#include "resource/effects/fxaa-v.sdr.cpp"
#include "resource/effects/fxaapre-f.sdr.cpp"
#include "resource/effects/gamma.sdr.cpp"
#include "resource/effects/lighting.sdr.cpp"
#include "resource/effects/ls-f.sdr.cpp"
#include "resource/effects/main-f.sdr.cpp"
#include "resource/effects/main-g.sdr.cpp"
#include "resource/effects/main-v.sdr.cpp"
#include "resource/effects/nanovg-f.sdr.cpp"
#include "resource/effects/nanovg-v.sdr.cpp"
#include "resource/effects/normals.sdr.cpp"
#include "resource/effects/passthrough-f.sdr.cpp"
#include "resource/effects/passthrough-v.sdr.cpp"
#include "resource/effects/post-f.sdr.cpp"
#include "resource/effects/post-v.sdr.cpp"
#include "resource/effects/shadowdebug-f.sdr.cpp"
#include "resource/effects/shadowdebug-v.sdr.cpp"
#include "resource/effects/shadows.sdr.cpp"
#include "resource/effects/shield-impact-f.sdr.cpp"
#include "resource/effects/shield-impact-v.sdr.cpp"
#include "resource/effects/tonemapping-f.sdr.cpp"
#include "resource/effects/video-f.sdr.cpp"
#include "resource/effects/video-v.sdr.cpp"
#include "resource/scripts/cfile_require.lua.cpp"
#include "resource/maps/app_icon.png.cpp"
#include "resource/maps/app_icon_d.png.cpp"
#include "resource/maps/app_icon_sse.png.cpp"
#include "resource/maps/app_icon_sse_d.png.cpp"
#include "resource/tables/ai_profiles.tbl.cpp"
#include "resource/tables/autopilot.tbl.cpp"
#include "resource/tables/controlconfigdefaults.tbl.cpp"
#include "resource/tables/fonts.tbl.cpp"
#include "resource/tables/game_settings.tbl.cpp"
#include "resource/tables/iff_defs.tbl.cpp"
#include "resource/tables/objecttypes.tbl.cpp"
#include "resource/tables/post_processing.tbl.cpp"
#include "resource/tables/species_defs.tbl.cpp"

def_file
Default_files[] = {
    { "data/effects",              "batched-f.sdr",               resource_effects_batched_f_sdr_res,                 resource_effects_batched_f_sdr_res_len },
    { "data/effects",              "batched-v.sdr",               resource_effects_batched_v_sdr_res,                 resource_effects_batched_v_sdr_res_len },
    { "data/effects",           "bloom-comp-f.sdr",            resource_effects_bloom_comp_f_sdr_res,              resource_effects_bloom_comp_f_sdr_res_len },
    { "data/effects",                 "blur-f.sdr",                  resource_effects_blur_f_sdr_res,                    resource_effects_blur_f_sdr_res_len },
    { "data/effects",           "brightpass-f.sdr",            resource_effects_brightpass_f_sdr_res,              resource_effects_brightpass_f_sdr_res_len },
    { "data/effects",                "decal-f.sdr",                 resource_effects_decal_f_sdr_res,                   resource_effects_decal_f_sdr_res_len },
    { "data/effects",                "decal-v.sdr",                 resource_effects_decal_v_sdr_res,                   resource_effects_decal_v_sdr_res_len },
    { "data/effects",     "default-material-f.sdr",      resource_effects_default_material_f_sdr_res,        resource_effects_default_material_f_sdr_res_len },
    { "data/effects",       "deferred-clear-f.sdr",        resource_effects_deferred_clear_f_sdr_res,          resource_effects_deferred_clear_f_sdr_res_len },
    { "data/effects",       "deferred-clear-v.sdr",        resource_effects_deferred_clear_v_sdr_res,          resource_effects_deferred_clear_v_sdr_res_len },
    { "data/effects",             "deferred-f.sdr",              resource_effects_deferred_f_sdr_res,                resource_effects_deferred_f_sdr_res_len },
    { "data/effects",             "deferred-v.sdr",              resource_effects_deferred_v_sdr_res,                resource_effects_deferred_v_sdr_res_len },
    { "data/effects",       "effect-distort-f.sdr",        resource_effects_effect_distort_f_sdr_res,          resource_effects_effect_distort_f_sdr_res_len },
    { "data/effects",       "effect-distort-v.sdr",        resource_effects_effect_distort_v_sdr_res,          resource_effects_effect_distort_v_sdr_res_len },
    { "data/effects",      "effect-particle-f.sdr",       resource_effects_effect_particle_f_sdr_res,         resource_effects_effect_particle_f_sdr_res_len },
    { "data/effects",        "effect-screen-g.sdr",         resource_effects_effect_screen_g_sdr_res,           resource_effects_effect_screen_g_sdr_res_len },
    { "data/effects",               "effect-v.sdr",                resource_effects_effect_v_sdr_res,                  resource_effects_effect_v_sdr_res_len },
    { "data/effects",                  "fog-f.sdr",                   resource_effects_fog_f_sdr_res,                     resource_effects_fog_f_sdr_res_len },
    { "data/effects",                 "fxaa-f.sdr",                  resource_effects_fxaa_f_sdr_res,                    resource_effects_fxaa_f_sdr_res_len },
    { "data/effects",                 "fxaa-v.sdr",                  resource_effects_fxaa_v_sdr_res,                    resource_effects_fxaa_v_sdr_res_len },
    { "data/effects",              "fxaapre-f.sdr",               resource_effects_fxaapre_f_sdr_res,                 resource_effects_fxaapre_f_sdr_res_len },
    { "data/effects",                  "gamma.sdr",                   resource_effects_gamma_sdr_res,                     resource_effects_gamma_sdr_res_len },
    { "data/effects",               "lighting.sdr",                resource_effects_lighting_sdr_res,                  resource_effects_lighting_sdr_res_len },
    { "data/effects",                   "ls-f.sdr",                    resource_effects_ls_f_sdr_res,                      resource_effects_ls_f_sdr_res_len },
    { "data/effects",                 "main-f.sdr",                  resource_effects_main_f_sdr_res,                    resource_effects_main_f_sdr_res_len },
    { "data/effects",                 "main-g.sdr",                  resource_effects_main_g_sdr_res,                    resource_effects_main_g_sdr_res_len },
    { "data/effects",                 "main-v.sdr",                  resource_effects_main_v_sdr_res,                    resource_effects_main_v_sdr_res_len },
    { "data/effects",               "nanovg-f.sdr",                resource_effects_nanovg_f_sdr_res,                  resource_effects_nanovg_f_sdr_res_len },
    { "data/effects",               "nanovg-v.sdr",                resource_effects_nanovg_v_sdr_res,                  resource_effects_nanovg_v_sdr_res_len },
    { "data/effects",                "normals.sdr",                 resource_effects_normals_sdr_res,                   resource_effects_normals_sdr_res_len },
    { "data/effects",          "passthrough-f.sdr",           resource_effects_passthrough_f_sdr_res,             resource_effects_passthrough_f_sdr_res_len },
    { "data/effects",          "passthrough-v.sdr",           resource_effects_passthrough_v_sdr_res,             resource_effects_passthrough_v_sdr_res_len },
    { "data/effects",                 "post-f.sdr",                  resource_effects_post_f_sdr_res,                    resource_effects_post_f_sdr_res_len },
    { "data/effects",                 "post-v.sdr",                  resource_effects_post_v_sdr_res,                    resource_effects_post_v_sdr_res_len },
    { "data/effects",          "shadowdebug-f.sdr",           resource_effects_shadowdebug_f_sdr_res,             resource_effects_shadowdebug_f_sdr_res_len },
    { "data/effects",          "shadowdebug-v.sdr",           resource_effects_shadowdebug_v_sdr_res,             resource_effects_shadowdebug_v_sdr_res_len },
    { "data/effects",                "shadows.sdr",                 resource_effects_shadows_sdr_res,                   resource_effects_shadows_sdr_res_len },
    { "data/effects",        "shield-impact-f.sdr",         resource_effects_shield_impact_f_sdr_res,           resource_effects_shield_impact_f_sdr_res_len },
    { "data/effects",        "shield-impact-v.sdr",         resource_effects_shield_impact_v_sdr_res,           resource_effects_shield_impact_v_sdr_res_len },
    { "data/effects",          "tonemapping-f.sdr",           resource_effects_tonemapping_f_sdr_res,             resource_effects_tonemapping_f_sdr_res_len },
    { "data/effects",                "video-f.sdr",                 resource_effects_video_f_sdr_res,                   resource_effects_video_f_sdr_res_len },
    { "data/effects",                "video-v.sdr",                 resource_effects_video_v_sdr_res,                   resource_effects_video_v_sdr_res_len },
    { "data/maps"   ,               "app_icon.png",                   resource_maps_app_icon_png_res,                     resource_maps_app_icon_png_res_len },
    { "data/maps"   ,             "app_icon_d.png",                 resource_maps_app_icon_d_png_res,                   resource_maps_app_icon_d_png_res_len },
    { "data/maps"   ,           "app_icon_sse.png",               resource_maps_app_icon_sse_png_res,                 resource_maps_app_icon_sse_png_res_len },
    { "data/maps"   ,         "app_icon_sse_d.png",             resource_maps_app_icon_sse_d_png_res,               resource_maps_app_icon_sse_d_png_res_len },
    { "data/scripts",          "cfile_require.lua",           resource_scripts_cfile_require_lua_res,             resource_scripts_cfile_require_lua_res_len },
    { "data/tables" ,            "ai_profiles.tbl",              resource_tables_ai_profiles_tbl_res,                resource_tables_ai_profiles_tbl_res_len },
    { "data/tables" ,              "autopilot.tbl",                resource_tables_autopilot_tbl_res,                  resource_tables_autopilot_tbl_res_len },
    { "data/tables" ,  "controlconfigdefaults.tbl",    resource_tables_controlconfigdefaults_tbl_res,      resource_tables_controlconfigdefaults_tbl_res_len },
    { "data/tables" ,                  "fonts.tbl",                    resource_tables_fonts_tbl_res,                      resource_tables_fonts_tbl_res_len },
    { "data/tables" ,          "game_settings.tbl",            resource_tables_game_settings_tbl_res,              resource_tables_game_settings_tbl_res_len },
    { "data/tables" ,               "iff_defs.tbl",                 resource_tables_iff_defs_tbl_res,                   resource_tables_iff_defs_tbl_res_len },
    { "data/tables" ,            "objecttypes.tbl",              resource_tables_objecttypes_tbl_res,                resource_tables_objecttypes_tbl_res_len },
    { "data/tables" ,        "post_processing.tbl",          resource_tables_post_processing_tbl_res,            resource_tables_post_processing_tbl_res_len },
    { "data/tables" ,           "species_defs.tbl",             resource_tables_species_defs_tbl_res,               resource_tables_species_defs_tbl_res_len }
};

default_file
defaults_get_file (const char* filename) {
    for (const auto& f : Default_files) {
        if (!stricmp (f.filename, filename)) {
            return { f.path_type, f.filename, f.contents, f.size };
        }
    }

    Error (
        LOCATION,
        "Default table '%s' missing from executable - contact a coder.",
        filename);

    return { };
}

std::vector< default_file >
defaults_get_all () {
    std::vector< default_file > files;

    for (auto& file : Default_files) {
        files.emplace_back (
            file.path_type, file.filename, file.contents, file.size);
    }

    return files;
}
