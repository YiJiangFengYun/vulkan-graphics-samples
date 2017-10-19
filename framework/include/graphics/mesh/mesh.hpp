#ifndef KGS_MESH_H
#define KGS_MESH_H

#include <set>
#include <utility>
#include <glm/glm.hpp>
#include <foundation/foundation.hpp>
#include "graphics/global.hpp"
#include "graphics/util/find_memory.hpp"
#include "graphics/util/single_time_command.hpp"
#include "graphics/mesh/mesh_option.hpp"
#include "graphics/mesh/mesh_data.hpp"

namespace kgs
{
	class BaseMesh
	{
	public:
		struct LayoutBindingInfo
		{
			std::string name;
			MeshData::DataType dataType;
			uint32_t bindingPriority;

			LayoutBindingInfo(std::string name,
				MeshData::DataType dataType,
				uint32_t bindingPriority) :
				name(name),
				dataType(dataType),
				bindingPriority(bindingPriority)
			{

			}

			Bool32 operator ==(const LayoutBindingInfo &target) const;

			Bool32 operator<(const LayoutBindingInfo &target) const;
		};

		struct SubMeshInfo
		{
			PrimitiveTopology topology;
			std::vector<uint32_t> indices;
		};

		BaseMesh();

		~BaseMesh();

		MeshType getMeshType() const;

		uint32_t getVertexCount() const;

		void setVertexCount(uint32_t value);

		//color
		std::vector<Color32> getColors() const;

		void setColors(std::vector<Color32> colors);

		//index
		uint32_t getSubMeshCount() const;

		void setSubMeshCount(uint32_t value);

		std::vector<uint32_t> getIndices(uint32_t subMeshIndex) const;

		void setIndices(std::vector<uint32_t> indices, PrimitiveTopology topology, uint32_t subMeshIndex);

		uint32_t getIndexCount(uint32_t subMeshIndex) const;

		uint32_t getIndexStart(uint32_t subMeshIndex) const;

		PrimitiveTopology getTopology(uint32_t subMeshIndex) const;

		/**Vertex colors of the Mesh multiplied to verties*/
		Color getMultipliedColor() const;

		/**Vertex colors of the Mesh multiplied to verties*/
		void setMultipliedColor(Color value);

		/**Vertex colors of the Mesh added to verties*/
		Color getAddedColor() const;

		/**Vertex colors of the Mesh added to verties*/
		void setAddedColor(Color value);

		virtual void apply(Bool32 makeUnreadable);

		//uv
		template<UVType uvType, UVIndex uvIndex>
		typename MeshData::DataTypeInfo<UVConstInfo<uvType>::ARRAY_TYPE>::ValueType getUVs();

		template<UVType uvType, UVIndex uvIndex>
		void setUVs(typename MeshData::DataTypeInfo<UVConstInfo<uvType>::ARRAY_TYPE>::ValueType uvs, uint32_t uvIndex);

		template<MeshData::DataType dataType>
		typename MeshData::DataTypeInfo<dataType>::ValueType getData(std::string name) const;


		template <MeshData::DataType dataType>
		void setData(std::string name, typename MeshData::DataTypeInfo<dataType>::ValueType value, uint32_t bindingPriority = KGS_VERTEX_BINDING_PRIORITY_OTHER_MIN);

		inline void _fillGraphicsPipelineCreateInfoForDraw(uint32_t subMeshIndex, vk::GraphicsPipelineCreateInfo &graphicsPipelineCreateInfo);

		inline void _fillCommandBufferForDraw(uint32_t subMeshIndex, vk::CommandBuffer &commandBuffer);

	protected:
		std::shared_ptr<Context> m_pContext;
		MeshType m_meshType;
		uint32_t m_vertexCount;
		std::shared_ptr<MeshData> m_pData;
		std::vector<LayoutBindingInfo> m_arrLayoutBindingInfos;
		std::unordered_map<std::string, LayoutBindingInfo> m_mapLayoutBindingInfos;
		uint32_t m_subMeshCount;
		std::vector<SubMeshInfo> m_subMeshInfos;

		Color m_multipliedColor;
		Color m_addedColor;

		std::set<LayoutBindingInfo> m_layoutBindingInfos;
		std::vector<SubMeshInfo> m_usingSubMeshInfos; //save sub mesh info to render.
		std::shared_ptr<vk::Buffer> m_pVertexBuffer;
		std::shared_ptr<vk::DeviceMemory> m_pVertexBufferMemory;
		std::shared_ptr<vk::Buffer> m_pIndexBuffer;
		std::shared_ptr<vk::DeviceMemory> m_pIndexBufferMemory;

		void _createVertexBuffer();

		void _createIndexBuffer();

		virtual std::vector<vk::VertexInputBindingDescription> _getVertexInputBindingDescriptions() = 0;

		virtual std::vector<vk::VertexInputAttributeDescription> _getVertexInputAttributeDescriptions() = 0;

		inline void _sortLayoutBindingInfos();
		//tool methods
#ifdef DEBUG
		inline void verifySubMeshIndex(uint32_t subMeshIndex) const;
#endif // DEBUG

		inline uint32_t _getIndexCount(uint32_t subMeshIndex) const;

		inline void _copyBuffer(std::shared_ptr<vk::Buffer>& pSrcBuffer, std::shared_ptr<vk::Buffer>& pDstBuffer, vk::DeviceSize size);
	};

	template <MeshType meshType>
	class Mesh : public BaseMesh
	{
	public:
		static const MeshData::DataType ARRAY_DATA_TYPE = MeshConstInfo<meshType>::ARRAY_TYPE;
		typedef typename MeshData::DataTypeInfo<ARRAY_DATA_TYPE>::BaseType BaseValueType;
		typedef typename MeshData::DataTypeInfo<ARRAY_DATA_TYPE>::ValueType ArrayValueType;
		typedef typename MeshTypeInfo<meshType>::PointType PointType;

		Mesh();

		~Mesh();

		//position
		ArrayValueType getPositions() const;

		void setPositions(ArrayValueType vertices);

		//normal
		ArrayValueType getNormals() const;

		void setNormals(ArrayValueType normals);

		//tangent
		ArrayValueType getTangents() const;

		void setTangents(ArrayValueType tangents);

		void apply(Bool32 makeUnreadable) override;

		/*The bounding volume of the mesh*/
		fd::Bounds<PointType> getBounds();

	private:
		
		fd::Bounds<PointType> m_bounds;

		std::vector<vk::VertexInputBindingDescription> _getVertexInputBindingDescriptions() override;

		std::vector<vk::VertexInputAttributeDescription> _getVertexInputAttributeDescriptions() override;


		inline void _updateBounds();
	};
}

#include "graphics/mesh/mesh.inl"

#endif // !KGS_MESH_H
