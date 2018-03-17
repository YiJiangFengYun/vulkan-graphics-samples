#include "cubemaps/window.hpp"

#include <iostream>
#include <gli/gli.hpp>

Window::OtherInfo::OtherInfo()
    : lodBias(0.0f)
{

}

Window::OtherInfo::OtherInfo(float lodBias)
    : lodBias(lodBias)
{

}

Window::Window(uint32_t width
	, uint32_t height
	, const char* title
)
	: sampleslib::Window<vg::SpaceType::SPACE_3>(width
		, height
		, title
	    )
	, m_displaySkybox(true)
	, m_skyBoxObject()
	, m_objects()
	, m_objectIndex(0)
	, m_arrObjectNames()
	, m_pCubeMapTex()
	, m_pShaderSkybox()
	, m_pPassSkybox()
	, m_pMaterialSkybox()
	, m_pShaderReflect()
	, m_pPassReflect()
	, m_pMaterialReflect()
{
	_init();
	_createTexture();
	_createMaterial();
	_createModel();
	_initScene();
}

Window::Window(std::shared_ptr<GLFWwindow> pWindow
	, std::shared_ptr<vk::SurfaceKHR> pSurface
)
	: sampleslib::Window<vg::SpaceType::SPACE_3>(pWindow
		, pSurface
	    )
	, m_displaySkybox(true)
	, m_skyBoxObject()
	, m_objects()
	, m_objectIndex(0)
	, m_arrObjectNames()
	, m_pCubeMapTex()
	, m_pShaderSkybox()
	, m_pPassSkybox()
	, m_pMaterialSkybox()
	, m_pShaderReflect()
	, m_pPassReflect()
	, m_pMaterialReflect()
{
	_init();
	_createTexture();
	_createMaterial();
	_createModel();
	_initScene();
}

void Window::_init()
{
	m_zoom = -4.0f;
	m_rotationSpeed = 0.25f;
	/// Build a quaternion from euler angles (pitch, yaw, roll), in radians.
	m_rotation = vg::Vector3(glm::radians(-7.25f), glm::radians(-120.0f), glm::radians(0.0f));
}

void Window::_createTexture()
{
	//load texture
	auto &pApp = vg::pApp;
	auto deviceFeatures = pApp->getPhysicalDeviceFeatures();
	std::string fileName;
	vk::Format format;
	if (deviceFeatures.textureCompressionBC) 
	{
		fileName = "textures/cubemap_yokohama_bc3_unorm.ktx";
		format = vk::Format::eBc2UnormBlock;
	}
	else if (deviceFeatures.textureCompressionASTC_LDR)
	{
		fileName = "textures/cubemap_yokohama_astc_8x8_unorm.ktx";
		format = vk::Format::eAstc8x8UnormBlock;
	}
	else if (deviceFeatures.textureCompressionETC2)
	{
		fileName = "textures/cubemap_yokohama_etc2_unorm.ktx";
		format = vk::Format::eEtc2R8G8B8UnormBlock;
	}
	else
	{
		throw std::runtime_error("Device does not support any compressed texture format!");
	}

	gli::texture_cube gliTex2D(gli::load(fileName));
	if (gliTex2D.empty()) {
		throw std::runtime_error("The texture do't exist! path: " + fileName);
	}

	auto pTex = new vg::TextureCube(format, VG_TRUE, 
		gliTex2D[0].extent().x, 
		gliTex2D[0].extent().y
	);
	m_pCubeMapTex = std::shared_ptr<vg::TextureCube>(pTex);
	uint32_t mipLevels = static_cast<uint32_t>(gliTex2D.levels());
	uint32_t faces = static_cast<uint32_t>(gliTex2D.faces());
	uint32_t count = mipLevels * faces;
	vg::TextureDataLayout textureLayout;
	std::vector<vg::TextureDataLayout::Component> components(count);
	for (uint32_t face = 0; face < faces; ++face) {
		for (uint32_t level = 0; level < mipLevels; ++level) {
			uint32_t index = face * mipLevels + level;
			components[index].mipLevel = level;
		    components[index].baseArrayLayer = face;
		    components[index].layerCount = 1u;
		    components[index].size = gliTex2D[face][level].size();
		    components[index].hasImageExtent = VG_TRUE;
		    components[index].width = gliTex2D[face][level].extent().x;
		    components[index].height = gliTex2D[face][level].extent().y;
		    components[index].depth = 1u;
		}
	}
	textureLayout.componentCount = components.size();
	textureLayout.pComponent = components.data();
	m_pCubeMapTex->applyData(textureLayout, gliTex2D.data(), gliTex2D.size());

	m_pCubeMapTex->setFilterMode(vg::FilterMode::TRILINEAR);
	m_pCubeMapTex->setSamplerAddressMode(vg::SamplerAddressMode::REPEAT);

	auto pDevice = pApp->getDevice();
	auto pPhysicalDevice = pApp->getPhysicalDevice();
	if (pApp->getPhysicalDeviceFeatures().samplerAnisotropy)
	{
		auto anisotropy = pPhysicalDevice->getProperties().limits.maxSamplerAnisotropy;
		m_pCubeMapTex->setAnisotropy(anisotropy);
	}
}

