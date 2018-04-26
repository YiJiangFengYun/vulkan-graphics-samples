#include "graphics_ext/material/material_outline.hpp"

namespace vge
{
    MaterialOutline::CreateInfo::CreateInfo(vg::Color outlineColor)
        : outlineColor(outlineColor)
    {
    }

    MaterialOutline::MaterialOutline(CreateInfo info)
        : vg::Material()
        , m_outlineInfoVert()
        , m_outlineInfoFrag()
    {
        _createOtherPasses();
        _initPasses(); 
    }

    void MaterialOutline::apply()
    {
        _applyInfos();
        vg::Material::apply();
    }

    vg::Pass * MaterialOutline::getPassOutline() const
    {
        return m_pPassOutline.get();
    }

    float MaterialOutline::getOutlineWidth() const
    {
        return m_outlineInfoVert.outlineWidth;
    }

    void MaterialOutline::setOutlineWidth(float value)
    {
        value = std::max(0.0f, value);
        m_outlineInfoVert.outlineWidth = value;
    }

    vg::Color MaterialOutline::getOutlineColor() const
    {
        return m_outlineInfoFrag.outlineColor;
    }

    void MaterialOutline::setOutlineColor(vg::Color value)
    {
        m_outlineInfoFrag.outlineColor = value;
    }

    void MaterialOutline::_beginBindToRender(const BindInfo info, BindResult *pResult)
    {
        {
            vg::RenderPassInfo trunkRenderPassInfo;
            trunkRenderPassInfo.pRenderPass = nullptr;
            trunkRenderPassInfo.pFrameBuffer = nullptr;
            trunkRenderPassInfo.framebufferWidth = info.trunkFramebufferWidth;
            trunkRenderPassInfo.framebufferHeight = info.trunkFramebufferHeight;
            trunkRenderPassInfo.projMatrix = *(info.pProjMatrix);
            trunkRenderPassInfo.viewMatrix = *(info.pViewMatrix);
            trunkRenderPassInfo.pPass = m_pMainPass.get();
            trunkRenderPassInfo.modelMatrix = *(info.pModelMatrix);
            trunkRenderPassInfo.pMesh = info.pMesh;
            trunkRenderPassInfo.subMeshIndex = info.subMeshIndex;
            trunkRenderPassInfo.viewport = fd::Viewport();
            trunkRenderPassInfo.scissor = info.hasClipRect ? info.clipRect : fd::Rect2D();

            vg::CmdInfo cmdInfo;
            cmdInfo.pRenderPassInfo = &trunkRenderPassInfo;

            pResult->pTrunkRenderPassCmdBuffer->addCmd(cmdInfo);
        }

        {
            vg::RenderPassInfo trunkRenderPassInfo;
            trunkRenderPassInfo.pRenderPass = nullptr;
            trunkRenderPassInfo.pFrameBuffer = nullptr;
            trunkRenderPassInfo.framebufferWidth = info.trunkFramebufferWidth;
            trunkRenderPassInfo.framebufferHeight = info.trunkFramebufferHeight;
            trunkRenderPassInfo.projMatrix = *(info.pProjMatrix);
            trunkRenderPassInfo.viewMatrix = *(info.pViewMatrix);
            trunkRenderPassInfo.pPass = m_pPassOutline.get();
            trunkRenderPassInfo.modelMatrix = *(info.pModelMatrix);
            trunkRenderPassInfo.pMesh = info.pMesh;
            trunkRenderPassInfo.subMeshIndex = info.subMeshIndex;
            trunkRenderPassInfo.viewport = fd::Viewport();

            fd::Rect2D clipRect = fd::Rect2D();
            if (info.hasClipRect)
            {
                clipRect.x = info.clipRect.x - m_outlineInfoVert.outlineWidth;
                clipRect.y = info.clipRect.y - m_outlineInfoVert.outlineWidth;
                clipRect.width = info.clipRect.width + 2 * m_outlineInfoVert.outlineWidth;
                clipRect.height = info.clipRect.height + 2 * m_outlineInfoVert.outlineWidth;
            }

            trunkRenderPassInfo.scissor = clipRect;

            vg::CmdInfo cmdInfo;
            cmdInfo.pRenderPassInfo = &trunkRenderPassInfo;

            pResult->pTrunkRenderPassCmdBuffer->addCmd(cmdInfo);
        }
    }

    void MaterialOutline::_createOtherPasses()
    {
        m_pShaderOutline = std::shared_ptr<vg::Shader>{new vg::Shader()};
	    m_pPassOutline = std::shared_ptr<vg::Pass>{ new vg::Pass(m_pShaderOutline.get())};
        _addPass(m_pPassOutline.get());
    }

    void MaterialOutline::_initPasses()
    {
        //outline pass
        {
            auto pPass = m_pPassOutline.get();
	        vg::Pass::BuildInDataInfo::Component buildInDataCmps[1] = {
	    		{vg::Pass::BuildInDataType::MATRIX_OBJECT_TO_NDC},
	    	};
	        vg::Pass::BuildInDataInfo buildInDataInfo;
	        buildInDataInfo.componentCount = 1u;
	        buildInDataInfo.pComponent = buildInDataCmps;
	        pPass->setBuildInDataInfo(buildInDataInfo);

            _applyInfos();
                
            pPass->apply();
        }
    }

    void MaterialOutline::_applyInfos()
    {
        auto pPass = m_pPassOutline.get();
        pPass->setDataValue("outline_info_vert", m_outlineInfoVert, 1u, 
            vg::DescriptorType::UNIFORM_BUFFER, vg::ShaderStageFlagBits::VERTEX);
        pPass->setDataValue("outline_info_frag", m_outlineInfoFrag, 2u,
            vg::DescriptorType::UNIFORM_BUFFER, vg::ShaderStageFlagBits::FRAGMENT);
    }
} //vge