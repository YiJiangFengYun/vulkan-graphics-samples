#ifndef VG_TRANSFORM_3_H
#define VG_TRANSFORM_3_H

#include "graphics/scene/transform.hpp"

namespace vg
{
    class Transform3 : public Transform<SpaceType::SPACE_3>
    {
    public:
        Transform3();
        void lookAt(const PointType& worldTarget, const VectorType& worldUp);
        void lookAt2(const PointType& worldEye, const PointType& worldTarget, const VectorType& worldUp);
        void rotateAround(const PointType& point, const VectorType& axis, const float& angle, const VectorType& scale);
    private:

    };
} //namespace kgs

#endif // !VG_TRANSFORM_3_H
