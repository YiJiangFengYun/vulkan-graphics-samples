#include "specialization_constants/window.hpp"

#include <iostream>
#include <gli/gli.hpp>

Window::Window(uint32_t width
    , uint32_t height
    , const char* title
)
    : sampleslib::Window<vg::SpaceType::SPACE_3>(width
        , height
        , title
        )
    , m_assimpScene()
    , m_pMaterials()
    , m_pVisualObjects()
{
    _init();
}
Window::Window(std::shared_ptr<GLFWwindow> pWindow
    , std::shared_ptr<vk::SurfaceKHR> pSurface
)
    : sampleslib::Window<vg::SpaceType::SPACE_3>(pWindow
        , pSurface
        )
    , m_assimpScene()
    , m_pMaterials()
    , m_pVisualObjects()
{
    _init();
}

void Window::_init()
{
    ParentWindowType::_init();
    _loadAssimpScene();    
    _createTexture();
    _createMaterial();
    _createVisualObjects();
    _fillScene();
}

void Window::_initState()
{
    ParentWindowType::_initState();
    m_sceneCount = SCENE_COUNT;
    m_cameraAspect = (float)m_width / 3.0f / (float)m_height;
    m_cameraZoom = -0.0f;
    m_cameraPosition = vg::Vector3(0.0f, 1.0f, -1.0f);
    /// Build a quaternion from euler angles (pitch, yaw, roll), in radians.
    m_cameraRotation = vg::Vector3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f));

    m_lightInfo.lightPos = vg::Vector4(0.0f, 2.0f, -1.0f, 1.0f);
}


void Window::_loadAssimpScene()
{
    const uint32_t layoutCount = 4u;
    sampleslib::AssimpScene::VertexLayoutComponent layouts[layoutCount] = {
        sampleslib::AssimpScene::VertexLayoutComponent::VERTEX_COMPONENT_POSITION,
        sampleslib::AssimpScene::VertexLayoutComponent::VERTEX_COMPONENT_NORMAL,
        sampleslib::AssimpScene::VertexLayoutComponent::VERTEX_COMPONENT_UV,
        sampleslib::AssimpScene::VertexLayoutComponent::VERTEX_COMPONENT_COLOR                        
    };
    sampleslib::AssimpScene::CreateInfo createInfo;
    createInfo.fileName = "models/color_teapot_spheres.dae";
    createInfo.isCreateObject = VG_TRUE;
    createInfo.layoutComponentCount = layoutCount;
    createInfo.pLayoutComponent = layouts;
    createInfo.offset = vg::Vector3(0.0f, 0.0f, 0.0f);
    createInfo.scale = vg::Vector3(0.1f, 0.1f, 0.1f);
    createInfo.isCreateObject = VG_FALSE;
    m_assimpScene.init(createInfo);
}

void Window::_createTexture()
{
    //load texture
    std::string fileName = "textures/metalplate_nomips_rgba.ktx";
    auto format = vk::Format::eR8G8B8A8Unorm;
    gli::texture2d gliTex2D(gli::load(fileName));
    if (gliTex2D.empty()) {
        throw std::runtime_error("The texture do't exist! path: " + fileName);
    }

    auto pTex = new vg::Texture2D(format, VG_FALSE, 
        gliTex2D[0].extent().x, 
        gliTex2D[0].extent().y
    );
    m_pTexture = std::shared_ptr<vg::Texture2D>(pTex);
    uint32_t mipLevels = static_cast<uint32_t>(gliTex2D.levels());
    vg::TextureDataInfo textureLayout;
    std::vector<vg::TextureDataInfo::Component> components(mipLevels);
    for (uint32_t i = 0; i < mipLevels; ++i)
    {
        components[i].mipLevel = i;
        components[i].baseArrayLayer = 0u;
        components[i].layerCount = 1u;
        components[i].size = gliTex2D[i].size();
        components[i].hasImageExtent = VG_TRUE;
        components[i].width = gliTex2D[i].extent().x;
        components[i].height = gliTex2D[i].extent().y;
        components[i].depth = 1u;
    }
    textureLayout.componentCount = components.size();
    textureLayout.pComponent = components.data();
    m_pTexture->applyData(textureLayout, gliTex2D.data(), gliTex2D.size());

    auto &pApp = vg::pApp;
    auto pDevice = pApp->getDevice();
    auto pPhysicalDevice = pApp->getPhysicalDevice();
    vk::Bool32 enableAnisotropy = VK_FALSE;
    float anisotropy = 0.0f;
    if (pApp->getPhysicalDeviceFeatures().samplerAnisotropy)
    {
        enableAnisotropy = VK_TRUE;
        anisotropy = pPhysicalDevice->getProperties().limits.maxSamplerAnisotropy;
    }

    vg::Texture::SamplerCreateInfo info = {
        vk::SamplerCreateFlags(),
        vk::Filter::eLinear,
        vk::Filter::eLinear,
        vk::SamplerMipmapMode::eLinear,
        vk::SamplerAddressMode::eRepeat,
        vk::SamplerAddressMode::eRepeat,
        vk::SamplerAddressMode::eRepeat,
        0.0f,
        enableAnisotropy,
        anisotropy,
        0.0f,
        (float)(m_pTexture->getImage()->getInfo().mipLevels),
    };
    m_pTexture->createSampler("other_sampler", info);
}

