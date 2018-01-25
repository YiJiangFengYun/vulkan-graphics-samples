#ifndef VG_INDEX_DATA_H
#define VG_INDEX_DATA_H

#include <foundation/foundation.hpp>
#include "graphics/global.hpp"

namespace vg {
     class IndexData : public Base
     {
     public:
        uint32_t getIndexCount() const;
        uint32_t getBufferSize() const;
        std::shared_ptr<vk::Buffer> getBuffer() const;
        std::shared_ptr<vk::DeviceMemory> getBufferMemory() const;
        uint32_t getMemorySize() const;
        const void *getMemory() const;

        IndexData();
        ~IndexData();

        void init(uint32_t indexCount
             , const void *memory
             , uint32_t size
             , Bool32 cacheMemory
             );
        
        template<typename IndexType>
        void init(uint32_t indexCount
            , const void *memory
            , Bool32 cacheMemory
            );

        template<typename IndexType>
        IndexType getIndex(uint32_t index) const;

        template<typename IndexType>
        std::vector<IndexType> getIndices(uint32_t offset, uint32_t count) const;

     private:
        uint32_t m_indexCount;
        uint32_t m_bufferSize;
        std::shared_ptr<vk::Buffer> m_pBuffer;
        std::shared_ptr<vk::DeviceMemory> m_pBufferMemory;
        uint32_t m_memorySize;
        void *m_pMemory;
     };
} //!vg
#include "graphics/vertex_data/index_data.inl"
#endif //!VG_INDEX_DATA_H