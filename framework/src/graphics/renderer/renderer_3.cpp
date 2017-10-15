#include "graphics/renderer/renderer_3.hpp"

#include <functional>

namespace kgs
{
	Renderer3::Renderer3(std::shared_ptr<vk::ImageView> pSwapchainImageView
		, vk::Format swapchainImageFormat
		, uint32_t swapchainImageWidth
		, uint32_t swapchainImageHeight
	)
		: Renderer(pSwapchainImageView
			, swapchainImageFormat
		    , swapchainImageWidth
		    , swapchainImageHeight)
	{

	}

	Renderer3::Renderer3(std::shared_ptr<vk::ImageView> pSwapchainImageView
		, vk::Format swapchainImageFormat
		, uint32_t swapchainImageWidth
		, uint32_t swapchainImageHeight
		, std::shared_ptr<SceneType> pScene
		, std::shared_ptr<CameraType> pCamera
	)
		: Renderer(pSwapchainImageView
			, swapchainImageFormat
			, swapchainImageWidth
			, swapchainImageHeight
			, pScene
			, pCamera)
	{

	}

	Renderer3::Renderer3(std::shared_ptr<TextureColorAttachment> pColorAttachmentTex
	)
		: Renderer(pColorAttachmentTex)
	{

	}

	Renderer3::Renderer3(std::shared_ptr<TextureColorAttachment> pColorAttachmentTex
		, std::shared_ptr<SceneType> pScene
		, std::shared_ptr<CameraType> pCamera
	)
		: Renderer(pColorAttachmentTex
			, pScene
			, pCamera)
	{

	}

	void Renderer3::_render(RenderInfo renderInfo)
	{
		auto queueTypeCount = static_cast<uint32_t>(RenderQueueType::RANGE_SIZE);

		Renderer::_render(renderInfo);

		//std::shared_ptr<Scene<SpaceType::SPACE_3>> pScene;
		auto pScene = m_pScene;
		uint32_t visualObjectCount = pScene->getVisualObjectCount();
		
		//----------Preparing render.

		//Filter visualObject is out of camera with its bounds.
		std::vector<std::shared_ptr<SceneType::VisualObjectType>> validVisualObjects(visualObjectCount); //allocate enough space for array to storage points.
		uint32_t validVisualObjectCount(0u);
		for (uint32_t i = 0; i < visualObjectCount; ++i)
		{
			auto pVisualObject = pScene->getVisualObjectWithIndex(i);
			if (_checkVisualObjectInsideCameraView(pVisualObject) == KGS_TRUE)
			{
				validVisualObjects[validVisualObjectCount++] = pVisualObject;
			}
		}

		//Get queue count for each queue type.
		std::vector<uint32_t> queueLengths(queueTypeCount, 0u);
		for (uint32_t i = 0; i < validVisualObjectCount; ++i)
		{
			auto pVisualObject = validVisualObjects[i];
			auto renderQueueType = tranMaterialShowTypeToRenderQueueType(pVisualObject->getMaterial()->getShowType());
			++queueLengths[static_cast<size_t>(renderQueueType)];
		}

		std::vector<std::vector<std::shared_ptr<SceneType::VisualObjectType>>> queues(queueTypeCount);
		//Resize queues and reset quue counts to zero for preparing next use.
		for (uint32_t i = 0; i < queueTypeCount; ++i)
		{
			queues[i].resize(queueLengths[i], nullptr);
			queueLengths[i] = 0u;
		}

		//Update queues to point to visual object.
		for (uint32_t i = 0; i < validVisualObjectCount; ++i)
		{
			auto pVisualObject = validVisualObjects[i];
			auto renderQueueType = tranMaterialShowTypeToRenderQueueType(pVisualObject->getMaterial()->getShowType());
			queues[static_cast<size_t>(renderQueueType)][queueLengths[static_cast<size_t>(renderQueueType)]++] = pVisualObject;
		}

		//sort transparent queue.
		std::sort(queues[static_cast<size_t>(RenderQueueType::TRANSPARENT)].begin(),
			queues[static_cast<size_t>(RenderQueueType::TRANSPARENT)].end(),
			std::bind(&Renderer3::_sortObjectsWithCameraZ, this, std::placeholders::_1, std::placeholders::_2));


		//-----Doing render.
		for (uint32_t typeIndex = 0u; typeIndex < queueTypeCount; ++typeIndex)
		{
			auto queueLength = queueLengths[typeIndex];
			for (uint32_t objectIndex = 0u; objectIndex < queueLength; ++objectIndex)
			{
				auto pVisualObject = queues[typeIndex][objectIndex];
				auto pMesh = pVisualObject->getMesh();
				auto subMeshCount = pMesh->getSubMeshCount();
				auto pMaterial = pVisualObject->getMaterial();
				auto passCount = pMaterial->getPassCount();
				for (uint32_t subMeshIndex = 0u; subMeshIndex < subMeshCount; ++subMeshIndex)
				{
					for (uint32_t passIndex = 0u; passIndex < passCount; ++passIndex)
					{
						std::shared_ptr<vk::PipelineLayout> pPipelineLayout;
						std::shared_ptr<vk::Pipeline> pPipeline;
						_createPipelineForRender(pPipelineLayout, pPipeline, renderInfo, pMesh, pMaterial, subMeshIndex, passIndex);
					}
				}
			}
		}


	}

