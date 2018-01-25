namespace vg
{
    template<typename VertexType>
    void VertexData::init(uint32_t vertexCount 
        , const void *memory
        , Bool32 cacheMemory
        )
    {
        uint32_t size = vertexCount * static_cast<uint32_t>(sizeof(VertexType));
        init(vertexCount, memory, size, cacheMemory);
    }

    template<typename VertexType>
    void init(uint32_t vertexCount 
        , const void *memory
        , Bool32 cacheMemory
        , const vk::PipelineVertexInputStateCreateInfo &vertexInputStateCreateInfo
        , const vk::PipelineInputAssemblyStateCreateInfo &inputAssemblyStateCreateInfo)
    {
        init(vertexCount, memory, cacheMemory);
        _setVertexInputStateCreateInfo(vertexInputStateCreateInfo);
        _setInputAssemblyStateCreateInfo(inputAssemblyStateCreateInfo);
    }

    template<typename VertexType>
    VertexType VertexData::getVertex(uint32_t index) const
    {
#ifdef DEBUG
        if (m_pMemory == nullptr) throw std::runtime_error("Failed to get vertex when not chache memory.");
#endif //!DEBUG
        VertexType vertex;
        memcpy(&vertex, 
            static_cast<VertexType *>(m_pMemory) + index, 
            sizeof(VertexType));
        return vertex;
    }

    template<typename VertexType>
    std::vector<VertexType> VertexData::getVertices(uint32_t offset, uint32_t count) const
    {
#ifdef DEBUG
        if (m_pMemory == nullptr) throw std::runtime_error("Failed to get verties when not chache memory.");
#endif //!DEBUG
        std::vector<VertexType> vertices(count);
        memcpy(vertices.data(), 
            static_cast<VertexType *>(m_pMemory) + offset, 
            sizeof(VertexType) * static_cast<size_t>(count));
        return vertices;
    }
} //!vg