void Window::_createMaterial()
{
    std::string vertShaderPaths = {
        "shaders/specialization_constants/uber.vert.spv",
    };

    std::string fragShaderPaths = {
        "shaders/specialization_constants/uber.frag.spv",
    };

    auto& pMaterials = m_pMaterials;
    auto& pApp = vg::pApp;
    for (uint32_t i = 0; i < SCENE_COUNT; ++i)
    {
        //material
        pMaterials[i] = std::shared_ptr<vg::Material>(new vg::Material());
        pMaterials[i]->setRenderPriority(0u);
        pMaterials[i]->setRenderQueueType(vg::MaterialShowType::OPAQUE);

        auto pShader = pMaterials[i]->getMainShader();
        auto pPass = pMaterials[i]->getMainPass();
        //shader
        pShader->load(vertShaderPaths, fragShaderPaths);
        //pass
        vg::Pass::BuildInDataInfo::Component buildInDataCmps[2] = {
            {vg::Pass::BuildInDataType::MATRIX_OBJECT_TO_NDC},
            {vg::Pass::BuildInDataType::MATRIX_OBJECT_TO_VIEW}
        };
        vg::Pass::BuildInDataInfo buildInDataInfo;
        buildInDataInfo.componentCount = 2u;
        buildInDataInfo.pComponent = buildInDataCmps;
        pPass->setBuildInDataInfo(buildInDataInfo);
        pPass->setCullMode(vk::CullModeFlagBits::eBack);
        pPass->setFrontFace(vk::FrontFace::eClockwise);
        pPass->setViewport(fd::Viewport(static_cast<float>(i) / static_cast<float>(SCENE_COUNT),
             0.0f,
             1.0f / static_cast<float>(SCENE_COUNT),
             1.0f));
        vk::PipelineDepthStencilStateCreateInfo depthStencilState = {};
        depthStencilState.depthTestEnable = VG_TRUE;
        depthStencilState.depthWriteEnable = VG_TRUE;
        depthStencilState.depthCompareOp = vk::CompareOp::eLessOrEqual;
        pPass->setDepthStencilInfo(depthStencilState);

        //!!!
        // Prepare specialization data

        // Host data to take specialization constants from
        struct SpecializationData {
            // Sets the lighting model used in the fragment "uber" shader
            uint32_t lightingModel;
            // Parameter for the toon shading part of the fragment shader
            float toonDesaturationFactor = 0.5f;
        } specializationData;

        specializationData.lightingModel = i;

        pPass->addSpecializationItem(vk::ShaderStageFlagBits::eFragment, "light_model", 0u, specializationData.lightingModel);
        pPass->addSpecializationItem(vk::ShaderStageFlagBits::eFragment, "toon_desaturation_factor", 1u, specializationData.toonDesaturationFactor);
        vg::PassTextureInfo::TextureInfo itemInfo = {
            m_pTexture.get(),
            nullptr,
            m_pTexture->getSampler("other_sampler"),
            vk::ImageLayout::eUndefined,
        };
        vg::PassTextureInfo mainTextureInfo = {
            vg::SamplerTextureType::TEX_2D,
            1u,
            &itemInfo,
            VG_PASS_OTHER_MIN_BINDING_PRIORITY,
            vg::ImageDescriptorType::COMBINED_IMAGE_SAMPLER,
            vk::ShaderStageFlagBits::eFragment,
        };
        pPass->addTexture("main_texture", mainTextureInfo);

        vg::PassDataInfo lightDataInfo = {
            VG_PASS_OTHER_DATA_MIN_LAYOUT_PRIORITY,
            vk::ShaderStageFlagBits::eVertex,
        };
        pPass->addData("light_info", lightDataInfo, m_lightInfo);
        pPass->apply();

        pMaterials[i]->apply();
    }

}

void Window::_createVisualObjects()
{
    const auto &meshes = m_assimpScene.getMeshes();
    if (meshes.size() > 1) throw std::runtime_error("Mesh count more than 1 in assimp scene.");
    for (uint32_t i = 0; i < SCENE_COUNT; ++i)
    {
        m_pVisualObjects[i] = std::shared_ptr<vg::VisualObject3>{new vg::VisualObject3()};
        m_pVisualObjects[i]->setMesh(meshes[0].get());
        auto pMaterial = m_pMaterials[i].get();
        m_pVisualObjects[i]->setMaterialCount(1u);
        m_pVisualObjects[i]->setMaterial(pMaterial);
    }
}

void Window::_fillScene()
{
    for (uint32_t i = 0; i < SCENE_COUNT; ++i)
    {
        m_pScenes[i]->addVisualObject(m_pVisualObjects[i].get());
    }
}

void Window::_onUpdate()
{
    ParentWindowType::_onUpdate();
}