void Window::_createMaterial()
{
	auto & pApp = vg::pApp;
    {
        auto & pShader = m_pShaderSkybox;
	    auto & pPass = m_pPassSkybox;
	    auto & pMaterial = m_pMaterialSkybox;
	    //shader
	    pShader = std::shared_ptr<vg::Shader>(
	    	new vg::Shader("shaders/cubemaps/skybox.vert.spv", "shaders/cubemaps/skybox.frag.spv")
	    	// new vg::Shader("shaders/test.vert.spv", "shaders/test.frag.spv")
	    	);
	    //pass
	    pPass = std::shared_ptr<vg::Pass>(new vg::Pass(pShader.get()));
	    pPass->setCullMode(vg::CullModeFlagBits::FRONT);
	    pPass->setFrontFace(vg::FrontFaceType::COUNTER_CLOCKWISE);
	    pPass->setMainTexture(m_pCubeMapTex.get());
	    pPass->apply();
	    //material
	    pMaterial = std::shared_ptr<vg::Material>(new vg::Material());
	    pMaterial->addPass(pPass.get());
	    pMaterial->setRenderPriority(0u);
	    pMaterial->setRenderQueueType(vg::MaterialShowType::OPAQUE);
	    pMaterial->apply();
	}
	
	{
		auto & pShader = m_pShaderReflect;
	    auto & pPass = m_pPassReflect;
	    auto & pMaterial = m_pMaterialReflect;
	    //shader
	    pShader = std::shared_ptr<vg::Shader>(
	    	new vg::Shader("shaders/cubemaps/reflect.vert.spv", "shaders/cubemaps/reflect.frag.spv")
	    	// new vg::Shader("shaders/test.vert.spv", "shaders/test.frag.spv")
	    	);
	    //pass
	    pPass = std::shared_ptr<vg::Pass>(new vg::Pass(pShader.get()));
	    pPass->setCullMode(vg::CullModeFlagBits::BACK);
	    pPass->setFrontFace(vg::FrontFaceType::COUNTER_CLOCKWISE);
		vk::PipelineDepthStencilStateCreateInfo depthStencilState = {};
	    depthStencilState.depthTestEnable = VG_TRUE;
	    depthStencilState.depthWriteEnable = VG_TRUE;
	    depthStencilState.depthCompareOp = vk::CompareOp::eLessOrEqual;
	    pPass->setDepthStencilInfo(depthStencilState);
	    pPass->setMainTexture(m_pCubeMapTex.get());
	    pPass->setDataValue("other_info", m_otherInfo, 2u);	
	    pPass->apply();
	    //material
	    pMaterial = std::shared_ptr<vg::Material>(new vg::Material());
	    pMaterial->addPass(pPass.get());
	    pMaterial->setRenderPriority(0u);
	    pMaterial->setRenderQueueType(vg::MaterialShowType::OPAQUE);
	    pMaterial->apply();
	}
}

void Window::_createModel()
{
	const uint32_t layoutCount = 3u;
	sampleslib::AssimpScene::VertexLayoutComponent layouts[layoutCount] = {
		sampleslib::AssimpScene::VertexLayoutComponent::VERTEX_COMPONENT_POSITION,
		sampleslib::AssimpScene::VertexLayoutComponent::VERTEX_COMPONENT_NORMAL,
		sampleslib::AssimpScene::VertexLayoutComponent::VERTEX_COMPONENT_UV,
	};
	//Sky box
	sampleslib::AssimpScene::CreateInfo createInfo;
	createInfo.fileName = "models/cube.obj";
	createInfo.isCreateObject = VG_TRUE;
	createInfo.layoutComponentCount = layoutCount;
	createInfo.pLayoutComponent = layouts;
	createInfo.offset = vg::Vector3(0.0f, 0.0f, 0.0f);
	createInfo.scale = vg::Vector3(0.05f);
	m_skyBoxObject.init(createInfo);

	//Other objects.
	std::vector<std::string> fileNames = { "models/sphere.obj", "models/teapot.dae", "models/torusknot.obj", "models/venus.fbx" };
	m_arrObjectNames = { "Sphere", "Teapot", "Torusknot", "Venus" };
	m_objects.resize(fileNames.size());
	size_t count = fileNames.size();
	for (size_t i = 0; i < count; ++i)
	{
		createInfo.fileName = fileNames[i].c_str();
		createInfo.scale = vg::Vector3(0.05f * (fileNames[i] == "models/venus.fbx" ? 3.0f : 1.0f));
		m_objects[i].init(createInfo);
	}
}

void Window::_initScene()
{
	{
		auto &objects = m_skyBoxObject.getObjects();
		for (auto &object : objects)
		{
			object->setMaterial(m_pMaterialSkybox.get());
			m_pScene->addVisualObject(object.get());
		}
	}
	{
		auto &objectScene = m_objects[m_objectIndex];
		auto &objects = objectScene.getObjects();
		for (auto &object : objects)
		{
			object->setMaterial(m_pMaterialReflect.get());
			m_pScene->addVisualObject(object.get());
		}
	}
}

void Window::_onUpdate()
{
	ParentWindowType::_onUpdate();

	auto pos = m_lastWinPos;
	auto size = m_lastWinSize;
	ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y + size.y + 10));
	ImGui::SetNextWindowSize(ImVec2(0, 0));
	ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	if (ImGui::SliderFloat("LOD bias", &m_otherInfo.lodBias, 0.0f, (float)m_pCubeMapTex->getMipmapLevels())) {
		m_pPassReflect->setDataValue("other_info", m_otherInfo, 2u);
		m_pPassReflect->apply();
	}
	ImGui::End();
}

void Window::_renderWithRenderer(vg::Renderer *pRenderer
		    , const vg::Renderer::RenderInfo &info
			, vg::Renderer::RenderResultInfo &resultInfo)
{

	ParentWindowType::_renderWithRenderer(pRenderer, info, resultInfo);	
}