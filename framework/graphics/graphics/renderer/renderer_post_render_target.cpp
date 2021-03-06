#include "graphics/renderer/renderer_post_render_target.hpp"

namespace vg
{
    RendererPostRenderTarget::RendererPostRenderTarget(uint32_t framebufferWidth
        , uint32_t framebufferHeight
        , vk::Format colorImageFormat
        , vk::Format depthStencilImageFormat
        )
        : PostRenderTarget(framebufferWidth, framebufferHeight)
    {
        _createObjs();
    }

    const Texture2DColorAttachment *RendererPostRenderTarget::getColorTargetTexture() const
    {
        return m_pColorTargetTex.get();
    }
        
    const TextureDepthStencilAttachment *RendererPostRenderTarget::getDepthStencilTargetTexture() const
    {
        return m_pDepthStencilTargetTex.get();
    }

    void RendererPostRenderTarget::_createObjs()
    {
        auto pDevice = pApp->getDevice();
        const auto framebufferWidth = m_framebufferWidth;
        const auto framebufferHeight = m_framebufferHeight;
        //color attachment
        auto pColorTex = new Texture2DColorAttachment(
            m_colorImageFormat,
            framebufferWidth,
            framebufferHeight
            );
        m_pColorTargetTex = std::shared_ptr<Texture2DColorAttachment>(pColorTex);
        m_pColorAttachment = m_pColorTargetTex->getImageView()->getImageView();

        auto pDepthStencilTex = new TextureDepthStencilAttachment(
            m_depthStencilImageFormat,
            framebufferWidth,
            framebufferHeight
            );
        m_pDepthStencilTargetTex = std::shared_ptr<TextureDepthStencilAttachment>(pDepthStencilTex);
        m_pDepthStencilAttachment = m_pDepthStencilTargetTex->getImageView()->getImageView();

        //render pass.
        auto pImage = m_pColorTargetTex->getImage();
        vk::AttachmentDescription colorAttachmentDes = {
            vk::AttachmentDescriptionFlags(),
            m_colorImageFormat,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            pImage->getInfo().layout,
        };

        pImage = m_pDepthStencilTargetTex->getImage();
        vk::AttachmentDescription depthStencilAttachmentDes = {
            vk::AttachmentDescriptionFlags(),
            m_depthStencilImageFormat,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eDontCare,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            pImage->getInfo().layout,
        };

        vk::AttachmentReference colorAttachmentRef = {
            0u,
            vk::ImageLayout::eColorAttachmentOptimal,
        };

        vk::AttachmentReference depthStencilAttachmentRef = {
            1u,
            vk::ImageLayout::eDepthStencilAttachmentOptimal,
        };

        vk::SubpassDescription subpass = {
            vk::SubpassDescriptionFlags(),       //flags
            vk::PipelineBindPoint::eGraphics,    //pipelineBindPoint
            0,                                   //inputAttachmentCount
            nullptr,                             //pInputAttachments
            1,                                   //colorAttachmentCount
            &colorAttachmentRef,                 //pColorAttachments
            nullptr,                             //pResolveAttachments
            &depthStencilAttachmentRef,          //pDepthStencilAttachment
            0,                                   //preserveAttachmentCount
            nullptr                              //pPreserveAttachments
        };

        std::array<vk::SubpassDependency, 2> dependencies = { 
            vk::SubpassDependency 
            {
                VK_SUBPASS_EXTERNAL,                                  
                0,                                                    
                vk::PipelineStageFlagBits::eTopOfPipe,                
                vk::PipelineStageFlagBits::eColorAttachmentOutput, 
                vk::AccessFlags(),                                    
                vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,       
                vk::DependencyFlagBits::eByRegion                     
            },
            vk::SubpassDependency
            {
                0,                                                    
                VK_SUBPASS_EXTERNAL,                                  
                vk::PipelineStageFlagBits::eColorAttachmentOutput,    
                vk::PipelineStageFlagBits::eBottomOfPipe,             
                vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
                vk::AccessFlags(),                      
                vk::DependencyFlagBits::eByRegion         
            }
        };

        std::array<vk::AttachmentDescription, 2> attachmentDess = { colorAttachmentDes, depthStencilAttachmentDes };
        vk::RenderPassCreateInfo renderPassCreateInfo = {
            vk::RenderPassCreateFlags(),
            static_cast<uint32_t>(attachmentDess.size()),
            attachmentDess.data(),
            1u,
            &subpass,
            static_cast<uint32_t>(dependencies.size()),
            dependencies.data()
        };

        m_pMyRenderPass = fd::createRenderPass(pDevice, renderPassCreateInfo);
        m_pRenderPass = m_pMyRenderPass.get();

        //frame buffer.
        std::array<vk::ImageView, 2> attachments = {
             *m_pColorAttachment,
             *m_pDepthStencilAttachment,
        };

        vk::FramebufferCreateInfo frameBufferCreateInfo = {
            vk::FramebufferCreateFlags(),      
            *m_pRenderPass,                                
            static_cast<uint32_t>(attachments.size()),      
            attachments.data(),                             
            framebufferWidth,                             
            framebufferHeight,                            
            1u,                                  
        };

        m_pMyFramebuffer = fd::createFrameBuffer(pDevice, frameBufferCreateInfo);
        m_pFramebuffer = m_pMyFramebuffer.get();
    }
} //vg