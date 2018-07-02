#include "graphics/light/light_point_3.hpp"

namespace vg
{
    const uint32_t LightPoint3::DEFAULT_DEPTH_TEXTURE_WIDTH = 1280u;
    const uint32_t LightPoint3::DEFAULT_DEPTH_TEXTURE_HEIGHT = 720u;
    const float LightPoint3::DEFAULT_RADIUS = 100.0f;
    LightPoint3::LightPoint3()
        : LightPoint3(DEFAULT_RADIUS
            , DEFAULT_DEPTH_TEXTURE_WIDTH
            , DEFAULT_DEPTH_TEXTURE_HEIGHT
            )
    {
        
    }

    LightPoint3::LightPoint3(float radius
        , uint32_t depthTextureWidth
        , uint32_t depthTextureHeight
        )
        : Light3()
        , m_cubeTargets(depthTextureWidth
            , depthTextureHeight
            )
        , m_pProjectors()
        , m_refProjectors()
    {
        uint32_t size = static_cast<uint32_t>(m_pProjectors.size());
        for (uint32_t i = 0; i < size; ++i)
        {
            m_pProjectors[i] = std::shared_ptr<Projector3>{new Projector3()};
        }
        //six projector is: x(+-), y(+-) and z(+-).
        uint32_t index = 0u;
        for (auto &pProjector : m_pProjectors)
        {
            pProjector->updateProj(glm::radians(90.0f), 1.0f, std::min(0.001f, radius), radius);
            Matrix4x4 transform(1.0f);
            if (m_space.rightHand == VG_TRUE)
            {
                switch (index)
                {
                    case static_cast<uint32_t>(CubemapFace::POSITIVE_X): //+x
                    {
                        transform = glm::rotate(transform, glm::radians(-90.0f), Vector3(0.0f, 0.0f, 1.0f));
                        break;
                    }
                    case static_cast<uint32_t>(CubemapFace::NEGATIVE_X): //-x
                    {
                        transform = glm::rotate(transform, glm::radians(90.0f), Vector3(0.0f, 0.0f, 1.0f));
                        break;
                    }
                    case static_cast<uint32_t>(CubemapFace::POSITIVE_Y): //+y
                    {
                        //Don't need to transform.
                        break;
                    }
                    case static_cast<uint32_t>(CubemapFace::NEGATIVE_Y): //-y
                    {
                        transform = glm::rotate(transform, glm::radians(180.0f), Vector3(0.0f, 0.0f, 1.0f));
                        break;
                    }
                    case static_cast<uint32_t>(CubemapFace::POSITIVE_Z): //+z
                    {
                        transform = glm::rotate(transform, glm::radians(90.0f), Vector3(1.0f, 0.0f, 0.0f));
                        break;
                    }
                    case static_cast<uint32_t>(CubemapFace::NEGATIVE_Z): //-z
                    {
                        transform = glm::rotate(transform, glm::radians(-90.0f), Vector3(1.0f, 0.0f, 0.0f));
                        break;
                    }
                }
            }
            else
            {
                switch (index)
                {
                    case static_cast<uint32_t>(CubemapFace::POSITIVE_X): //+x
                    {
                        transform = glm::rotate(transform, glm::radians(90.0f), Vector3(0.0f, 1.0f, 0.0f));
                        break;
                    }
                    case static_cast<uint32_t>(CubemapFace::NEGATIVE_X): //-x
                    {
                        transform = glm::rotate(transform, glm::radians(-90.0f), Vector3(0.0f, 1.0f, 0.0f));
                        break;
                    }
                    case static_cast<uint32_t>(CubemapFace::POSITIVE_Y): //+y
                    {
                        transform = glm::rotate(transform, glm::radians(-90.0f), Vector3(1.0f, 0.0f, 0.0f));
                        break;
                    }
                    case static_cast<uint32_t>(CubemapFace::NEGATIVE_Y): //-y
                    {
                        transform = glm::rotate(transform, glm::radians(90.0f), Vector3(1.0f, 0.0f, 0.0f));
                        break;
                    }
                    case static_cast<uint32_t>(CubemapFace::POSITIVE_Z): //+z
                    {
                        //Don't need to transform.
                        break;
                    }
                    case static_cast<uint32_t>(CubemapFace::NEGATIVE_Z): //-z
                    {
                        transform = glm::rotate(transform, glm::radians(180.0f), Vector3(0.0f, 1.0f, 0.0f));
                        break;
                    }
                }
            }
            pProjector->setTransformMatrix(transform);
            ++index;
        }

        for (uint32_t i = 0; i < size; ++i)
        {
            m_refProjectors[i] = m_pProjectors[i].get();
        }
    }

    LightDepthRenderInfo LightPoint3::getDepthRenderInfo() const
    {
        LightDepthRenderInfo info = {
            static_cast<uint32_t>(CubemapFace::RANGE_SIZE),
            reinterpret_cast<const BaseProjector *const *>(m_refProjectors.data()),
            reinterpret_cast<const PreDepthTarget *const *>(m_cubeTargets.getFaceTargets().data()),
        };
        return info;
    }
} //vg