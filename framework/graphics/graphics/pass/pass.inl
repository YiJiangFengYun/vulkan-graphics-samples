namespace vg
{
	template<typename T>
	void Pass::addData(const std::string name, const PassDataInfo &info, const T &value)
	{
		m_data.addData<T>(name, info, value);
		m_dataChanged = VG_TRUE;
	}

	template<typename T>
	T Pass::getData(const std::string name) const
	{
		return m_data.getData<T>(name);
	}

	template<typename T>
	void Pass::setData(const std::string name, const T &value)
	{
		m_data.setData<T>(name, value);
		m_dataContentChanged = VG_TRUE;
		m_dataContentChanges[name] = VG_TRUE;
	}
    
	template<typename T>
	void Pass::addData(const std::string name, const PassDataInfo &info, const std::vector<T> &values)
	{
		m_data.addData<T>(name, info, values);
		m_dataChanged = VG_TRUE;
	}

	template <typename T>
	std::vector<T> Pass::getData(const std::string name, const uint32_t count) const
	{
		return m_data.getData<T>(name, count);
	}

	template<typename T>
	void Pass::setData(const std::string name, const std::vector<T> &values)
	{
		m_data.setData<T>(name, values);
		m_dataContentChanged = VG_TRUE;
		m_dataContentChanges[name] = VG_TRUE;
	}

	template<typename T>
	void Pass::addData(const std::string name, const PassDataInfo &info, const T * const pSrc, const uint32_t count)
	{
		m_data.addData<T>(name, info, pSrc, count);
		m_dataChanged = VG_TRUE;
	}

	template<typename T>
	void Pass::getData(const std::string name, const T * const pDst, const uint32_t count)
	{
		m_data.getData<T>(name, pDst, count);
	}

	template<typename T>
	void Pass::setData(const std::string name, const T * const pSrc, const uint32_t count)
	{
		m_data.setData<T>(name, pSrc, count);
		m_dataContentChanged = VG_TRUE;
		m_dataContentChanges[name] = VG_TRUE;
	}

	template <typename T>
    void Pass::addSpecializationItem(vk::ShaderStageFlagBits shaderStage
        , std::string name
        , uint32_t priority
        , const T &data
        )
	{
		m_mapSpecilizationDatas[shaderStage].addSpecialization(name, priority, data);
		m_specializationChanged = VG_TRUE;
        m_mapSpecializationDataChanges[shaderStage] = VG_TRUE;
	}

	template <typename T>
    void Pass::setSpecializationItem(vk::ShaderStageFlagBits shaderStage
        , std::string name
        , uint32_t priority
        , const T &data
        )
	{
		m_mapSpecilizationDatas[shaderStage].setSpecialization(name, priority, data);
		m_specializationChanged = VG_TRUE;
        m_mapSpecializationDataChanges[shaderStage] = VG_TRUE;
	}

	template<typename T>
	void Pass::setPushConstantUpdate(std::string name
		, uint32_t offset
	    , const T &data
		)
	{
		m_pushConstant.setPushConstantUpdate(name, offset, data);
	}

	template <typename T>
	void Pass::_updateBuildInData(BuildInDataType type, T data)
	{
		uint32_t count = static_cast<uint32_t>(BuildInDataType::COUNT);
		uint32_t offset1 = 0u;
		uint32_t i = 0u;
		for (i = 0; i < count; ++i)
		{
			if (i != static_cast<uint32_t>(type))
			{
				offset1 += buildInDataTypeSizes[i];
			}
			else
			{
				break;
			}
		}

		uint32_t offset2 = 0u;
		uint32_t componentCount = m_buildInDataInfo.componentCount;
		for (uint32_t componentIndex = 0u; componentIndex < componentCount; ++componentIndex)
		{
			Pass::BuildInDataType comType = (*(m_buildInDataInfo.pComponent + componentIndex)).type;
			if (type != comType)
			{
				offset2 += buildInDataTypeSizes[static_cast<uint32_t>(comType)];
			}
			else
			{
				break;
			}
		}

		if ( i < count)
		{
			size_t size = sizeof(T);
			if (memcmp((char *)(&m_buildInDataCache) + offset1, &data, size) != 0) {
				memcpy((char *)(&m_buildInDataCache) + offset1, &data, size);
				setData(VG_PASS_BUILDIN_DATA_NAME
				    , &data
					, size
					, offset2);
			}
		}
	}
	
} //namespace kgs
