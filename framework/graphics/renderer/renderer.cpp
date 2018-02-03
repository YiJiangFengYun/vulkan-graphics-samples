#include "graphics/renderer/renderer.hpp"

#include "graphics/vertex_data/util.hpp"

namespace vg
{
	const vk::Format BaseRenderer::DEFAULT_DEPTH_STENCIL_FORMAT(vk::Format::eD32SfloatS8Uint);

	BaseRenderer::BaseRenderer(std::shared_ptr<vk::ImageView> pSwapchainImageView
		, vk::Format swapchainImageFormat
		, uint32_t swapchainImageWidth
		, uint32_t swapchainImageHeight
	)
		: Base(BaseType::RENDERER)
		, m_pSwapchainImageView(pSwapchainImageView)
		, m_swapchainImageFormat(swapchainImageFormat)
		, m_colorImageFormat(swapchainImageFormat)
		, m_depthStencilImageFormat(DEFAULT_DEPTH_STENCIL_FORMAT)
		, m_framebufferWidth(swapchainImageWidth)
		, m_framebufferHeight(swapchainImageHeight)
		, m_clearValueColor(0.0f)
		, m_clearValueDepth(1.0f)
		, m_clearValueStencil(0u)
		, m_renderArea(0.0f, 0.0f, 1.0f, 1.0f)
		, m_pipelineCache()
	{
		_init();
	}

	BaseRenderer::BaseRenderer(std::shared_ptr<TextureColorAttachment> pColorAttachmentTex
	)
		: Base(BaseType::RENDERER)
		, m_pColorTexture(pColorAttachmentTex)
		, m_colorImageFormat(pColorAttachmentTex->_getVKFormat())
		, m_depthStencilImageFormat(DEFAULT_DEPTH_STENCIL_FORMAT)
		, m_framebufferWidth(pColorAttachmentTex->getWidth())
		, m_framebufferHeight(pColorAttachmentTex->getHeight())
		, m_clearValueColor(0.0f)
		, m_clearValueDepth(1.0f)
		, m_clearValueStencil(0u)
		, m_renderArea(0.0f, 0.0f, 1.0f, 1.0f)
		, m_pipelineCache()
	{
		_init();
	}

	BaseRenderer::~BaseRenderer()
	{
		//_freeGraphicsQueue();
	}

	Bool32 BaseRenderer::isValidForRender()
	{
		return _isValidForRender();
	}

	void BaseRenderer::render(const RenderInfo &info, RenderResultInfo &resultInfo)
	{
		_preRender();
		_render(info, resultInfo);
		_postRender();
	}

	uint32_t BaseRenderer::getFramebufferWidth()
	{
		return m_framebufferWidth;
	}

	uint32_t BaseRenderer::getFramebufferHeight()
	{
		return m_framebufferHeight;
	}

	vk::Format BaseRenderer::getColorImageFormat()
	{
		return m_colorImageFormat;
	}

	vk::Format BaseRenderer::getDepthStencilImageFormat()
	{
		return m_depthStencilImageFormat;
	}

	const Color &BaseRenderer::getClearValueColor() const
	{
		return m_clearValueColor;
	}

	void BaseRenderer::setClearValueColor(Color color)
	{
		m_clearValueColor = color;
	}

	float BaseRenderer::getClearValueDepth() const
	{
		return m_clearValueDepth;
	}

	void BaseRenderer::setClearValueDepth(float value)
	{
		m_clearValueDepth = value;
	}

	uint32_t BaseRenderer::getClearValueStencil() const
	{
		return m_clearValueStencil;
	}

	void BaseRenderer::setClearValueStencil(uint32_t value)
	{
		m_clearValueStencil = value;
	}

	const fd::Rect2D &BaseRenderer::getClearArea() const
	{
		return m_renderArea;
	}

