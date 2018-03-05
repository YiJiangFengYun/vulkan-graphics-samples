#ifndef WINDOW_H
#define WINDOW_H

#include "sampleslib/window.hpp"

#define SCENE_COUNT 3u
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
	std::vector<vg::Vector3> m_tempPositions;
	std::vector<vg::Color32> m_tempColors;
	std::vector<uint32_t> m_tempIndices;
	std::shared_ptr<vg::VisualObject3> m_pModel;
	std::shared_ptr<vg::DimSepMesh3> m_pMesh;
	std::shared_ptr<vg::Material> m_pMaterials[SCENE_COUNT];
	std::shared_ptr<vg::Pass> m_pPasses[SCENE_COUNT];
	std::shared_ptr<vg::Shader> m_pShaders[SCENE_COUNT];
	void _init();
	void _loadModel();
	void _createMesh();
	void _createMaterial();
	void _createModel();
	virtual void _onUpdate() override;
	virtual void _renderWithRenderer(const std::shared_ptr<vg::Renderer> &pRenderer
		    , const vg::Renderer::RenderInfo &info
			, vg::Renderer::RenderResultInfo &resultInfo) override;
};

#endif // !WINDOW_
