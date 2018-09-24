// -*- mode: c++; -*-

#ifndef FREESPACE2_GRAPHICS_UNIFORMS_H
#define FREESPACE2_GRAPHICS_UNIFORMS_H

#include "graphics/util/uniform_structs.h"
#include "material.h"

/**
 * @file
 *
 * @brief Contains procedures for converting rendering data into their GPU
 * uniform representations
 */

namespace graphics {
namespace uniforms {

void convert_model_material (
    model_uniform_data* data_out, const model_material& material,
    const matrix4& model_transform, const vec3d& scale,
    size_t transform_buffer_offset);
}
} // namespace graphics

#endif // FREESPACE2_GRAPHICS_UNIFORMS_H