	void BaseRenderer::setClearArea(fd::Rect2D area)
	{
#ifdef DEBUG
		if (area.width < 0)
			throw std::invalid_argument("The width of area is smaller than 0!");
		else if (area.width > 1)
			throw std::invalid_argument("The width of area is bigger than 1!");
		if (area.height < 0)
			throw std::invalid_argument("The height of area is smaller than 0!");
		else if (area.height > 1)
			throw std::invalid_argument("The height of area is bigger than 1!");
		if (area.x < 0)
			throw std::invalid_argument("the x of area is smaller than 0!");
		else if (area.x > area.width)
			throw std::invalid_argument("The x of area is bigger than the width of area!");
		if (area.y < 0)
			throw std::invalid_argument("the y of area is smaller than 0!");
		else if (area.y > area.height)
			throw std::invalid_argument("The y of area is bigger than the height of area!");
#endif // DEBUG

		m_renderArea = area;
	}

	Bool32 BaseRenderer::_isValidForRender()
	{
		return VG_TRUE;
	}

	void BaseRenderer::_preRender()
	{
		m_pipelineCache.start();
	}

	void BaseRenderer::_render(const RenderInfo &info, RenderResultInfo &resultInfo)
	{
	}

	void BaseRenderer::_postRender()
	{
		m_pipelineCache.end();
	}

	void BaseRenderer::_createPipelineForRender(std::shared_ptr<vk::Pipeline> &pPipeline,
		std::shared_ptr<BaseMesh> pMesh,
		std::shared_ptr<Material> pMaterial,
		uint32_t subMeshIndex,
		uint32_t passIndex)
	{
		ContentMesh * pContentMesh = dynamic_cast<ContentMesh *>(pMesh.get());
		auto pPass = pMaterial->getPassWithIndex(passIndex);
		PipelineCache::Info info(
			*m_pRenderPass,
			pPass,
			pContentMesh->getVertexData(),
			0u,
			pContentMesh->getIndexData(),
			subMeshIndex
		);
		pPipeline = m_pipelineCache.caching(info);
	}

	void BaseRenderer::_recordCommandBufferForRender(std::shared_ptr<vk::Pipeline> pPipeline,
		std::shared_ptr<BaseMesh> pMesh,
		std::shared_ptr<Material> pMaterial,
		uint32_t subMeshIndex,
		uint32_t passIndex)
	{
		LOG(plog::debug) << "Pre begin command buffer for render." << std::endl;
		ContentMesh * pContentMesh = dynamic_cast<ContentMesh *>(pMesh.get());
		auto pPass = pMaterial->getPassWithIndex(passIndex);
		
		vk::CommandBufferBeginInfo beginInfo = {
			vk::CommandBufferUsageFlagBits::eSimultaneousUse,  //flags
			nullptr                                            //pInheritanceInfo
		};

		m_pCommandBuffer->begin(beginInfo);
		LOG(plog::debug) << "Post begin command buffer for render." << std::endl;

		vk::ClearValue clearValueColor = {
			std::array<float, 4>{m_clearValueColor[0]
					, m_clearValueColor[1]
					, m_clearValueColor[2]
					, m_clearValueColor[3]
			}
		};
		vk::ClearValue clearValueDepthStencil = {
			vk::ClearDepthStencilValue(m_clearValueDepth
				, m_clearValueStencil
			)
		};
		std::array<vk::ClearValue, 2> clearValues = { clearValueColor
			, clearValueDepthStencil
		};


		const auto& renderArea = m_renderArea;

		vk::RenderPassBeginInfo renderPassBeginInfo = {
			*m_pRenderPass,                                   //renderPass
			*m_pFrameBuffer,                                  //framebuffer
			vk::Rect2D(                                       //renderArea
				vk::Offset2D(static_cast<int32_t>(std::round(m_framebufferWidth * renderArea.x))
					, static_cast<int32_t>(std::round(m_framebufferHeight * renderArea.y))
				),
				vk::Extent2D(static_cast<uint32_t>(std::round(m_framebufferWidth * renderArea.width)),
					static_cast<uint32_t>(std::round(m_framebufferHeight * renderArea.height))
				)
			),
			static_cast<uint32_t>(clearValues.size()),      //clearValueCount
			clearValues.data()                              //pClearValues
		};

		m_pCommandBuffer->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
        
        const auto& viewportOfPass = pPass->getViewport();
		const auto& scissorOfPass = pPass->getScissor();
		//View port info.
		vk::Viewport viewport = {
			(float)m_framebufferWidth * viewportOfPass.x,                      //x
			(float)m_framebufferHeight * viewportOfPass.y,                     //y
			(float)m_framebufferWidth * viewportOfPass.width,                  //width
			(float)m_framebufferHeight * viewportOfPass.height,                 //height
			1.0f * viewportOfPass.minDepth,                                     //minDepth
			1.0f * viewportOfPass.maxDepth                                      //maxDepth
		};

		m_pCommandBuffer->setViewport(0, viewport);

		vk::Rect2D scissor = {
			{                               //offset
				static_cast<int32_t>(std::round(m_framebufferWidth * scissorOfPass.x)),    //x
				static_cast<int32_t>(std::round(m_framebufferHeight * scissorOfPass.y))    //y
			},
			{                               //extent
				static_cast<uint32_t>(std::round(m_framebufferWidth * scissorOfPass.width)),   //width
				static_cast<uint32_t>(std::round(m_framebufferHeight * scissorOfPass.height))  //height
			}
		};

		m_pCommandBuffer->setScissor(0, scissor);

		auto pPipelineLayout = pPass->getPipelineLayout();		

		//push constants
		auto pushConstantUpdates = pPass->getPushconstantUpdates();
		for(const auto& pPushConstantUpdate : pushConstantUpdates)
		{
			m_pCommandBuffer->pushConstants(*pPipelineLayout, 
			    pPushConstantUpdate->getStageFlags(), 
				pPushConstantUpdate->getOffset(),
				pPushConstantUpdate->getSize(),
				pPushConstantUpdate->getData());
		}


		m_pCommandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *pPipeline);