	Bool32 Renderer3::_checkVisualObjectInsideCameraView(std::shared_ptr<typename SceneType::VisualObjectType> pVisualObject)
	{
		//get bounds and its 8 points.
		auto pMesh = pVisualObject->getMesh();
		auto bounds = dynamic_cast<SceneType::VisualObjectType::MeshType *>(pMesh.get())->getBounds();
		auto min = bounds.getMin();
		auto max = bounds.getMax();
		typedef SpaceTypeInfo<SpaceType::SPACE_3>::PointType PointType;
		typedef SpaceTypeInfo<SpaceType::SPACE_3>::MatrixVectorType MatrixVectorType;
		PointType p0(min);
		PointType p1(max.x, min.y, min.z);
		PointType p2(min.x, max.y, min.z);
		PointType p3(min.x, min.y, max.z);
		PointType p4(max.x, max.y, min.z);
		PointType p5(max.x, min.y, max.z);
		PointType p6(min.x, max.y, max.z);
		PointType p7(max);
		const uint32_t pointCount = 8;
		PointType points[pointCount] = { p0, p1, p2, p3, p4, p5, p6, p7 };

		//get MVP matrix.
		auto mvpMatrix = _getMVPMatrix(pVisualObject);

		Bool32 isInsideCameraView = KGS_FALSE;
		for (uint32_t i = 0; i < pointCount; ++i)
		{
			PointType p = points[i];
			//transform point from model coordinate system to normalize device coordinate system.
			p = mvpMatrix * MatrixVectorType(p, 1.0f);

			//check if point is inside camera view.
			if ((p.x > -1 && p.x < 1)
				&& (p.y > -1 && p.y < 1)
				&& (p.z > 0 && p.z < 1)
				)
			{
				isInsideCameraView = KGS_TRUE;
				//Bounds is inside camera view as long as one of Points is inside camera view.
				break;
			}
		}

		return isInsideCameraView;
	}

	Bool32 Renderer3::_sortObjectsWithCameraZ(std::shared_ptr<typename SceneType::ObjectType> pObject1, std::shared_ptr<typename SceneType::ObjectType> pObject2)
	{
		//get MV matrix.
		auto mvMatrix1 = _getMVMatrix(pObject1);
		auto mvMatrix2 = _getMVMatrix(pObject2);
		//get position
		auto pos1 = pObject1->getTransform().getLocalPosition();
		auto pos2 = pObject2->getTransform().getLocalPosition();

		typedef SpaceTypeInfo<SpaceType::SPACE_3>::PointType PointType;
		typedef SpaceTypeInfo<SpaceType::SPACE_3>::MatrixVectorType MatrixVectorType;
		//transform point from model coordinate system to camera coordinate system.
		pos1 = mvMatrix1 * MatrixVectorType(pos1, 1.0f);
		pos2 = mvMatrix2 * MatrixVectorType(pos2, 1.0f);

		//it is smaller if its z is bigger than the other.
		return static_cast<Bool32>(pos1.z > pos2.z);

	}
} //namespace kgs