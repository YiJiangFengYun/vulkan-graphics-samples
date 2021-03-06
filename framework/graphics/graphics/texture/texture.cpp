#include "graphics/texture/texture.hpp"

namespace vg
{
    inline uint32_t caculateImageSizeWithMipmapLevel(uint32_t size, uint32_t mipmapLevel);

    TextureDataInfo::Component::Component(uint32_t mipLevel
        , uint32_t baseArrayLayer
        , uint32_t layerCount
        , uint32_t size
        , Bool32 hasImageExtent
        , uint32_t width
        , uint32_t height
        , uint32_t depth
    )
        : mipLevel(mipLevel)
        , baseArrayLayer(baseArrayLayer)
        , layerCount(layerCount)
        , size(size)
        , hasImageExtent(hasImageExtent)
        , width(width)
        , height(height)
        , depth(depth)
    {

    }

    TextureDataInfo::TextureDataInfo(uint32_t componentCount
        , Component *pComponent
        )
        : componentCount(componentCount)
        , pComponent(pComponent)
    {

    }

    Texture::ImageInfo::ImageInfo(vk::ImageCreateFlags flags
        , vk::ImageType imageType
        , vk::Format format
        , vk::Extent3D extent
        , uint32_t mipLevels
        , uint32_t arrayLayers
        , vk::SampleCountFlagBits samples
        , vk::ImageTiling tiling
        , vk::ImageUsageFlags usage
        , vk::SharingMode sharingMode
        , vk::ImageLayout layout
        , vk::ImageAspectFlags allAspect
        )
        : flags(flags)
        , imageType(imageType)
        , format(format)
        , extent(extent)
        , mipLevels(mipLevels)
        , arrayLayers(arrayLayers)
        , samples(samples)
        , tiling(tiling)
        , usage(usage)
        , sharingMode(sharingMode)
        , layout(layout)
        , allAspect(allAspect)
    {

    }

    Texture::Image::Image(ImageInfo info)
        : m_info(info)
        , m_pImage()
        , m_pImageMemory()
    {
        _create();
    }

    Texture::ImageInfo Texture::Image::getInfo() const
    {
        return m_info;
    }

    const vk::Image *Texture::Image::getImage() const
    {
        return m_pImage.get();
    }

    const vk::DeviceMemory *Texture::Image::getImageMemory() const
    {
        return m_pImageMemory.get();
    }
    
