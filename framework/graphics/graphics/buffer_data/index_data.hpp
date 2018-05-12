#ifndef VG_INDEX_DATA_H
#define VG_INDEX_DATA_H

#include <foundation/foundation.hpp>
#include "graphics/global.hpp"
#include "graphics/buffer_data/buffer_data_option.hpp"

namespace vg {
     class IndexData : public Base
     {
     public:
        using PipelineStateID = uint32_t;
        template<vk::IndexType indexType>
        struct IndexTypeInfo 
        {
            using ValueType = void;
        };

        template<>
        struct IndexTypeInfo<vk::IndexType::eUint16> 
        {
            using ValueType = uint16_t;
        };

        template<>
        struct IndexTypeInfo<vk::IndexType::eUint32>
        {
            using ValueType = uint32_t;
        };

        struct SubIndexData {
            vk::IndexType indexType;           
            uint32_t indexCount;
            uint32_t bufferSize;
            vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo;
            uint32_t vertexDataIndex;

            SubIndexData(vk::IndexType indexType = vk::IndexType()
                , uint32_t indexCount = 0u
                , uint32_t bufferSize = 0u
                , vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo = vk::PipelineInputAssemblyStateCreateInfo()
                , uint32_t vertexDataIndex = 0u);
        };
        uint32_t getSubIndexDataCount() const;
        const SubIndexData *getSubIndexDatas() const;
        uint32_t getBufferSize() const;
        const vk::Buffer *getBuffer() const;
        uint32_t getBufferMemorySize() const;
        const vk::DeviceMemory *getBufferMemory() const;
        uint32_t getMemorySize() const;
        const void *getMemory() const;

        IndexData();
        IndexData(MemoryPropertyFlags bufferMemoryPropertyFlags);
        ~IndexData();

        void init(uint32_t subDataCount
            , const SubIndexData *pSubDatas
            , const void *memory
            , uint32_t size
            , Bool32 cacheMemory
            );

        void init(uint32_t indexCount
             , vk::IndexType indexType             
             , const void *memory
             , uint32_t size
             , Bool32 cacheMemory
             , const vk::PipelineInputAssemblyStateCreateInfo &inputAssemblyStateInfo
             );

        void updateDesData(uint32_t subDataCount, const SubIndexData *pSubDatas);
        void updateDesData(vk::IndexType indexType, const vk::PipelineInputAssemblyStateCreateInfo &inputAssemblyStateInfo);
        void updateDesData(uint32_t indexCount, vk::IndexType indexType,  uint32_t size, 
            const vk::PipelineInputAssemblyStateCreateInfo &inputAssemblyStateInfo);

        void updateSubDataCount(uint32_t count);
        void updateIndexCount(fd::ArrayProxy<uint32_t> indexCounts, uint32_t count, uint32_t offset = 0u);
        void updateBufferSize(fd::ArrayProxy<uint32_t> bufferSizes, uint32_t count, uint32_t offset = 0u);
        void updateVertexDataIndex(fd::ArrayProxy<uint32_t> vertexDataIndices, uint32_t count, uint32_t offset = 0u);       
        void updateStateInfo(fd::ArrayProxy<vk::PipelineInputAssemblyStateCreateInfo> stateInfos, uint32_t count, uint32_t offset = 0u);

        void updateBuffer(const void *memory, uint32_t size, Bool32 cacheMemory);
        void updateBuffer(fd::ArrayProxy<MemorySlice> memories
           , uint32_t size
           , Bool32 cacheMemory
           );

        template<vk::IndexType indexType>
        typename IndexTypeInfo<indexType>::ValueType getIndex(uint32_t index) const;

        template<vk::IndexType indexType>
        std::vector<typename IndexTypeInfo<indexType>::ValueType> getIndices(uint32_t offset, uint32_t count) const;
        
     private:
        MemoryPropertyFlags m_bufferMemoryPropertyFlags;
        std::vector<SubIndexData> m_subDatas;
        uint32_t m_subDataCount;
        uint32_t m_bufferSize;
        std::shared_ptr<vk::Buffer> m_pBuffer;
        uint32_t m_bufferMemorySize;
        std::shared_ptr<vk::DeviceMemory> m_pBufferMemory;
        uint32_t m_memorySize;
        void *m_pMemory;
        void *m_pMmemoryForHostVisible;
        
        Bool32 _isDeviceMemoryLocal() const;
        void _createBuffer(fd::ArrayProxy<MemorySlice> memories, uint32_t memorySize);
        Bool32 _isEqual(uint32_t subDataCount1, const SubIndexData *pSubDatas1, 
            uint32_t subDataCount2, const SubIndexData *pSubDatas2);
     };
} //!vg
#endif //!VG_INDEX_DATA_H