#include "graphics/scene/visual_object.hpp"

namespace vg
{
	BaseVisualObject::BindInfo::BindInfo(uint32_t trunkFramebufferWidth
		, uint32_t trunkFramebufferHeight
		, const Matrix4x4 *pProjMatrix
        , const Matrix4x4 *pViewMatrix
#if defined(DEBUG) && defined(VG_ENABLE_COST_TIMER)
        , fd::CostTimer *pPreparingBuildInDataCostTimer
        , fd::CostTimer *pPreparingPipelineCostTimer
        , fd::CostTimer *pPreparingCommandBufferCostTimer
#endif //DEBUG and VG_ENABLE_COST_TIMER
        )
        : trunkFramebufferWidth(trunkFramebufferWidth)
		, trunkFramebufferHeight(trunkFramebufferHeight)
		, pProjMatrix(pProjMatrix)
        , pViewMatrix(pViewMatrix)
		
#if defined(DEBUG) && defined(VG_ENABLE_COST_TIMER)
        , pPreparingBuildInDataCostTimer(pPreparingBuildInDataCostTimer)
        , pPreparingPipelineCostTimer(pPreparingPipelineCostTimer)
        , pPreparingCommandBufferCostTimer(pPreparingCommandBufferCostTimer)
#endif //DEBUG and VG_ENABLE_COST_TIMER
    {
    }

    BaseVisualObject::BindResult::BindResult(CmdBuffer *pBranchCmdBuffer
        , CmdBuffer *pTrunkRenderPassCmdBuffer
        , CmdBuffer *pTrunkWaitBarrierCmdBuffer
        )
	    : pBranchCmdBuffer(pBranchCmdBuffer)
        , pTrunkRenderPassCmdBuffer(pTrunkRenderPassCmdBuffer)
        , pTrunkWaitBarrierCmdBuffer(pTrunkWaitBarrierCmdBuffer)
    {

    }

	BaseVisualObject::BaseVisualObject()
	    : m_pMaterial()
		, m_pMesh()
		, m_subMeshOffset(-1)
		, m_subMeshCount(-1)
		, m_isVisibilityCheck(VG_TRUE)
		, m_hasClipRect(VG_FALSE)
		, m_clipRects()
	{

	}

	BaseVisualObject::~BaseVisualObject()
	{

	}

	Material *BaseVisualObject::getMaterial() const
	{
		return m_pMaterial;
	}

	void BaseVisualObject::setMaterial(Material *pMaterial)
	{
		m_pMaterial = pMaterial;
	}

	BaseMesh *BaseVisualObject::getMesh() const
	{
		return m_pMesh;
	}

	uint32_t BaseVisualObject::getSubMeshOffset() const
	{
		if (m_subMeshOffset < 0) {
            return dynamic_cast<const ContentMesh *>(m_pMesh)->getSubMeshOffset();
		} else {
		    return m_subMeshOffset;
		}
	}
		
	uint32_t BaseVisualObject::getSubMeshCount() const
	{
		if (m_subMeshCount < 0) {
		    return dynamic_cast<const ContentMesh *>(m_pMesh)->getSubMeshCount();
		} else {
            return m_subMeshCount;
		}
	}

	void BaseVisualObject::updateSubMeshInfo(uint32_t subMeshOffset, uint32_t subMeshCount)
	{
		m_subMeshOffset = subMeshOffset;
		m_subMeshCount = static_cast<int32_t>(subMeshCount);
		m_clipRects.resize(subMeshCount);
	}

	Bool32 BaseVisualObject::getIsVisibilityCheck() const
	{
		return m_isVisibilityCheck;
	}

	void BaseVisualObject::setIsVisibilityCheck(Bool32 value)
	{
		m_isVisibilityCheck = value;
	}

	Bool32 BaseVisualObject::getHasClipRect() const
	{
		return m_hasClipRect;
	}

	void BaseVisualObject::setHasClipRect(Bool32 value)
	{
		m_hasClipRect = value;
	}

	uint32_t BaseVisualObject::getClipRectCount() const
	{
		return static_cast<uint32_t>(m_clipRects.size());
	}
		
	const fd::Rect2D *BaseVisualObject::getClipRects() const
	{
		return m_clipRects.data();
	}

	void BaseVisualObject::updateClipRects(fd::ArrayProxy<fd::Rect2D> rects, uint32_t count, uint32_t offset)
	{
		_asyncMeshData();
		for (uint32_t i = 0; i < count; ++i)
        {
            m_clipRects[offset] = *(rects.data() + i);
            ++offset;
        }
	}
		
	void BaseVisualObject::updateClipRects(fd::Rect2D rect, uint32_t count, uint32_t offset)
	{
		_asyncMeshData();
		for (uint32_t i = 0; i < count; ++i)
        {
			m_clipRects[offset] = rect;
            ++offset;
        }
	}

	void BaseVisualObject::bindToRender(const BindInfo info, BindResult *pResult)
	{
		auto &result = *pResult;
		uint32_t branchCmdCount = 0u;
		uint32_t trunkRenderPassCount = 0u;
		uint32_t trunkWaitBarrierCount = 0u;
		if (m_pMaterial != nullptr)
		{
			auto modelMatrix = _getModelMatrix();

			uint32_t subMeshCount = getSubMeshCount();
			uint32_t subMeshOffset = getSubMeshOffset();
			
			for (uint32_t i = 0; i < subMeshCount; ++i)
			{
				uint32_t subMeshIndex = subMeshOffset + i;
				Material::BindInfo infoForVisualizer = {
			        info.pProjMatrix,
			        info.pViewMatrix,
				    info.trunkFramebufferWidth,
				    info.trunkFramebufferHeight,
                    &modelMatrix,
			        m_pMesh,
			        subMeshIndex,
			        m_hasClipRect,
			        *(m_clipRects.data() + i),
#if defined(DEBUG) && defined(VG_ENABLE_COST_TIMER)
                    info.pPreparingBuildInDataCostTimer,
                    info.pPreparingPipelineCostTimer,
                    info.pPreparingCommandBufferCostTimer,
#endif //DEBUG and VG_ENABLE_COST_TIMER
		            };
    
		        Material::BindResult resultForVisualizer;
				
				if (result.pBranchCmdBuffer != nullptr)
					resultForVisualizer.pBranchCmdBuffer = result.pBranchCmdBuffer;
				if (result.pTrunkRenderPassCmdBuffer != nullptr)
					resultForVisualizer.pTrunkRenderPassCmdBuffer = result.pTrunkRenderPassCmdBuffer;
				if (result.pTrunkWaitBarrierCmdBuffer != nullptr)
				    resultForVisualizer.pTrunkWaitBarrierCmdBuffer = result.pTrunkWaitBarrierCmdBuffer;

		        m_pMaterial->bindToRender(infoForVisualizer, &resultForVisualizer);

			}
		}
		
	}

	void BaseVisualObject::_asyncMeshData()
	{
		if (m_subMeshCount < 0)
		{
			if (m_clipRects.size() < dynamic_cast<const ContentMesh *>(m_pMesh)->getSubMeshCount())
			{
				m_clipRects.resize(dynamic_cast<const ContentMesh *>(m_pMesh)->getSubMeshCount());
			}
		}
	}

} //namespace kgs