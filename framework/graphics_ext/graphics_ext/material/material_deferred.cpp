#include "graphics_ext/material/material_deferred.hpp"

namespace vge
{
    MaterialDeferred::DeferredAttachmentInfo::DeferredAttachmentInfo(vk::Format format)
        : format(format)
    {}

    MaterialDeferred::CreateInfo::CreateInfo(vk::Format colorAttachmentFormat
        , vk::Format depthStencilAttachmentFormat
        , uint32_t deferredAttachmentCount
        , const DeferredAttachmentInfo *pDeferredAttachments
        )
        : colorAttachmentFormat(colorAttachmentFormat)
        , depthStencilAttachmentFormat(depthStencilAttachmentFormat)
        , deferredAttachmentCount(deferredAttachmentCount)
        , pDeferredAttachments(pDeferredAttachments)
    {}

    MaterialDeferred::MaterialDeferred(CreateInfo info
        , TextureCache<vg::Texture2DColorAttachment> *pColorAttachmentCache
        , TextureCache<vg::Texture2DDepthStencilAttachment> *pDepthStencilAttachmentCache
        )
        : vg::Material(VG_TRUE)
        , m_info(info)
        , m_deferredAttachmentInfos()
        , m_pColorAttachmentCache(pColorAttachmentCache)
        , m_pDepthStencilAttachmentCache(pDepthStencilAttachmentCache)
        , m_pMyColorAttachmentCache()
        , m_pMyDepthStencilAttachmentCache()
        , m_attachmentSize()
        , m_pColorAttachment()
        , m_pDepthStencilAttachment()
        , m_arrPDeferredAttachments()
        , m_pRectMesh()
    {
        m_deferredAttachmentInfos.resize(info.deferredAttachmentCount);
        memcpy(m_deferredAttachmentInfos.data(), 
            info.pDeferredAttachments, 
            info.deferredAttachmentCount * sizeof(DeferredAttachmentInfo));
        m_info.pDeferredAttachments = m_deferredAttachmentInfos.data();

        if (pColorAttachmentCache == nullptr) {
            m_pMyColorAttachmentCache = std::shared_ptr<TextureCache<vg::Texture2DColorAttachment>> {
                new TextureCache<vg::Texture2DColorAttachment>(),
            };
            m_pColorAttachmentCache = m_pMyColorAttachmentCache.get();
        }
        if (pDepthStencilAttachmentCache == nullptr) {
            m_pMyDepthStencilAttachmentCache = std::shared_ptr<TextureCache<vg::Texture2DDepthStencilAttachment>> {
                new TextureCache<vg::Texture2DDepthStencilAttachment>(),
            };
            m_pDepthStencilAttachmentCache = m_pMyDepthStencilAttachmentCache.get();
        }

        _createDeferredAttachments(info);
        _createRenderPass(info);
        _createOtherPasses(info);
        _initPasses(info);
    }

