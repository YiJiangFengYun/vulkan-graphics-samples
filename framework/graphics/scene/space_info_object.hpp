#ifndef VG_SPACE_TRANSFORM_TYPE_INFO_H
#define VG_SPACE_TRANSFORM_TYPE_INFO_H

#include "graphics/scene/space_info.hpp"
#include "graphics/scene/transform_2.hpp"
#include "graphics/scene/transform_3.hpp"

namespace vg
{
	template<SpaceType type>
	struct SpaceTransformTypeInfo
	{
		typedef void TransformType;
	};

	template<>
	struct SpaceTransformTypeInfo<SpaceType::SPACE_2>
	{
		typedef Transform2 TransformType;
	};

	template<>
	struct SpaceTransformTypeInfo<SpaceType::SPACE_3>
	{
		typedef Transform3 TransformType;
	};
} //namespace kgs

#endif // !VG_SPACE_TRANSFORM_TYPE_INFO_H
