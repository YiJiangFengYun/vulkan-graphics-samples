#include "graphics/material/material.hpp"

namespace vg
{
    Material::BindInfo::BindInfo(uint32_t framebufferWidth
        , uint32_t framebufferHeight
        , const Matrix4x4 *pProjMatrix
        , const Matrix4x4 *pViewMatrix
        , InstanceID objectID
        , const Matrix4x4 *pModelMatrix
        , const BaseMesh *pMesh
        , uint32_t subMeshIndex
        , Bool32 hasClipRect
        , const fd::Rect2D clipRect
#if defined(DEBUG) && defined(VG_ENABLE_COST_TIMER)
        , fd::CostTimer *pPreparingPipelineCostTimer
        , fd::CostTimer *pPreparingCommandBufferCostTimer
#endif //DEBUG and VG_ENABLE_COST_TIMER
        )
        : pProjMatrix(pProjMatrix)
        , pViewMatrix(pViewMatrix)
        , framebufferWidth(framebufferWidth)
        , framebufferHeight(framebufferHeight)
        , objectID(objectID)
        , pModelMatrix(pModelMatrix)
        , pMesh(pMesh)
        , subMeshIndex(subMeshIndex)
        , hasClipRect(hasClipRect)
        , clipRect(clipRect)
#if defined(DEBUG) && defined(VG_ENABLE_COST_TIMER)
        , pPreparingPipelineCostTimer(pPreparingPipelineCostTimer)
        , pPreparingCommandBufferCostTimer(pPreparingCommandBufferCostTimer)
#endif //DEBUG and VG_ENABLE_COST_TIMER
    {
    }

    Material::BindResult::BindResult(CmdBuffer *pBranchCmdBuffer
        , CmdBuffer *pTrunkRenderPassCmdBuffer
        , CmdBuffer *pTrunkWaitBarrierCmdBuffer
        )
        : pBranchCmdBuffer(pBranchCmdBuffer)
        , pTrunkRenderPassCmdBuffer(pTrunkRenderPassCmdBuffer)
        , pTrunkWaitBarrierCmdBuffer(pTrunkWaitBarrierCmdBuffer)
    {

    }

    Material::EndBindInfo::EndBindInfo(InstanceID objectID
        , uint32_t subMeshIndex
        )
        : objectID(objectID)
        , subMeshIndex(subMeshIndex)
    {

    }

    Material::MaterialCreateInfo::MaterialCreateInfo(Bool32 onlyOnce)
        : onlyOnce(onlyOnce)

    {

    }

    Material::Material(Bool32 onlyOnce)
        : Base(BaseType::MATERIAL)
        , name()
        , m_onlyOnce(onlyOnce)
        , m_pBindTargetID(std::shared_ptr<InstanceID>{new InstanceID(0)})
        , m_renderQueueType()
        , m_renderPriority()
        , m_arrPasses()
        , m_mapPasses()
        , m_pMainShader()
        , m_pMainPass()
        , m_pMyMainShader()
        , m_pMyMainPass()
    {
        m_pMyMainShader = std::shared_ptr<vg::Shader>{new vg::Shader()};
        m_pMyMainPass = std::shared_ptr<vg::Pass>{ new vg::Pass(m_pMyMainShader.get())};
        m_pMainShader = m_pMyMainShader.get();
        m_pMainPass = m_pMyMainPass.get();
        _addPass(m_pMainPass);
    }

    Material::Material(MaterialCreateInfo createInfo)
        : Material(createInfo.onlyOnce)
    {
    }

    Material::~Material()
    {
    }

    uint32_t Material::getPassCount() const
    {
        return static_cast<uint32_t>(m_arrPasses.size());
    }

    const Pass *Material::getPassWithIndex(uint32_t index) const
    {
        return m_arrPasses[index];
    }

    Pass *Material::getPassWithIndex(uint32_t index)
    {
        return m_arrPasses[index];
    }

    const Pass * const *Material::getPasses() const
    {
        return m_arrPasses.data();
    }

    Pass * const *Material::getPasses()
    {
        return m_arrPasses.data();
    }

    Bool32 Material::isHas(const Pass *pass) const
    {
        return m_mapPasses.find(pass->getID()) != m_mapPasses.cend();
    }


    const Shader *Material::getMainShader() const 
    {
        return m_pMainShader;
    }

    Shader *Material::getMainShader()
    {
        return m_pMainShader;
    }

    const Pass *Material::getMainPass() const
    {
        return m_pMainPass;
    }

    Pass *Material::getMainPass()
    {
        return m_pMainPass;
    }

    void Material::apply()
    {
        for (const auto& item : m_mapPasses)
        {
            item.second->apply();
        }
    }

    MaterialShowType Material::getShowType() const
    {
        return m_renderQueueType;
    }

    void Material::setRenderQueueType(MaterialShowType type)
    {
        m_renderQueueType = type;
    }

    uint32_t Material::getRenderPriority() const
    {
        return m_renderPriority;
    }

    void Material::setRenderPriority(uint32_t priority)
    {
        m_renderPriority = priority;
    }

    void Material::beginBind(const BindInfo info, BindResult *pResult) const
    {
        if (m_onlyOnce == VG_TRUE) {
            if (*m_pBindTargetID != 0) 
                throw std::runtime_error("The material binding is only once, but it was used to repeatedly bind.");
            *m_pBindTargetID = info.objectID;
        }

        _beginBind(info, pResult);

    }

    void Material::endBind(const EndBindInfo info) const
    {
        if (m_onlyOnce == VG_TRUE) {
            *m_pBindTargetID = 0;
        }

        _endBind(info);
        
    }

    void Material::_addPass(Pass *pPass)
    {
        if (isHas(pPass)) return;
        m_arrPasses.push_back(pPass);
        m_mapPasses[pPass->getID()] = pPass;
    }

    void Material::_removePass(Pass *pPass)
    {
        if (isHas(pPass) == VG_FALSE) return;
        m_arrPasses.erase(std::remove(m_arrPasses.begin(), m_arrPasses.end(), pPass));
        m_mapPasses.erase(pPass->getID());
    }

    void Material::_beginBind(const BindInfo info, BindResult *pResult) const
    {
        if (pResult->pTrunkRenderPassCmdBuffer != nullptr)
        {
            auto &result = *pResult;
            RenderPassInfo trunkRenderPassInfo;
            trunkRenderPassInfo.pRenderPass = nullptr;
            trunkRenderPassInfo.pFramebuffer = nullptr;
            trunkRenderPassInfo.framebufferWidth = info.framebufferWidth;
            trunkRenderPassInfo.framebufferHeight = info.framebufferHeight;
            trunkRenderPassInfo.projMatrix = *(info.pProjMatrix);
            trunkRenderPassInfo.viewMatrix = *(info.pViewMatrix);
            trunkRenderPassInfo.pPass = *m_arrPasses.data();
            trunkRenderPassInfo.modelMatrix = *(info.pModelMatrix);
            trunkRenderPassInfo.pMesh = info.pMesh;
            trunkRenderPassInfo.subMeshIndex = info.subMeshIndex;
            trunkRenderPassInfo.viewport = fd::Viewport();
            trunkRenderPassInfo.scissor = info.hasClipRect ? info.clipRect : fd::Rect2D();
            trunkRenderPassInfo.objectID = info.objectID;
            CmdInfo cmdInfo;
            cmdInfo.pRenderPassInfo = &trunkRenderPassInfo;
            result.pTrunkRenderPassCmdBuffer->addCmd(cmdInfo);
        }
    }
    
    void Material::_endBind(const EndBindInfo info) const
    {

    }
}