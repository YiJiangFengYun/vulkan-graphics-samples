#include "graphics/material/cmd.hpp"

#include "graphics/util/util.hpp"
namespace vg
{
    template <typename T, typename ParentT, uint32_t parentPointOffset>
    void addData(uint32_t &count
         , uint32_t &capacity
         , std::vector<T> &arr
         , std::vector<uint32_t> *pToParentIndices
         , std::vector<ParentT> *pParentArr
		 , uint32_t parentArrCount
         , const T &newData
         )
    {
        if (count == capacity)
        {
            capacity = getNextCapacity(capacity);
            arr.resize(capacity);
            if (pParentArr != nullptr) {
                pToParentIndices->resize(capacity);
			    for (uint32_t i = 0; i < count; ++i)
			    {
			    	auto &parent = (*pParentArr)[(*pToParentIndices)[i]];
                    auto point = &(arr[i]);
                    memcpy((char *)(&parent) + parentPointOffset, &point, sizeof(point));
			    }
            }
        }
        auto &dstData = arr[count];
        dstData = newData;
        if (pParentArr != nullptr) {
            auto pDst = &dstData;
			uint32_t parentIndex = parentArrCount - 1;
            ParentT * pParent = &((*pParentArr)[parentIndex]);
            memcpy((char *)pParent + parentPointOffset, &pDst, sizeof(pDst));
            (*pToParentIndices)[count] = parentIndex;
        }
        ++count;
    }

    template <typename T, typename ParentT, uint32_t parentPointOffset>
    void addData(uint32_t &count
         , uint32_t &capacity
         , std::vector<std::vector<T>> &arr
         , std::vector<ParentT> *pParentArr
		 , uint32_t parentArrCount
         , uint32_t dataCount
         , const T *pDatas
         )
    {
        if (count == capacity)
        {
            capacity = getNextCapacity(capacity);
            arr.resize(capacity);
        }
        auto &dstData = arr[count];
        dstData.resize(dataCount);
        memcpy(dstData.data(), pDatas, dataCount * sizeof(T));
        if (pParentArr != nullptr) {
            auto pDst = dstData.data();
			uint32_t parentIndex = parentArrCount - 1;
            ParentT * pParent = &((*pParentArr)[parentIndex]);
            memcpy((char *)pParent + parentPointOffset, &pDst, sizeof(pDst));
        }
        ++count;
    }

    CmdInfo::CmdInfo()
        : pRenderPassInfo(nullptr)
        , pBarrierInfo(nullptr)
    {

    }

    BarrierInfo::BarrierInfo(vk::PipelineStageFlags srcStageMask
        , vk::PipelineStageFlags  dstStageMask
        , vk::DependencyFlags dependencyFlags
        , uint32_t memoryBarrierCount
        , const vk::MemoryBarrier * pMemoryBarriers
        , uint32_t bufferMemoryBarrierCount
        , const vk::BufferMemoryBarrier * pBufferMemoryBarriers
        , uint32_t imageMemoryBarrierCount
        , const vk::ImageMemoryBarrier * pImageMemoryBarriers
        )
        : srcStageMask(srcStageMask)
        , dstStageMask(dstStageMask)
        , dependencyFlags(dependencyFlags)
        , memoryBarrierCount(memoryBarrierCount)
        , pMemoryBarriers(pMemoryBarriers)
        , bufferMemoryBarrierCount(bufferMemoryBarrierCount)
        , pBufferMemoryBarriers(pBufferMemoryBarriers)
        , imageMemoryBarrierCount(imageMemoryBarrierCount)
        , pImageMemoryBarriers(pImageMemoryBarriers)
    {

    }

    CmdDraw::CmdDraw(uint32_t vertexCount
        , uint32_t instanceCount
        , uint32_t firstVertex
        , uint32_t firstInstance
        )
        : vertexCount(vertexCount)
        , instanceCount(instanceCount)
        , firstVertex(firstVertex)
        , firstInstance(firstInstance)
    {

    }

    CmdDrawIndexed::CmdDrawIndexed(uint32_t indexCount
        , uint32_t instanceCount
        , uint32_t firstIndex
        , uint32_t vertexOffset
        , uint32_t firstInstance
        )
        : indexCount(indexCount)
        , instanceCount(instanceCount)
        , firstIndex(firstIndex)
        , vertexOffset(vertexOffset)
        , firstInstance(firstInstance)
    {

    }