		uint32_t descriptSetCount = pPass->getDescriptorSet() != nullptr ? 1 : 0;
		std::vector<vk::DescriptorSet> descriptorSets(descriptSetCount);
		if (pPass->getDescriptorSet() != nullptr)
		{
			descriptorSets[0] = *pPass->getDescriptorSet();
		}
		m_pCommandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pPipelineLayout, 0u, descriptorSets, nullptr);

        vertexDataToCommandBuffer(pContentMesh->getVertexData(), *m_pCommandBuffer, 0);
		indexDataToCommandBuffer(pContentMesh->getIndexData(), *m_pCommandBuffer, subMeshIndex);
		m_pCommandBuffer->drawIndexed(pContentMesh->getIndexData()->getSubIndexDatas()[subMeshIndex].indexCount, 1u, 0u, 0u, 0u);
		//m_pCommandBuffer->draw(3, 1, 0, 0);

		m_pCommandBuffer->endRenderPass();

		LOG(plog::debug) << "Pre end command buffer." << std::endl;
		m_pCommandBuffer->end();
		LOG(plog::debug) << "Post end command buffer." << std::endl;
	}

	void BaseRenderer::_init()
	{
		_createRenderPass();
		_createDepthStencilTex();
		_createFramebuffer();
		_createCommandPool();
		_createCommandBuffer();
	}

	void BaseRenderer::_createRenderPass()
	{
		vk::AttachmentDescription colorAttachment = {
			vk::AttachmentDescriptionFlags(),     //flags
			m_colorImageFormat,                   //format
			vk::SampleCountFlagBits::e1,          //samples
			vk::AttachmentLoadOp::eClear,         //loadOp
			vk::AttachmentStoreOp::eStore,        //storeOp
			vk::AttachmentLoadOp::eDontCare,      //stencilLoadOp
			vk::AttachmentStoreOp::eDontCare,     //stencilStoreOp
			vk::ImageLayout::eUndefined,          //initialLayout
			m_pColorTexture == nullptr ? vk::ImageLayout::ePresentSrcKHR : vk::ImageLayout::eShaderReadOnlyOptimal     //finalLayout
		};

		vk::AttachmentDescription depthAttachment = {
			vk::AttachmentDescriptionFlags(),
			m_depthStencilImageFormat,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eDontCare,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal
		};

		vk::AttachmentReference colorAttachmentRef = {
			uint32_t(0),
			vk::ImageLayout::eColorAttachmentOptimal
		};

		vk::AttachmentReference depthAttachmentRef = {
			uint32_t(1),
			vk::ImageLayout::eDepthStencilAttachmentOptimal
		};

		vk::SubpassDescription subpass = {
			vk::SubpassDescriptionFlags(),       //flags
			vk::PipelineBindPoint::eGraphics,    //pipelineBindPoint
			0,                                   //inputAttachmentCount
			nullptr,                             //pInputAttachments
			1,                                   //colorAttachmentCount
			&colorAttachmentRef,                 //pColorAttachments
			nullptr,                             //pResolveAttachments
			&depthAttachmentRef,                 //pDepthStencilAttachment
			0,                                   //preserveAttachmentCount
			nullptr                              //pPreserveAttachments
		};

		vk::SubpassDependency dependency = {
			VK_SUBPASS_EXTERNAL,
			0,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlags(),
			vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
			vk::DependencyFlags()
		};

		std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		vk::RenderPassCreateInfo createInfo = {
			vk::RenderPassCreateFlags(),
			static_cast<uint32_t>(attachments.size()),
			attachments.data(),
			1u,
			&subpass,
			1u,
			&dependency
		};

		auto pDevice = pApp->getDevice();
		m_pRenderPass = fd::createRenderPass(pDevice, createInfo);
	}

	void BaseRenderer::_createDepthStencilTex()
	{
		m_pDepthStencilTexture = std::shared_ptr<TextureDepthStencilAttachment>(
			new TextureDepthStencilAttachment(
				TextureFormat::D32_SFLOAT_S8_UINT,
				m_framebufferWidth,
				m_framebufferHeight
			)
			);
	}

	void BaseRenderer::_createFramebuffer()
	{
		std::array<vk::ImageView, 2> attachments;
		if (m_pColorTexture != nullptr)
		{
			attachments = { *m_pColorTexture->_getImageView(), *m_pDepthStencilTexture->_getImageView() };
		}
		else
		{
			attachments = { *m_pSwapchainImageView, *m_pDepthStencilTexture->_getImageView() };
		}

		vk::FramebufferCreateInfo createInfo = {
			vk::FramebufferCreateFlags(),                   //flags
			*m_pRenderPass,                                 //renderPass
			static_cast<uint32_t>(attachments.size()),      //attachmentCount
			attachments.data(),                             //pAttachments
			m_framebufferWidth,                             //width
			m_framebufferHeight,                            //height
			1u                                              //layers
		};

		auto pDevice = pApp->getDevice();
		m_pFrameBuffer = fd::createFrameBuffer(pDevice, createInfo);
	}

	void BaseRenderer::_createCommandPool()
	{
		auto pDevice = pApp->getDevice();
		auto graphicsFamily = pApp->getGraphicsFamily();
		vk::CommandPoolCreateInfo createInfo = {
			vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			graphicsFamily
		};
		m_pCommandPool = fd::createCommandPool(pDevice, createInfo);
	}

	void BaseRenderer::_createCommandBuffer()
	{
		auto pCommandPool = m_pCommandPool;
		vk::CommandBufferAllocateInfo allocateInfo = {
			*pCommandPool,                             //commandPool
			vk::CommandBufferLevel::ePrimary,          //level
			1u                                         //commandBufferCount
		};

		auto pDevice = pApp->getDevice();

		LOG(plog::debug) << "Pre allocate command buffer from pool." << std::endl;
		m_pCommandBuffer = fd::allocateCommandBuffer(pDevice, pCommandPool, allocateInfo);
		LOG(plog::debug) << "Post allocate command buffer from pool." << std::endl;
	}
}