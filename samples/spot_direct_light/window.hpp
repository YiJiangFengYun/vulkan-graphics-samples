#ifndef WINDOW_H
#define WINDOW_H

#include "sampleslib/window.hpp"
#include "sampleslib/scene_assimp.hpp"

#define MAX_LIGHT_COUNT 2

#define MIN_LIGHT_RANGE 10.0f
#define MAX_LIGHT_RANGE 200.0f
#define DEFAULT_LIGHT_RANGE 100.0f

class Window : public sampleslib::Window<vg::SpaceType::SPACE_3>
{
public:
    using ParentWindowType = sampleslib::Window<vg::SpaceType::SPACE_3>;
    
    Window(uint32_t width
        , uint32_t height
        , const char* title
    );
    Window(std::shared_ptr<GLFWwindow> pWindow
        , std::shared_ptr<vk::SurfaceKHR> pSurface
    );
private:
    sampleslib::AssimpScene m_assimpScene;
    std::shared_ptr<vg::Material> m_pMaterial;

    float m_lightRange;
    vg::Vector3 m_lightStrength;
    std::shared_ptr<vg::LightSpot3> m_pSpotLight;
    std::shared_ptr<vg::LightDirect3> m_pDirectLight;
    vg::Bool32 m_useDirectLight;
    std::shared_ptr<vg::LightAmbient3> m_pAmbientLight;
    vg::Vector3 m_ambientLightStrength;

    virtual void _init() override;
    virtual void _initState() override;
    void _createModel();
    void _createLights();
    void _createMaterial();
    void _initScene();
    void _enableLighting();
    void _enableShadow();
    void _updateLights();

    virtual void _onPostReCreateSwapchain() override;
    virtual void _onUpdate() override;

};

#endif // !WINDOW_