    void Texture::Image::_create()
    {
        auto &info = m_info;
        vk::ImageCreateInfo createInfo = {
            info.flags,
            info.imageType,
            info.format,
            info.extent,
            info.mipLevels,
            info.arrayLayers,
            info.samples,
            info.tiling,
            info.usage,
            info.sharingMode,
            0U,
            nullptr,
            vk::ImageLayout::eUndefined
        };

        auto pDevice = pApp->getDevice();
        m_pImage = fd::createImage(pDevice, createInfo);

        const auto &memRequirements = pDevice->getImageMemoryRequirements(*m_pImage);

        vk::MemoryAllocateInfo allocInfo = {
            memRequirements.size,
            vg::findMemoryType(pApp->getPhysicalDevice(), memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
        };

        m_pImageMemory = fd::allocateMemory(pDevice, allocInfo);

        pDevice->bindImageMemory(*m_pImage, *m_pImageMemory, vk::DeviceSize(0));

        if (m_info.layout != vk::ImageLayout::eUndefined) {
            //Transform Image layout to final layout.
            auto pCommandBuffer = beginSingleTimeCommands();

            vk::ImageMemoryBarrier barrier = {};
            barrier.oldLayout = vk::ImageLayout::eUndefined;
            barrier.newLayout = info.layout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = *m_pImage;
    
            barrier.subresourceRange.aspectMask = m_info.allAspect;
            barrier.subresourceRange.baseMipLevel = 0u;
            barrier.subresourceRange.levelCount = m_info.mipLevels;
            barrier.subresourceRange.baseArrayLayer = 0u;
            barrier.subresourceRange.layerCount = m_info.arrayLayers;

            vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eAllCommands;
            vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eAllCommands;
    
            pCommandBuffer->pipelineBarrier(srcStageMask,
                dstStageMask,
                vk::DependencyFlags(),
                nullptr, nullptr,
                barrier);

            endSingleTimeCommands(pCommandBuffer);
        }

        
    }

    Texture::ImageViewInfo::ImageViewInfo(vk::ImageViewCreateFlags flags
        , vk::Image image
        , vk::ImageViewType viewType
        , vk::Format format
        , vk::ComponentMapping components
        , vk::ImageSubresourceRange subResourceRange
        )
        : flags(flags)
        , image(image)
        , viewType(viewType)
        , format(format)
        , components(components)
        , subResourceRange(subResourceRange)
    {

    }

    Texture::ImageViewCreateInfo::ImageViewCreateInfo(vk::ImageViewType viewType
        , vk::ComponentMapping components
        , vk::ImageSubresourceRange subResourceRange
        )
        : viewType(viewType)
        , components(components)
        , subResourceRange(subResourceRange)
    {

    }

    Texture::ImageView::ImageView(ImageViewInfo info)
        : m_info(info)
        , m_pImageView()
    {
        _create();
    }

    Texture::ImageViewInfo Texture::ImageView::getInfo() const
    {
        return m_info;
    }

    const vk::ImageView *Texture::ImageView::getImageView() const
    {
        return m_pImageView.get();
    }

    void Texture::ImageView::_create()
    {
        auto &info = m_info;
        vk::ImageViewCreateInfo createInfo = {
            info.flags,
            info.image,
            info.viewType,
            info.format,
            info.components,
            info.subResourceRange,
        };

        auto pDevice = pApp->getDevice();
        m_pImageView = fd::createImageView(pDevice, createInfo);
    }

    Texture::SamplerInfo::SamplerInfo(vk::SamplerCreateFlags flags
        , vk::Filter magFilter
        , vk::Filter minFilter
        , vk::SamplerMipmapMode mipmapMode
        , vk::SamplerAddressMode addressModeU
        , vk::SamplerAddressMode addressModeV
        , vk::SamplerAddressMode addressModeW
        , float mipLodBias
        , vk::Bool32 anisotropyEnable
        , float maxAnisotropy
        , vk::Bool32 compareEnable
        , vk::CompareOp compareOp
        , float minLod
        , float maxLod
        , vk::BorderColor borderColor
        , vk::Bool32 unnormalizedCoordinates
        )
        : flags(flags)
        , magFilter(magFilter)
        , minFilter(minFilter)
        , mipmapMode(mipmapMode)
        , addressModeU(addressModeU)
        , addressModeV(addressModeV)
        , addressModeW(addressModeW)
        , mipLodBias(mipLodBias)
        , anisotropyEnable(anisotropyEnable)
        , maxAnisotropy(maxAnisotropy)
        , compareEnable(compareEnable)
        , compareOp(compareOp)
        , minLod(minLod)
        , maxLod(maxLod)
        , borderColor(borderColor)
        , unnormalizedCoordinates(unnormalizedCoordinates)
    {

    }

    Texture::SamplerCreateInfo::SamplerCreateInfo(vk::SamplerCreateFlags flags
        , vk::Filter magFilter
        , vk::Filter minFilter
        , vk::SamplerMipmapMode mipmapMode
        , vk::SamplerAddressMode addressModeU
        , vk::SamplerAddressMode addressModeV
        , vk::SamplerAddressMode addressModeW
        , float mipLodBias
        , vk::Bool32 anisotropyEnable
        , float maxAnisotropy
        , float minLod
        , float maxLod
        , vk::BorderColor borderColor
        )
        : flags(flags)
        , magFilter(magFilter)
        , minFilter(minFilter)
        , mipmapMode(mipmapMode)
        , addressModeU(addressModeU)
        , addressModeV(addressModeV)
        , addressModeW(addressModeW)
        , mipLodBias(mipLodBias)
        , anisotropyEnable(anisotropyEnable)
        , maxAnisotropy(maxAnisotropy)
        , minLod(minLod)
        , maxLod(maxLod)
        , borderColor(borderColor)
    {

    }

    Texture::Sampler::Sampler(SamplerInfo info)
        : m_info(info)
        , m_pSampler()
    {
        _create();
    }

    Texture::SamplerInfo Texture::Sampler::getInfo() const
    {
        return m_info;
    }

    const vk::Sampler *Texture::Sampler::getSampler() const
    {
        return m_pSampler.get();
    }

    void Texture::Sampler::_create()
    {
        auto &info = m_info;
        vk::SamplerCreateInfo createInfo = {
            info.flags,                
            info.magFilter, 
            info.minFilter,  
            info.mipmapMode,  
            info.addressModeU, 
            info.addressModeV, 
            info.addressModeW, 
            info.mipLodBias, 
            info.anisotropyEnable,     
            info.maxAnisotropy,
            info.compareEnable, 
            info.compareOp, 
            info.minLod, 
            info.maxLod, 
            info.borderColor,
            info.unnormalizedCoordinates,
        };

        auto pDevice = pApp->getDevice();
        m_pSampler = fd::createSampler(pDevice, createInfo);
    }

    Texture::Texture(vk::Format format
        , Bool32 mipMap
        , vk::ImageUsageFlags additionalUsage
        , Bool32 defaultImageView
        , Bool32 defaultSampler
        )
        : Base(BaseType::TEXTURE)
        , m_type()
        , m_width(1U)
        , m_height(1U)
        , m_depth(1U)
        , m_arrayLength(1U)
        , m_format(format)        
        , m_mipMap(mipMap)
        , m_mipLevels()
        , m_arrayLayers()
        , m_allAspectFlags()
        , m_usageFlags(additionalUsage)
        , m_layout()
        , m_isCreateDefaultImageView(defaultImageView)
        , m_isCreateDefaultSampler(defaultSampler)
        , m_dataLayout()
        , m_components()
        , m_pMemory(nullptr)
        , m_memorySize(0u)
        , m_pImage()
        , m_pImageView()
        , m_pSampler()
        , m_mapPOtherImageViews()
        , m_mapPOtherSamplers()
    {

    }

    Texture::~Texture()
    {
        if (m_pMemory != nullptr)
        {
            free(m_pMemory);
        }
    }

    TextureType Texture::getType() const
    {
        return m_type;
    }

    Bool32 Texture::getIsMipmap() const
    {
        return m_mipMap;
    }

    vk::ImageType Texture::getImageType() const
    {
        return arrTextureTypeToVKImageType[static_cast<size_t>(m_type)].second;
    }

    vk::ImageViewType Texture::getImageViewType() const
    {
        return arrTextureTypeToVKImageViewType[static_cast<size_t>(m_type)].second;
    }

    const Texture::Image *Texture::getImage() const
    {
        return m_pImage.get();
    }

    const Texture::ImageView *Texture::getImageView() const
    {
        return m_pImageView.get();
    }

    const Texture::Sampler *Texture::getSampler() const
    {
        return m_pSampler.get();
    }

    const Texture::ImageView *Texture::createImageView(std::string name, ImageViewCreateInfo createInfo)
    {
        auto pImageView = m_mapPOtherImageViews[name];
        if (pImageView != nullptr) {
            VG_LOG(plog::warning) << "Image view its key is " << name << " has exist!" << std::endl;
            return pImageView.get();
        }

#ifdef DEBUG
        auto allAspect = m_pImage->getInfo().allAspect;
        if ((allAspect & createInfo.subResourceRange.aspectMask) != createInfo.subResourceRange.aspectMask) {
            auto errorStr = "All aspect of the image don't contain all need aspect to create this image view.";
            VG_LOG(plog::error) << errorStr << std::endl;
            throw std::invalid_argument(errorStr);
        }
#endif //DEBUG
        ImageViewInfo info = {
            vk::ImageViewCreateFlags(),
            *(m_pImage->getImage()),
            createInfo.viewType,
            m_format,
            createInfo.components,
            createInfo.subResourceRange,
        };

        pImageView = std::shared_ptr<ImageView>{new ImageView(info)};
        m_mapPOtherImageViews[name] = pImageView;
        return pImageView.get();
        
    }

    const Texture::ImageView *Texture::getImageView(std::string name) const
    {
        auto &map = m_mapPOtherImageViews;
        const auto& iterator = map.find(name);
        if (iterator == map.cend())
        {
            return nullptr;
        }
        else
        {
            return iterator->second.get();
        }
    }

    const Texture::Sampler *Texture::createSampler(std::string name, SamplerCreateInfo createInfo)
    {
        auto pSampler = m_mapPOtherSamplers[name];
        if (pSampler != nullptr) {
            VG_LOG(plog::warning) << "Sampler its key is " << name << " has exist!" << std::endl;
            return pSampler.get();
        }
        SamplerInfo info = {
            createInfo.flags,
            createInfo.magFilter,
            createInfo.minFilter,
            createInfo.mipmapMode,
            createInfo.addressModeU,
            createInfo.addressModeV,
            createInfo.addressModeW,
            createInfo.mipLodBias,
            createInfo.anisotropyEnable,
            createInfo.maxAnisotropy,
            VG_FALSE,
            vk::CompareOp::eNever,
            createInfo.minLod,
            createInfo.maxLod,
            createInfo.borderColor,
        };
        pSampler = std::shared_ptr<Sampler>{new Sampler(info)};
        m_mapPOtherSamplers[name] = pSampler;
        return pSampler.get();
    }

    const Texture::Sampler *Texture::getSampler(std::string name) const
    {
        auto &map = m_mapPOtherSamplers;
        const auto& iterator = map.find(name);
        if (iterator == map.cend())
        {
            return nullptr;
        }
        else
        {
            return iterator->second.get();
        }
    }

    void Texture::_init(Bool32 importContent)
    {
        _updateMipMapLevels();
        _updateArrayLayer();
        _createImage(importContent);
        _createImageView();
        _createSampler();
    }

    void Texture::_updateMipMapLevels()
    {
        if (m_mipMap)
        {
            // calculate num of mip maps
            // numLevels = 1 + floor(log2(max(w, h, d)))
            m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max({ m_width, m_height, m_depth }))) + 1);
        }
        else
        {
            m_mipLevels = 1u;
        }
    }

    void Texture::_updateArrayLayer()
    {
        m_arrayLayers = getTexArrayLayers(m_type, m_arrayLength);
    }

    void Texture::_createImage(Bool32 importContent)
    {
        vk::ImageType vkImageType;
        vkImageType = arrTextureTypeToVKImageType[static_cast<size_t>(m_type)].second;


#ifdef DEBUG
        //check whether other arguments is compatibility with texture type.
        checkTexImageSize(m_type, m_width, m_height, m_depth);

#endif // DEBUG

        //caculate compatible array layer and flag.
        vk::ImageCreateFlags flags;
        flags |= arrTextureTypeToImageCreateFlags[static_cast<size_t>(m_type)].second;

        vk::ImageUsageFlags usage = m_usageFlags;
        if (importContent)
            usage |= vk::ImageUsageFlagBits::eTransferDst;

        ImageInfo info = {
            flags,
            vkImageType,
            m_format,
            {
                m_width,
                m_height,
                m_depth
            },
            m_mipLevels,
            m_arrayLayers,
            vk::SampleCountFlagBits::e1,
            vk::ImageTiling::eOptimal,
            usage,
            vk::SharingMode::eExclusive,
            m_layout,
            m_allAspectFlags,
        };

        m_pImage = std::shared_ptr<Image>{new Image(info)};
    }

    void Texture::_createImageView()
    {
       if (m_isCreateDefaultImageView == VG_FALSE) return;
        vk::ImageViewType vkImageViewType;
        vkImageViewType = arrTextureTypeToVKImageViewType[static_cast<size_t>(m_type)].second;

        ImageViewInfo info = {
            vk::ImageViewCreateFlags(),
            *(m_pImage->getImage()),
            vkImageViewType,
            m_format,
            vk::ComponentMapping(),
            {
                m_allAspectFlags,
                uint32_t(0),
                m_mipLevels,
                uint32_t(0),
                m_arrayLayers,
            },
        };

        m_pImageView = std::shared_ptr<ImageView>{new ImageView(info)};
    }

    void Texture::_createSampler()
    {
        if (m_isCreateDefaultSampler == VG_FALSE) return;
        SamplerInfo info = {
            vk::SamplerCreateFlags(),
            vk::Filter::eLinear,
            vk::Filter::eLinear,
            vk::SamplerMipmapMode::eLinear,
            vk::SamplerAddressMode::eClampToEdge,
            vk::SamplerAddressMode::eClampToEdge,
            vk::SamplerAddressMode::eClampToEdge,
            0.0f,
            VK_FALSE,
            0.0f,
            VK_FALSE,
            vk::CompareOp::eNever,
            0.0f,
            static_cast<float>(m_mipLevels),
            vk::BorderColor::eFloatTransparentBlack,
            VK_FALSE,
        };
        m_pSampler = std::shared_ptr<Sampler>{new Sampler(info)};
    }

    void Texture::_applyData(const TextureDataInfo &layoutInfo
        , const void *memory
        , uint32_t size
        , Bool32 cacheMemory
        , Bool32 createMipmaps)
    {
        vk::Image image = *(m_pImage->getImage());
        if (cacheMemory)
        {
            m_components.resize(layoutInfo.componentCount);
            memcpy(m_components.data(), layoutInfo.pComponent,
                sizeof(TextureDataInfo::Component) * layoutInfo.componentCount);
            m_dataLayout = layoutInfo;
            m_dataLayout.pComponent = m_components.data();
        }

        if (m_pMemory != nullptr && (m_memorySize < size || ! cacheMemory)) {
            free(m_pMemory);
            m_pMemory = nullptr;
            m_memorySize = 0;
        }

        if (size)
        {
            if (cacheMemory) 
            {
                if (m_pMemory == nullptr)
                {
                    m_pMemory = malloc(size);
                    m_memorySize = size;
                }
                memcpy(m_pMemory, memory, size);
                m_realSize = size;
            }
            auto pDevice = pApp->getDevice();

            //create staging buffer.
            std::shared_ptr<vk::Buffer> pStagingBuffer;
            std::shared_ptr<vk::DeviceMemory> pStagingBufferMemory;
            _createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                pStagingBuffer, pStagingBufferMemory);

            void *data = pDevice->mapMemory(*pStagingBufferMemory, 0, size);
            memcpy(data, memory, static_cast<size_t>(size));
            pDevice->unmapMemory(*pStagingBufferMemory);
            auto pCommandBuffer = beginSingleTimeCommands();
            if (createMipmaps)
            {
                //transfer image from initial current image layout to dst layout.
                //here use undefined layout not to use curr layout of image, it can clear image old data.
                _tranImageLayout(pCommandBuffer, image, m_layout, vk::ImageLayout::eTransferDstOptimal,
                    0, 1, 0, m_arrayLayers);

                //copy the first mip of the chain.
                _copyBufferToImage(pCommandBuffer, *pStagingBuffer, image, m_width, m_height, m_depth, 0, 0, m_arrayLayers);

#ifdef DEBUG
                //check format.
                const auto &pPhysicalDevice = pApp->getPhysicalDevice();
                const auto &formatProperties = pPhysicalDevice->getFormatProperties(m_format);
                if ((formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitSrc) == vk::FormatFeatureFlags())
                {
                    throw std::runtime_error("The texture format don't support for blit source, mip-chain generation requires it.");
                }
                if ((formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitDst) == vk::FormatFeatureFlags())
                {
                    throw std::runtime_error("The texture format don't support for blit destination, mip-chain generation requires it.");
                }
#endif // DEBUG

                //transition first mip level to transfer source for read during blit.
                _tranImageLayout(pCommandBuffer, image, vk::ImageLayout::eTransferDstOptimal, 
                vk::ImageLayout::eTransferSrcOptimal, 0, 1, 0, m_arrayLayers);

                for (uint32_t i = 1; i < m_mipLevels; ++i)
                {
                    vk::ImageBlit blit;
                    blit.srcSubresource.aspectMask = m_allAspectFlags;
                    blit.srcSubresource.baseArrayLayer = 0;
                    blit.srcSubresource.layerCount = m_arrayLayers;
                    blit.srcSubresource.mipLevel = i - 1;
                    blit.dstSubresource.aspectMask = m_allAspectFlags;
                    blit.dstSubresource.baseArrayLayer = 0;
                    blit.dstSubresource.layerCount = m_arrayLayers;
                    blit.dstSubresource.mipLevel = i;

                    // each mipmap is the size divided by two
                    blit.srcOffsets[1] = vk::Offset3D(caculateImageSizeWithMipmapLevel(m_width, i - 1),
                        caculateImageSizeWithMipmapLevel(m_height, i - 1),
                        caculateImageSizeWithMipmapLevel(m_depth, i - 1));

                    blit.dstOffsets[1] = vk::Offset3D(caculateImageSizeWithMipmapLevel(m_width, i),
                        caculateImageSizeWithMipmapLevel(m_height, i),
                        caculateImageSizeWithMipmapLevel(m_depth, i));

                    // transferDst go to transferSrc because this mipmap will be the source for the next iteration (the next level)
                    _tranImageLayout(pCommandBuffer, image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                        i, 1, 0, m_arrayLayers);

                    pCommandBuffer->blitImage(image, vk::ImageLayout::eTransferSrcOptimal,
                        image, vk::ImageLayout::eTransferDstOptimal, blit,
                        vk::Filter::eLinear);

                    _tranImageLayout(pCommandBuffer, image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eTransferSrcOptimal,
                        i, 1, 0, m_arrayLayers);
                }

                //transfer all level and all layer to shader read layout.
                _tranImageLayout(pCommandBuffer, image, vk::ImageLayout::eTransferSrcOptimal, m_layout,
                    0, m_mipLevels, 0, m_arrayLayers);
            }
            else
            {
                const auto pComponent = layoutInfo.pComponent;
                const auto componentCount = layoutInfo.componentCount;
                std::vector<vk::BufferImageCopy> bufferCopyRegions(componentCount);
                uint32_t offset = 0u;
                for (uint32_t i = 0u; i < componentCount; ++i)
                {
                    uint32_t width;
                    uint32_t height;
                    uint32_t depth;
                    const auto component = *(layoutInfo.pComponent + i);
                    if (component.hasImageExtent)
                    {
                        width = component.width;
                        height = component.height;
                        depth = component.depth;
                    }
                    else
                    {
                        width = caculateImageSizeWithMipmapLevel(m_width, component.mipLevel);
                        height = caculateImageSizeWithMipmapLevel(m_height, component.mipLevel);
                        depth = caculateImageSizeWithMipmapLevel(m_depth, component.mipLevel);
                    }
                    vk::ImageSubresourceLayers subresourceLayers = {
                        m_allAspectFlags,              //aspectMask
                        component.mipLevel,            //mipLevel
                        component.baseArrayLayer,      //baseArrayLayer
                        component.layerCount           //layerCount
                    };
                    vk::BufferImageCopy copyInfo = { 
                        offset,                                 //bufferOffset
                        0,                                      //bufferRowLength
                        0,                                      //bufferImageHeight
                        subresourceLayers,                      //imageSubresource
                        vk::Offset3D(0, 0, 0),                  //imageOffset
                        vk::Extent3D(width, height, depth)      //imageExtent
                    };

                    bufferCopyRegions[i] = copyInfo;
                    offset += component.size;
                }
                //transfer image from initial current image layout to dst layout.
                //here use undefined layout not to use curr layout of image, it can clear image old data.
                _tranImageLayout(pCommandBuffer, image, m_layout, vk::ImageLayout::eTransferDstOptimal,
                    0, m_mipLevels, 0, m_arrayLayers);
                
                pCommandBuffer->copyBufferToImage(*pStagingBuffer, image, vk::ImageLayout::eTransferDstOptimal, bufferCopyRegions);

                //transfer to shader read layout.
                _tranImageLayout(pCommandBuffer, image, vk::ImageLayout::eTransferDstOptimal, m_layout,
                    0, m_mipLevels, 0, m_arrayLayers);

            }

            endSingleTimeCommands(pCommandBuffer);
        }
    }

    void Texture::_createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
        std::shared_ptr<vk::Buffer>& pBuffer, std::shared_ptr<vk::DeviceMemory>& pBufferMemory)
    {
        vk::BufferCreateInfo createInfo = {
            vk::BufferCreateFlags(),
            size,
            usage,
            vk::SharingMode::eExclusive
        };

        auto pDevice = pApp->getDevice();
        pBuffer = fd::createBuffer(pDevice, createInfo);

        vk::MemoryRequirements memReqs = pDevice->getBufferMemoryRequirements(*pBuffer);
        vk::MemoryAllocateInfo allocateInfo = {
            memReqs.size,
            vg::findMemoryType(pApp->getPhysicalDevice(), memReqs.memoryTypeBits, properties)
        };

        pBufferMemory = fd::allocateMemory(pDevice, allocateInfo);

        pDevice->bindBufferMemory(*pBuffer, *pBufferMemory, 0);
    }

    void  Texture::_tranImageLayout(std::shared_ptr<vk::CommandBuffer> &pCommandBuffer, vk::Image image,
        vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
        uint32_t baseMipLevel, uint32_t levelCount,
        uint32_t baseArrayLayer, uint32_t layerCount)
    {
        vk::ImageMemoryBarrier barrier = {};
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;

        barrier.subresourceRange.aspectMask = m_allAspectFlags;
        barrier.subresourceRange.baseMipLevel = baseMipLevel;
        barrier.subresourceRange.levelCount = levelCount;
        barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
        barrier.subresourceRange.layerCount = layerCount;

        std::vector<std::tuple<vk::ImageLayout, vk::AccessFlags, vk::PipelineStageFlags>> arrLayoutToAccess = {
            std::tuple<vk::ImageLayout, vk::AccessFlags, vk::PipelineStageFlags>(vk::ImageLayout::eUndefined
                , vk::AccessFlags()
                , vk::PipelineStageFlagBits::eTopOfPipe
                )
            , std::tuple<vk::ImageLayout, vk::AccessFlags, vk::PipelineStageFlags>(vk::ImageLayout::eTransferDstOptimal
                , vk::AccessFlagBits::eTransferWrite
                , vk::PipelineStageFlagBits::eTransfer
                )
            , std::tuple<vk::ImageLayout, vk::AccessFlags, vk::PipelineStageFlags>(vk::ImageLayout::eTransferSrcOptimal
                , vk::AccessFlagBits::eTransferRead
                , vk::PipelineStageFlagBits::eTransfer
                )
            , std::tuple<vk::ImageLayout, vk::AccessFlags, vk::PipelineStageFlags>(vk::ImageLayout::eShaderReadOnlyOptimal
                , vk::AccessFlagBits::eShaderRead
                , vk::PipelineStageFlagBits::eFragmentShader
                )
            , std::tuple<vk::ImageLayout, vk::AccessFlags, vk::PipelineStageFlags>(vk::ImageLayout::eDepthStencilAttachmentOptimal
                , vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite
                , vk::PipelineStageFlagBits::eEarlyFragmentTests
                )
        };
        vk::PipelineStageFlags srcStageMask;
        vk::PipelineStageFlags dstStageMask;
        Bool32 isFindSrc = VG_FALSE;
        Bool32 isFindDst = VG_FALSE;
        for (const auto& item : arrLayoutToAccess)
        {
            if (oldLayout == std::get<0>(item))
            {
                barrier.srcAccessMask = std::get<1>(item);
                srcStageMask = std::get<2>(item);
                isFindSrc = VG_TRUE;
            }
            if (newLayout == std::get<0>(item))
            {
                barrier.dstAccessMask = std::get<1>(item);
                dstStageMask = std::get<2>(item);
                isFindDst = VG_TRUE;
            }
            if (isFindSrc && isFindDst)
            {
                break;
            }
        }

        pCommandBuffer->pipelineBarrier(srcStageMask,
            dstStageMask,
            vk::DependencyFlags(),
            nullptr, nullptr,
            barrier);
    }

    inline uint32_t caculateImageSizeWithMipmapLevel(uint32_t size, uint32_t mipmapLevel)
    {
        return std::max(1u, size >> mipmapLevel);
    }

    void Texture::_copyBufferToImage(std::shared_ptr<vk::CommandBuffer> &pCommandBuffer, vk::Buffer buffer, vk::Image image,
        uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevel,
        uint32_t baseArrayLayer, uint32_t layerCount)
    {
        vk::BufferImageCopy copyInfo = { 0, 0, 0, vk::ImageSubresourceLayers(
            m_allAspectFlags, mipLevel, baseArrayLayer, layerCount),
            vk::Offset3D(0, 0, 0),
            vk::Extent3D(width, height, depth)
        };

        pCommandBuffer->copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, copyInfo);
    }
}