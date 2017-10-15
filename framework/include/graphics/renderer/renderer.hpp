#ifndef KGS_RENDERER_H
#define KGS_RENDERER_H

#include "graphics/scene/space_info.hpp"
#include "graphics/scene/scene.hpp"
#include "graphics/scene/camera.hpp"
#include "graphics/renderer/renderer_option.hpp"
#include "graphics/texture/texture_color_attachment.hpp"
#include "graphics/texture/texture_depth_stencil_attachment.hpp"

//todo: batch mesh,
//todo: cache graphics pipeline.

namespace kgs
{
	class BaseRenderer
	{
	public:
		static const vk::Format DEFAULT_DEPTH_STENCIL_FORMAT;

		BaseRenderer(std::shared_ptr<vk::ImageView> pSwapchainImageView
			, vk::Format swapchainImageFormat
			, uint32_t swapchainImageWidth
			, uint32_t swapchainImageHeight
		);

		BaseRenderer(std::shared_ptr<TextureColorAttachment> pColorAttachmentTex
		);

		void render(RenderInfo renderInfo);

	protected:
		//compositions
		uint32_t m_framebufferWidth;
		uint32_t m_framebufferHeight;
		vk::Format m_colorImageFormat;
		vk::Format m_depthStencilImageFormat;
		std::shared_ptr<vk::RenderPass> m_pRenderPass;
		std::shared_ptr<TextureDepthStencilAttachment> m_pDepthStencilTexture;
		std::shared_ptr<vk::Framebuffer> m_pFrameBuffer;
		std::shared_ptr<vk::CommandBuffer> m_pCommandBuffer;

		//aggregations
		//Renderer will use swapchain image when color attachment texture is null.
		std::shared_ptr<vk::ImageView> m_pSwapchainImageView;
		vk::Format m_swapchainImageFormat;
		//Renderer will render to color texture when it is not null.
		std::shared_ptr<TextureColorAttachment> m_pColorTexture;

		virtual void _render(RenderInfo renderInfo);

		void _createRenderPass();
		void _createFramebuffer();
		void _createCommandBuffer();

		void _createPipelineForRender(std::shared_ptr<vk::PipelineLayout> &pPipelineLayout, 
			std::shared_ptr<vk::Pipeline> &pPipeline,
			const RenderInfo& renderInfo, 
			std::shared_ptr<BaseMesh> pMesh,
			std::shared_ptr<Material> pMaterial,
			uint32_t subMeshIndex = 0u,
			uint32_t passIndex = 0u);
		void _recordCommandBufferForRender(const RenderInfo& renderInfo, 
			std::shared_ptr<vk::PipelineLayout> pPipelineLayout,
			std::shared_ptr<vk::Pipeline> pPipeline,
			std::shared_ptr<BaseMesh> pMesh,
			std::shared_ptr<Material> pMaterial,
			uint32_t subMeshIndex = 0u,
			uint32_t passIndex = 0u);

	private:
		BaseRenderer() = delete;
		inline void _init();
	};

	template <SpaceType SPACE_TYPE>
	class Renderer : public BaseRenderer
	{
	public:
		typedef Scene<SPACE_TYPE> SceneType;
		typedef Camera<SPACE_TYPE> CameraType;

		Renderer(std::shared_ptr<vk::ImageView> pSwapchainImageView
			, vk::Format swapchainImageFormat
			, uint32_t swapchainImageWidth
			, uint32_t swapchainImageHeight
		);

		Renderer(std::shared_ptr<vk::ImageView> pSwapchainImageView
			, vk::Format swapchainImageFormat
			, uint32_t swapchainImageWidth
			, uint32_t swapchainImageHeight
			, std::shared_ptr<SceneType> pScene
			, std::shared_ptr<CameraType> pCamera
		);

		Renderer(std::shared_ptr<TextureColorAttachment> pColorAttachmentTex
		);

		Renderer(std::shared_ptr<TextureColorAttachment> pColorAttachmentTex
			, std::shared_ptr<SceneType> pScene
			, std::shared_ptr<CameraType> pCamera
		);

		void reset(std::shared_ptr<SceneType> pScene
			, std::shared_ptr<CameraType> pCamera
		);

	protected:
		//compositions

		//aggregations
		std::shared_ptr<SceneType> m_pScene;
		std::shared_ptr<CameraType> m_pCamera;

		void _render(RenderInfo renderInfo) override;

		virtual Bool32 _checkVisualObjectInsideCameraView(std::shared_ptr<typename SceneType::VisualObjectType> pVisualObject) = 0;

		inline virtual typename SpaceTypeInfo<SPACE_TYPE>::MatrixType _getMVPMatrix(std::shared_ptr<typename SceneType::ObjectType> pObject);

		inline virtual typename SpaceTypeInfo<SPACE_TYPE>::MatrixType _getMVMatrix(std::shared_ptr<typename SceneType::ObjectType> pObject);

	private:
		Renderer() = delete;
	};
} //namespace kgs

#include "graphics/renderer/renderer.inl"

#endif // !KGS_RENDERER_H
