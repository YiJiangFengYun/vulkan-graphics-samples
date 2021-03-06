#ifndef VG_LIGHT_3_H
#define VG_LIGHT_3_H

#include "graphics/scene/space_info.hpp"
#include "graphics/scene/light.hpp"

namespace vg
{
    template <uint32_t DATA_SIZE, uint32_t TEXTURE_COUNT>
    class Light3 : public Light<SpaceType::SPACE_3, DATA_SIZE, TEXTURE_COUNT>
    {
    public:
        Light3();
    };

} //namespace kgs

#include "graphics/scene/light_3.inl"

#endif // !VG_LIGHT_3_H