    void MaterialDeferred::_beginBindToRender(const BindInfo info, BindResult *pResult)
    {
		uint32_t trunkFramebufferWidth = info.trunkFramebufferWidth;
		uint32_t trunkFramebufferHeight = info.trunkFramebufferHeight;

        uint32_t targetSize = static_cast<uint32_t>(
			std::max(
				info.clipRect.width * trunkFramebufferWidth, 
				info.clipRect.height * trunkFramebufferHeight
			)
		);
        if (_updateResultAttachments(targetSize))
        {
            _updateFramebuffer();
            _updatePasses();
        }

        auto pRectMesh = _getRectMesh();
        //rect pos
        std::vector<vg::Vector2> rectPoses(4);
        //rect uv
        std::vector<vg::Vector2> rectUVs(4);
        fd::Viewport viewport;
        fd::Rect2D scissor;
        if (info.hasClipRect) {
            // in this case, we only draw the part of scene that contain the object of the mesh.
            auto rateX = static_cast<float>(trunkFramebufferWidth) / static_cast<float>(m_attachmentSize);
            auto rateY = static_cast<float>(trunkFramebufferHeight) / static_cast<float>(m_attachmentSize);
            viewport = fd::Viewport(
                rateX * (- info.clipRect.x), 
                rateY * (- info.clipRect.y), 
                rateX * 1.0f,
                rateY * 1.0f
            );
            
            scissor = fd::Rect2D(0.0f, 0.0f, rateX * info.clipRect.width, rateY * info.clipRect.height);
            rectPoses = {
                vg::Vector2{info.clipRect.x,                       info.clipRect.y},
                vg::Vector2{info.clipRect.x + info.clipRect.width, info.clipRect.y},
                vg::Vector2{info.clipRect.x + info.clipRect.width, info.clipRect.y + info.clipRect.height},
                vg::Vector2{info.clipRect.x,                       info.clipRect.y + info.clipRect.height},
            };
            rectUVs = {
                vg::Vector2{0.0f, 0.0f},
                vg::Vector2{rateX * info.clipRect.width, 0.0f},
                vg::Vector2{rateX * info.clipRect.width, rateY * info.clipRect.height},
                vg::Vector2{0.0f, rateY * info.clipRect.height},
            };
        } else {
            // in this case, we will cast entire scene where there is the object of the mesh in.
            viewport = fd::Viewport();
            scissor = info.hasClipRect ? info.clipRect : fd::Rect2D();
            rectPoses = {
                vg::Vector2{0.0f, 0.0f},
                vg::Vector2{1.0f, 0.0f},
                vg::Vector2{1.0f, 1.0f},
                vg::Vector2{0.0f, 1.0f},
            };
            rectUVs = {
                vg::Vector2{0.0f, 0.0f},
                vg::Vector2{1.0f, 0.0f},
                vg::Vector2{1.0f, 1.0f},
                vg::Vector2{0.0f, 1.0f},
            };
        }
        //map [0, 1] to [-1, 1];
        for (auto &point : rectPoses) {
            point.x = point.x * 2 - 1;
            point.y = point.y * 2 - 1;
        }
    
        std::vector<uint32_t> indices = {
            0, 1, 3, 3, 1, 2
        };
    
	    pRectMesh->setVertexCount(static_cast<uint32_t>(rectPoses.size()));
        pRectMesh->setPositions(rectPoses);
        pRectMesh->setTextureCoordinates<vg::TextureCoordinateType::VECTOR_2, vg::TextureCoordinateIndex::TextureCoordinate_0>(
            rectUVs
        );
        pRectMesh->setIndices(indices, vg::PrimitiveTopology::TRIANGLE_LIST, 0u);
        pRectMesh->apply(VG_TRUE);
    
        auto &result = *pResult;
    
        //frist pass
        {
            vg::RenderPassInfo renderPassInfo;
            renderPassInfo.pRenderPass = m_pRenderPass.get();
            renderPassInfo.subPassIndex = 0u;
	    	renderPassInfo.pFrameBuffer = m_pFrameBuffer.get();
            renderPassInfo.framebufferWidth = m_attachmentSize;
            renderPassInfo.framebufferHeight = m_attachmentSize;
            renderPassInfo.projMatrix = *(info.pProjMatrix);
            renderPassInfo.viewMatrix = *(info.pViewMatrix);
            renderPassInfo.pPass = m_pPassDeferred.get();
            renderPassInfo.modelMatrix = *(info.pModelMatrix);
            renderPassInfo.pMesh = info.pMesh;
            renderPassInfo.subMeshIndex = info.subMeshIndex;
            renderPassInfo.viewport = viewport;
            renderPassInfo.scissor = scissor;
    
            vk::ClearValue clearValueColor = {
	    		std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f}
	    	};
	    	vk::ClearValue clearValueDepthStencil = {
	    		vk::ClearDepthStencilValue(1.0f, 0)
	    	};
    
            vk::ClearValue clearValues[5];
	    	clearValues[0] = clearValueColor;
	    	clearValues[1] = clearValueColor;
	    	clearValues[2] = clearValueColor;
	    	clearValues[3] = clearValueColor;
	    	clearValues[4] = clearValueDepthStencil;
    
            renderPassInfo.clearValueCount = 5;
            renderPassInfo.pClearValues = clearValues;
    
            vg::CmdInfo cmdInfo;
            cmdInfo.pRenderPassInfo = &renderPassInfo;
            result.pBranchCmdBuffer->addCmd(cmdInfo);
        }
    
