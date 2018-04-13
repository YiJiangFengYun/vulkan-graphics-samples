#ifndef VG_CMD_PARSER_HPP
#define VG_CMD_PARSER_HPP

#include "graphics/global.hpp"
#include "graphics/module.hpp"
#include "graphics/renderer/pipeline_cache.hpp"
#include "graphics/mesh/mesh.hpp"
#include "graphics/buffer_data/util.hpp"
#include "graphics/material/cmd.hpp"

namespace vg
{
    class CMDParser
    {
    public:
        struct ResultInfo
        {
			ResultInfo();
        };

        static void recordTrunk(CmdBuffer *pCmdBuffer
            , vk::CommandBuffer *pCommandBuffer
            , PipelineCache *pPipelineCache
            , vk::RenderPass *pRenderPass
            , ResultInfo *pResult = nullptr
            );
        static void recordTrunkWaitBarrier(CmdBuffer *pTrunkWaitBarrierCmdBuffer
            , vk::CommandBuffer *pCommandBuffer
            );
        
        static void recordBranch(CmdBuffer *pCmdBuffer
            , vk::CommandBuffer *pCommandBuffer
            , PipelineCache *pPipelineCache
            , ResultInfo *pResult = nullptr
            );

        static void recordItemRenderPassBegin(const RenderPassInfo *pRenderPassInfo
            ,  vk::CommandBuffer *pCommandBuffer);
        static void recordItemRenderPassEnd(const RenderPassInfo *pRenderPassInfo
            ,  vk::CommandBuffer *pCommandBuffer);
        static void recordItemRenderPassEnd(vk::CommandBuffer *pCommandBuffer);

        static void recordItemNextSubpass(vk::CommandBuffer *pCommandBuffer);

        static void recordItem(const RenderPassInfo *pRenderPassInfo
            ,  vk::CommandBuffer *pCommandBuffer
            , PipelineCache *pPipelineCache
            , ResultInfo *pResult = nullptr);

		static void _createPipeline(vk::RenderPass *pRenderPass,
		    const BaseMesh *pMesh,
		    uint32_t subMeshIndex,
		    Pass *pPass,
		    PipelineCache *pPipelineCache,
		    std::shared_ptr<vk::Pipeline> &pPipeline);

		static void _recordCommandBuffer(vk::Pipeline *pPipeline,
	        vk::CommandBuffer *pCommandBuffer,
		    uint32_t framebufferWidth,
		    uint32_t framebufferHeight,
		    const BaseMesh *pMesh,
		    uint32_t subMeshIndex,
		    Pass *pPass,
            const fd::Viewport *pViewport,
		    const fd::Rect2D *pClip);
    };
} //vg

#endif //VG_CMD_PARSER_HPP