    RenderPassInfo::RenderPassInfo(const vk::RenderPass *pRenderPass
        , uint32_t subPassIndex
        , const vk::Framebuffer *pFrameBuffer
        , uint32_t framebufferWidth
        , uint32_t framebufferHeight
        , fd::Rect2D renderArea
        , uint32_t clearValueCount
        , const vk::ClearValue *pClearValues
        , Matrix4x4 projMatrix
        , Matrix4x4 viewMatrix
        , const Pass *pPass
        , Matrix4x4 modelMatrix
        , const BaseMesh *pMesh
        , uint32_t subMeshIndex
        , fd::Viewport viewport
        , fd::Rect2D scissor
        , const CmdDraw *pCmdDraw
        , const CmdDrawIndexed *pCmdDrawIndexed
#if defined(DEBUG) && defined(VG_ENABLE_COST_TIMER)
        , fd::CostTimer *pPreparingBuildInDataCostTimer
        , fd::CostTimer *pPreparingPipelineCostTimer
        , fd::CostTimer *pPreparingCommandBufferCostTimer
#endif //DEBUG and VG_ENABLE_COST_TIMER
        )
        : pRenderPass(pRenderPass)
        , subPassIndex(subPassIndex)
        , pFrameBuffer(pFrameBuffer)
	    , framebufferWidth(framebufferWidth)
	    , framebufferHeight(framebufferHeight)
        , renderArea(renderArea)
        , clearValueCount(clearValueCount)
        , pClearValues(pClearValues)
        , projMatrix(projMatrix)
        , viewMatrix(viewMatrix)
        , pPass(pPass)
        , modelMatrix(modelMatrix)
        , pMesh(pMesh)
        , subMeshIndex(subMeshIndex)
        , viewport(viewport)
        , scissor(scissor)
        , pCmdDraw(pCmdDraw)
        , pCmdDrawIndexed(pCmdDrawIndexed)
#if defined(DEBUG) && defined(VG_ENABLE_COST_TIMER)
        , pPreparingBuildInDataCostTimer(pPreparingBuildInDataCostTimer)
        , pPreparingPipelineCostTimer(pPreparingPipelineCostTimer)
        , pPreparingCommandBufferCostTimer(pPreparingCommandBufferCostTimer)
#endif //DEBUG and VG_ENABLE_COST_TIMER
    {    
    }


    CmdBuffer::CmdBuffer()
        : m_cmdInfoCount(0u)
        , m_cmdInfoCapacity(0u)
        , m_cmdInfos()

        , m_renderPassInfoCount(0u)
        , m_renderPassInfoCapacity(0u)
        , m_renderPassInfos()
		, m_renderPassInfoToCmdInfoIndices()

        , m_clearValuesCount(0u)
        , m_clearValuesCapacity(0u)
        , m_clearValues()

        , m_cmdDrawCount(0u)
        , m_cmdDrawCapacity(0u)
        , m_cmdDraws()
		, m_cmdDrawToRenderPassInfoIndices()

        , m_cmdDrawIndexedCount(0u)
        , m_cmdDrawIndexedCapacity(0u)
        , m_cmdDrawIndexeds()
		, m_cmdDrawIndexedToRenderPassInfoIndices()

        , m_barrierInfoCount(0u)
        , m_barrierInfosCapacity(0u)
        , m_barrierInfos()
		, m_barrierInfoToCmdInfoIndices()

        , m_memoryBarrierCount(0u)
        , m_memoryBarrierCapacity(0u)
        , m_memoryBarriers()

        , m_bufferMemoryBarrierCount(0u)
        , m_bufferMemoryBarrierCapacity(0u)
        , m_bufferMemoryBarriers()

