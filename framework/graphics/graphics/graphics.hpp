#ifndef VG_GRAPHICS_H
#define VG_GRAPHICS_H

#include <graphics/global.hpp>

#include <graphics/app/app.hpp>

#include <graphics/buffer_data/buffer_data_option.hpp>
#include <graphics/buffer_data/vertex_data.hpp>
#include <graphics/buffer_data/index_data.hpp>
#include <graphics/buffer_data/uniform_buffer_data.hpp>

#include <graphics/texture/texture_1d.hpp>
#include <graphics/texture/texture_1d_array.hpp>
#include <graphics/texture/texture_2d.hpp>
#include <graphics/texture/texture_2d_array.hpp>
#include <graphics/texture/texture_3d.hpp>
#include <graphics/texture/texture_cube.hpp>
#include <graphics/texture/texture_cube_array.hpp>
#include <graphics/texture/texture_color_attachment.hpp>
#include <graphics/texture/texture_depth_stencil_attachment.hpp>
#include <graphics/texture/texture_default.hpp>

#include <graphics/mesh/mesh.hpp>
#include <graphics/mesh/mesh_2.hpp>
#include <graphics/mesh/mesh_3.hpp>

#include <graphics/pass/shader.hpp>
#include <graphics/pass/pass.hpp>
#include <graphics/pass/pass_default.hpp>

#include <graphics/material/material.hpp>
#include <graphics/material/material_default.hpp>

#include <graphics/post_render/post_render.hpp>

#include <graphics/renderer/renderer.hpp>
#include <graphics/renderer/renderer_target_surface.hpp>
#include <graphics/renderer/renderer_target_color_texture.hpp>

#include <graphics/scene/scene_2.hpp>
#include <graphics/scene/scene_3.hpp>
#include <graphics/scene/camera_3.hpp>
#include <graphics/scene/camera_op_2.hpp>
#include <graphics/scene/camera_op_3.hpp>
#include <graphics/scene/visual_object_2.hpp>
#include <graphics/scene/visual_object_3.hpp>

#include <graphics/light/light_point_3.hpp>
#include <graphics/light/light_ambient_3.hpp>
#include <graphics/light/light_direct_3.hpp>
#include <graphics/light/light_spot_3.hpp>

#include <graphics/util/queue_family.hpp>
#include <graphics/util/swapchain_info.hpp>
#include <graphics/util/util.hpp>
#include <graphics/util/gemo_util.hpp>

#include <graphics/module.hpp>


namespace vg
{

} //namespace kgs

#endif // !VG_GRAPHICS_H