        {
            
            vg::RenderPassInfo renderPassInfo;
            renderPassInfo.pRenderPass = m_pRenderPass.get();
            renderPassInfo.subPassIndex = 1u;
	    	renderPassInfo.pFrameBuffer = m_pFrameBuffer.get();
            renderPassInfo.framebufferWidth = m_attachmentSize;
            renderPassInfo.framebufferHeight = m_attachmentSize;
            renderPassInfo.projMatrix = vg::Matrix4x4(1.0f);
            renderPassInfo.viewMatrix = vg::Matrix4x4(1.0f);
            renderPassInfo.pPass = m_pPassComposition.get();
            renderPassInfo.modelMatrix = vg::Matrix4x4(1.0f);
            renderPassInfo.pMesh = nullptr;
            renderPassInfo.subMeshIndex = 0u;
            renderPassInfo.viewport = fd::Viewport();
            renderPassInfo.scissor = scissor;
    
            vg::CmdDraw cmdDraw = {3,1,0,0};
            renderPassInfo.pCmdDraw = &cmdDraw;
    
            vg::CmdInfo cmdInfo;
            cmdInfo.pRenderPassInfo = &renderPassInfo;
            result.pBranchCmdBuffer->addCmd(cmdInfo);
    
        }
    
        //final pass is trunk pass.
        {
            vg::RenderPassInfo trunkRenderPassInfo;
            trunkRenderPassInfo.pRenderPass = nullptr;
	    	trunkRenderPassInfo.pFrameBuffer = nullptr;
            trunkRenderPassInfo.framebufferWidth = info.trunkFramebufferWidth;
            trunkRenderPassInfo.framebufferHeight = info.trunkFramebufferHeight;
            trunkRenderPassInfo.projMatrix = vg::Matrix4x4(1.0f);
            trunkRenderPassInfo.viewMatrix = vg::Matrix4x4(1.0f);
            trunkRenderPassInfo.pPass = m_pMainPass.get();
            trunkRenderPassInfo.modelMatrix = vg::Matrix4x4(1.0f);
            trunkRenderPassInfo.pMesh = pRectMesh;
            trunkRenderPassInfo.subMeshIndex = 0u;
            trunkRenderPassInfo.viewport = fd::Viewport();
            trunkRenderPassInfo.scissor = info.hasClipRect ? info.clipRect : fd::Rect2D();
    
            vg::CmdInfo cmdInfo;
            cmdInfo.pRenderPassInfo = &trunkRenderPassInfo;
            result.pTrunkRenderPassCmdBuffer->addCmd(cmdInfo);
        }
    
    }

    vg::Pass * MaterialDeferred::getPassDeferred() const
    {
        return m_pPassDeferred.get();
    }
    
    vg::Pass * MaterialDeferred::getPassComposition() const
    {
        return m_pPassComposition.get();
    }

    void MaterialDeferred::_createDeferredAttachments(CreateInfo createInfo)
    {
        m_arrPDeferredAttachments.resize(createInfo.deferredAttachmentCount);
    }
        
    void MaterialDeferred::_createRenderPass(CreateInfo createInfo)
    {
        uint32_t attachmentCount = createInfo.deferredAttachmentCount + 2;
        std::vector<vk::AttachmentDescription> attachmentDescs(attachmentCount);
        std::vector<vk::Format> attachmentFormats(attachmentCount);
        std::vector<vk::ImageLayout> attachmentLayouts(attachmentCount);
        uint32_t offset = 0u;
        attachmentFormats[offset] = createInfo.colorAttachmentFormat;
        attachmentLayouts[offset] = vk::ImageLayout::eShaderReadOnlyOptimal;
        ++offset;
        for (uint32_t i = 0; i < attachmentCount - 2; ++i, ++offset)
        {
            attachmentFormats[offset] = (createInfo.pDeferredAttachments + i)->format;
            attachmentLayouts[offset] = vk::ImageLayout::eShaderReadOnlyOptimal;
        }
        attachmentFormats[offset] = createInfo.depthStencilAttachmentFormat;
        attachmentLayouts[offset] = vk::ImageLayout::eDepthStencilReadOnlyOptimal;

        for (uint32_t i = 0; i < attachmentCount; ++i) {
            attachmentDescs[i].format = attachmentFormats[i];
            attachmentDescs[i].samples = vk::SampleCountFlagBits::e1;
            attachmentDescs[i].loadOp = vk::AttachmentLoadOp::eClear;
            attachmentDescs[i].storeOp = vk::AttachmentStoreOp::eDontCare;
            attachmentDescs[i].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            attachmentDescs[i].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
            attachmentDescs[i].initialLayout = attachmentLayouts[i];
            attachmentDescs[i].finalLayout = attachmentLayouts[i];
        }
    
        attachmentDescs[0].storeOp = vk::AttachmentStoreOp::eStore;
        attachmentDescs[attachmentCount - 1].storeOp = vk::AttachmentStoreOp::eStore;
    
        uint32_t subpassCount = 2u;
        std::vector<vk::SubpassDescription> subpassDescs(subpassCount);
    
        
        //first subpass: fill G-Buffer components
        std::vector<vk::AttachmentReference> colorAttachmentRefs1(createInfo.deferredAttachmentCount);
        for (uint32_t i = 0; i < createInfo.deferredAttachmentCount; ++i)
        {
			colorAttachmentRefs1[i] = {i + 1, vk::ImageLayout::eColorAttachmentOptimal};
        }

        vk::AttachmentReference depthAttachmentRef1 = {attachmentCount - 1, vk::ImageLayout::eDepthStencilAttachmentOptimal};

        subpassDescs[0].pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpassDescs[0].colorAttachmentCount = createInfo.deferredAttachmentCount;
        subpassDescs[0].pColorAttachments = colorAttachmentRefs1.data();
        subpassDescs[0].pDepthStencilAttachment = &depthAttachmentRef1;
        
    
        
        //Second subpass: final composition with G-Buffer components.
        vk::AttachmentReference colorAttachmentRef2 = {0, vk::ImageLayout::eColorAttachmentOptimal};
        std::vector<vk::AttachmentReference> inputAttachmentRefs2(createInfo.deferredAttachmentCount);
        for (uint32_t i = 0; i < createInfo.deferredAttachmentCount; ++i)
        {
            inputAttachmentRefs2[i] = {i + 1, vk::ImageLayout::eShaderReadOnlyOptimal};
        } 
        subpassDescs[1].pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpassDescs[1].colorAttachmentCount = 1u;
        subpassDescs[1].pColorAttachments = &colorAttachmentRef2;
        subpassDescs[1].pDepthStencilAttachment = nullptr;;
        subpassDescs[1].inputAttachmentCount = static_cast<uint32_t>(inputAttachmentRefs2.size());
        subpassDescs[1].pInputAttachments = inputAttachmentRefs2.data();
        
    
        uint32_t dependencyCount = subpassCount + 1;
        std::vector<vk::SubpassDependency> dependencies(dependencyCount);
    
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
        dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eLateFragmentTests;
        dependencies[0].srcAccessMask = vk::AccessFlagBits::eShaderRead;
        dependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite |
            vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        dependencies[0].dependencyFlags = vk::DependencyFlagBits::eByRegion;
    
        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = 1;
        dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eLateFragmentTests;
        dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
        dependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite |
            vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        dependencies[1].dstAccessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
        dependencies[1].dependencyFlags = vk::DependencyFlagBits::eByRegion;
    
        dependencies[2].srcSubpass = 1;
        dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[2].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependencies[2].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
        dependencies[2].srcAccessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
        dependencies[2].dstAccessMask = vk::AccessFlagBits::eShaderRead;
        dependencies[2].dependencyFlags = vk::DependencyFlagBits::eByRegion;
        
    
	    vk::RenderPassCreateInfo renderPassCreateInfo = {
	    	vk::RenderPassCreateFlags(),
	    	static_cast<uint32_t>(attachmentDescs.size()),
	    	attachmentDescs.data(),
	    	static_cast<uint32_t>(subpassDescs.size()),
	    	subpassDescs.data(),
	    	static_cast<uint32_t>(dependencies.size()),
	    	dependencies.data()
	    };
	    auto pDevice = vg::pApp->getDevice();
	    m_pRenderPass = fd::createRenderPass(pDevice, renderPassCreateInfo);
    }

    void MaterialDeferred::_createOtherPasses(CreateInfo createInfo)
    {
        m_pShaderDeferred = std::shared_ptr<vg::Shader>{new vg::Shader()};
        m_pShaderComposition = std::shared_ptr<vg::Shader>{new vg::Shader()};
	    m_pPassDeferred = std::shared_ptr<vg::Pass>{ new vg::Pass(m_pShaderDeferred.get())};
	    m_pPassComposition = std::shared_ptr<vg::Pass>{ new vg::Pass(m_pShaderComposition.get())};
	    _addPass(m_pPassDeferred.get());
	    _addPass(m_pPassComposition.get());
    }
        
    void MaterialDeferred::_initPasses(CreateInfo createInfo)
    {
         //deferred pass
        {
            auto pPass = m_pPassDeferred.get();
            pPass->setSubpass(0u);
            const uint32_t attachmentCount = 3u;
	        vk::PipelineColorBlendAttachmentState attachmentStates[attachmentCount] = {};
            for (uint32_t i = 0; i < attachmentCount; ++i)
            {
                attachmentStates[i].colorWriteMask = vk::ColorComponentFlagBits::eR | 
                    vk::ColorComponentFlagBits::eG | 
                    vk::ColorComponentFlagBits::eB | 
                    vk::ColorComponentFlagBits::eA;
	            attachmentStates[i].blendEnable = VG_FALSE;
            }
	        
	        vk::PipelineColorBlendStateCreateInfo colorBlendState = {};
	        colorBlendState.attachmentCount = attachmentCount;
	        colorBlendState.pAttachments = attachmentStates;
	        pPass->setColorBlendInfo(colorBlendState);
            vk::PipelineDepthStencilStateCreateInfo depthStencilState = {};
	        depthStencilState.depthTestEnable = VG_TRUE;
	        depthStencilState.depthWriteEnable = VG_TRUE;
	        depthStencilState.depthCompareOp = vk::CompareOp::eLessOrEqual;
	        pPass->setDepthStencilInfo(depthStencilState);
            pPass->apply();
        }
    
        //composition pass
        {
            auto pPass = m_pPassComposition.get();
            pPass->setSubpass(1u);
            pPass->setPolygonMode(vg::PolygonMode::FILL);
	        pPass->setCullMode(vg::CullModeFlagBits::NONE);
	        pPass->setFrontFace(vg::FrontFaceType::COUNTER_CLOCKWISE);
	        vg::Pass::BuildInDataInfo buildInDataInfo;
	        buildInDataInfo.componentCount = 0u;
	        buildInDataInfo.pComponent = nullptr;
	        pPass->setBuildInDataInfo(buildInDataInfo);
    
            vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = {
                vk::PipelineInputAssemblyStateCreateFlags(),
                vk::PrimitiveTopology::eTriangleList
            };
    
            pPass->setDefaultInputAssemblyState(inputAssemblyState);
            
             const uint32_t attachmentCount = 1u;
	        vk::PipelineColorBlendAttachmentState attachmentStates[attachmentCount] = {};
            for (uint32_t i = 0; i < attachmentCount; ++i)
            {
                attachmentStates[i].colorWriteMask = vk::ColorComponentFlagBits::eR | 
                    vk::ColorComponentFlagBits::eG | 
                    vk::ColorComponentFlagBits::eB | 
                    vk::ColorComponentFlagBits::eA;
	            attachmentStates[i].blendEnable = VG_FALSE;
            }
	        
	        vk::PipelineColorBlendStateCreateInfo colorBlendState = {};
	        colorBlendState.attachmentCount = attachmentCount;
	        colorBlendState.pAttachments = attachmentStates;
	        pPass->setColorBlendInfo(colorBlendState);
    
	        vk::PipelineDepthStencilStateCreateInfo depthStencilState = {};
	        depthStencilState.depthTestEnable = VG_FALSE;
	        depthStencilState.depthWriteEnable = VG_FALSE;
	        depthStencilState.depthCompareOp = vk::CompareOp::eAlways;
	        pPass->setDepthStencilInfo(depthStencilState);

            
                
            pPass->apply();
        }
    
        //main pass
        {
            auto pPass = m_pMainPass.get();
            pPass->setPolygonMode(vg::PolygonMode::FILL);
	        pPass->setCullMode(vg::CullModeFlagBits::NONE);
	        pPass->setFrontFace(vg::FrontFaceType::COUNTER_CLOCKWISE);
            vg::Pass::BuildInDataInfo buildInDataInfo;
	        buildInDataInfo.componentCount = 0u;
	        buildInDataInfo.pComponent = nullptr;
	        pPass->setBuildInDataInfo(buildInDataInfo);
            vk::PipelineDepthStencilStateCreateInfo depthStencilState = {};
	        depthStencilState.depthTestEnable = VG_TRUE;
	        depthStencilState.depthWriteEnable = VG_TRUE;
	        depthStencilState.depthCompareOp = vk::CompareOp::eLessOrEqual;
            pPass->setDepthStencilInfo(depthStencilState);
            
            pPass->apply();
        }
    }
       
    vg::Bool32 MaterialDeferred::_updateResultAttachments(uint32_t targetSize)
    {
        targetSize = static_cast<uint32_t>(std::pow(2, std::ceil(std::log2(targetSize))));
        if (m_attachmentSize != targetSize) 
        {
            auto info = m_info;
            //free
            {
                for (uint32_t i = 0; i < info.deferredAttachmentCount; ++i) {
                    if (m_arrPDeferredAttachments[i] != nullptr) {
                        auto deferredAttachmentInfo = *(info.pDeferredAttachments + i);
                        vge::TextureCacheAllocInfo allocInfo = {
                            deferredAttachmentInfo.format,
                            m_attachmentSize,
                            m_attachmentSize,
                            VG_TRUE,
                        };
                    
                        m_pColorAttachmentCache->freeTexture(allocInfo, m_arrPDeferredAttachments[i]);
                    }
                }

                {
                    if (m_pColorAttachment != nullptr) {
                        vge::TextureCacheAllocInfo allocInfo = {
                            info.colorAttachmentFormat,
                            m_attachmentSize,
                            m_attachmentSize,
                            VG_FALSE,
                        };
                    
                        m_pColorAttachmentCache->freeTexture(allocInfo, m_pColorAttachment);
                    }
                }

                {
                    if (m_pDepthStencilAttachment != nullptr) {
                        vge::TextureCacheAllocInfo allocInfo = {
                            info.depthStencilAttachmentFormat,
                            m_attachmentSize,
                            m_attachmentSize,
                            VG_FALSE,
                        };

                        m_pDepthStencilAttachmentCache->freeTexture(allocInfo, m_pDepthStencilAttachment);
                    }
                }
            }

            m_attachmentSize = targetSize;

            //allocate
            {
                for (uint32_t i = 0; i < info.deferredAttachmentCount; ++i) {
                    auto deferredAttachmentInfo = *(info.pDeferredAttachments + i);
                    vge::TextureCacheAllocInfo allocInfo = {
                        deferredAttachmentInfo.format,
                        m_attachmentSize,
                        m_attachmentSize,
                        VG_TRUE,
                    };
                    m_arrPDeferredAttachments[i] = m_pColorAttachmentCache->allocateTexture(allocInfo);
                }

                {
                    vge::TextureCacheAllocInfo allocInfo = {
                        info.colorAttachmentFormat,
                        m_attachmentSize,
                        m_attachmentSize,
                        VG_FALSE,
                    };
                    
                    m_pColorAttachment = m_pColorAttachmentCache->allocateTexture(allocInfo);
                }

                {
                    vge::TextureCacheAllocInfo allocInfo = {
                        info.depthStencilAttachmentFormat,
                        m_attachmentSize,
                        m_attachmentSize,
                        VG_FALSE,
                    };

                    m_pDepthStencilAttachment = m_pDepthStencilAttachmentCache->allocateTexture(allocInfo);

                    //Create depth only image view for sampling from shader in trunk pass.
                    if (m_pDepthStencilAttachment->getImageView("only_depth") == nullptr) {
                        auto pImage = m_pDepthStencilAttachment->getImage();
                        vg::Texture::ImageViewCreateInfo info = {
                            vk::ComponentMapping(),
                            {
                                vk::ImageAspectFlagBits::eDepth,
                                0u,
                                pImage->getInfo().mipLevels,
                                0u,
                                pImage->getInfo().arrayLayers,
                            },
                        };
                        m_pDepthStencilAttachment->createImageView("only_depth", info);
                    }
                }
            }
            
            return VG_TRUE;
        }
        else
        {
            return VG_FALSE;
        }
    }

    void MaterialDeferred::_updateFramebuffer()
    {
        auto info = m_info;
        uint32_t attachmentCount = info.deferredAttachmentCount;
        attachmentCount += 2u;

        std::vector<vk::ImageView> attachments(attachmentCount);
        uint32_t offset = 0u;
        attachments[offset] = *(m_pColorAttachment->getImageView()->getImageView());
        ++offset;
        for (uint32_t i = 0; i < info.deferredAttachmentCount; ++i, ++offset)
        {
            attachments[offset] = *(m_arrPDeferredAttachments[i]->getImageView()->getImageView());
        }
        attachments[offset] = *(m_pDepthStencilAttachment->getImageView()->getImageView());
    
	    vk::FramebufferCreateInfo createInfo = {
	    	vk::FramebufferCreateFlags(),                   //flags
	    	*m_pRenderPass,                                 //renderPass
	    	attachmentCount,                                //attachmentCount
	    	attachments.data(),                             //pAttachments
	    	m_attachmentSize,                               //width
	    	m_attachmentSize,                               //height
	    	1u                                              //layers
	    };
	    auto pDevice = vg::pApp->getDevice();
	    m_pFrameBuffer = fd::createFrameBuffer(pDevice, createInfo);
    }

    void MaterialDeferred::_updatePasses()
    {
        {
            auto pPass = m_pPassComposition.get();
            auto info = m_info;
            for (uint32_t i = 0; i < info.deferredAttachmentCount; ++i) 
            {
                pPass->setTexture("input_" + std::to_string(i), m_arrPDeferredAttachments[i], i, vg::ShaderStageFlagBits::FRAGMENT, 
                    vg::DescriptorType::INPUT_ATTACHMENT);
            }
            pPass->apply();
        }

        {
            auto pPass = m_pMainPass.get();
            pPass->setTexture("color", m_pColorAttachment, 0u, vg::ShaderStageFlagBits::FRAGMENT);
            pPass->setTexture("depth", m_pDepthStencilAttachment, 1u, vg::ShaderStageFlagBits::FRAGMENT,
                vg::DescriptorType::COMBINED_IMAGE_SAMPLER, m_pDepthStencilAttachment->getImageView("only_depth"));
            pPass->apply();
        }
    }

    vg::DimSepMesh2 *MaterialDeferred::_getRectMesh()
    {
    	if (m_pRectMesh == nullptr)
    	{
    		std::shared_ptr<vg::DimSepMesh2> pNewMesh = std::shared_ptr<vg::DimSepMesh2>{
                new vg::DimSepMesh2(vg::MemoryPropertyFlagBits::HOST_VISIBLE)
            };
            m_pRectMesh = pNewMesh;
    	}
        return m_pRectMesh.get();
    }

} //vge