        , m_imageMemoryBarrierCount(0u)
        , m_imageMemoryBarrierCapacity(0u)
        , m_imageMemoryBarriers()
    {    
    }    
    uint32_t CmdBuffer::getCmdCount() const
    {
        return m_cmdInfoCount;
    }    
    const CmdInfo *CmdBuffer::getCmdInfos() const
    {
        return m_cmdInfos.data();
    }    
    void CmdBuffer::begin()
    {
        m_cmdInfoCount =  0u;

        m_renderPassInfoCount = 0u;
        m_cmdDrawCount = 0u;
        m_cmdDrawIndexedCount = 0u;

        m_barrierInfoCount = 0u;
        m_memoryBarrierCount = 0u;
        m_bufferMemoryBarrierCount = 0u;
        m_imageMemoryBarrierCount = 0u;
    }    
    void CmdBuffer::addCmd(CmdInfo cmdInfo)
    {
        addData<CmdInfo, nullptr_t, 0>(m_cmdInfoCount, 
            m_cmdInfoCapacity,
            m_cmdInfos,
            nullptr,
            nullptr,
			0,
            cmdInfo
            );

        //copy render pass info
        if (cmdInfo.pRenderPassInfo != nullptr)
        {
            addData<RenderPassInfo, CmdInfo, offsetof(CmdInfo, pRenderPassInfo)>(
                m_renderPassInfoCount,
                m_renderPassInfoCapacity,
                m_renderPassInfos,
                &m_renderPassInfoToCmdInfoIndices,
                &m_cmdInfos,
				m_cmdInfoCount,
                *(cmdInfo.pRenderPassInfo)
            );

            auto &renderPassInfo = *(cmdInfo.pRenderPassInfo);
            
            auto clearValueCount = renderPassInfo.clearValueCount;
            auto pClearValues = renderPassInfo.pClearValues;
            addData<vk::ClearValue, RenderPassInfo, offsetof(RenderPassInfo, pClearValues)>(
                m_clearValuesCount,
                m_clearValuesCapacity,
                m_clearValues,
                &m_renderPassInfos,
				m_renderPassInfoCount,
                clearValueCount,
                pClearValues
            );

            if (renderPassInfo.pCmdDraw != nullptr) {
                addData<CmdDraw, RenderPassInfo, offsetof(RenderPassInfo, pCmdDraw)>(
                    m_cmdDrawCount,
                    m_cmdDrawCapacity,
                    m_cmdDraws,
                    &m_cmdDrawToRenderPassInfoIndices,
                    &m_renderPassInfos,
                    m_renderPassInfoCount,
                    *(renderPassInfo.pCmdDraw)
                );
            }
            if (renderPassInfo.pCmdDrawIndexed != nullptr) {
                addData<CmdDrawIndexed, RenderPassInfo, offsetof(RenderPassInfo, pCmdDrawIndexed)>(
                    m_cmdDrawIndexedCount,
                    m_cmdDrawIndexedCapacity,
                    m_cmdDrawIndexeds,
                    &m_cmdDrawIndexedToRenderPassInfoIndices,
                    &m_renderPassInfos,
                    m_renderPassInfoCount,
                    *(renderPassInfo.pCmdDrawIndexed)
                );
            }
        }

        //copy barrier info
        if (cmdInfo.pBarrierInfo != nullptr)
        {
            addData<BarrierInfo, CmdInfo, offsetof(CmdInfo, pBarrierInfo)>(
                m_barrierInfoCount,
                m_barrierInfosCapacity,
                m_barrierInfos,
                &m_barrierInfoToCmdInfoIndices,
                &m_cmdInfos,
				m_cmdInfoCount,
                *(cmdInfo.pBarrierInfo)
            );

            auto memoryBarrierCount = cmdInfo.pBarrierInfo->memoryBarrierCount;
            auto pMemoryBarriers = cmdInfo.pBarrierInfo->pMemoryBarriers;
            addData<vk::MemoryBarrier, BarrierInfo, offsetof(BarrierInfo, pMemoryBarriers)>(
                m_memoryBarrierCount,
                m_memoryBarrierCapacity,
                m_memoryBarriers,
                &m_barrierInfos,
				m_barrierInfoCount,
                memoryBarrierCount,
                pMemoryBarriers
            );
            
            auto bufferMemoryBarrierCount = cmdInfo.pBarrierInfo->bufferMemoryBarrierCount;
            auto pBufferMemoryBarriers = cmdInfo.pBarrierInfo->pBufferMemoryBarriers;
            addData<vk::BufferMemoryBarrier, BarrierInfo, offsetof(BarrierInfo, pBufferMemoryBarriers)>(
                m_bufferMemoryBarrierCount,
                m_bufferMemoryBarrierCapacity,
                m_bufferMemoryBarriers,
                &m_barrierInfos,
				m_barrierInfoCount,
                bufferMemoryBarrierCount,
                pBufferMemoryBarriers
            );

            auto imageMemoryBarrierCount = cmdInfo.pBarrierInfo->imageMemoryBarrierCount;
            auto pImageMemoryBarriers = cmdInfo.pBarrierInfo->pImageMemoryBarriers;
            addData<vk::ImageMemoryBarrier, BarrierInfo, offsetof(BarrierInfo, pImageMemoryBarriers)>(
                m_imageMemoryBarrierCount,
                m_imageMemoryBarrierCapacity,
                m_imageMemoryBarriers,
                &m_barrierInfos,
				m_barrierInfoCount,
                imageMemoryBarrierCount,
                pImageMemoryBarriers
            );
        }
    }
    
    void CmdBuffer::end()
    {    
    }
} //vg