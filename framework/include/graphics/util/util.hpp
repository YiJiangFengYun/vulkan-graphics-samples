#ifndef KGS_UTIL_H
#define KGS_UTIL_H

#include <string>
#include <unordered_map>
#include <vector>
#include "graphics/global.hpp"

namespace kgs
{
	template <typename T>
	inline Bool32 isHas(std::string name, std::unordered_map<std::string, T>& map, std::vector<T>& arr)
	{
		auto iterator = map.find(name);
		return iterator != map.end();
	}

	template <typename T>
	inline T getValue(std::string name, std::unordered_map<std::string, T>& map, std::vector<T>& arr, Bool32 isThrowNotExist = KGS_FALSE)
	{
		auto iterator = map.find(name);
		if (iterator == map.end())
		{
			if (isThrowNotExist == KGS_TRUE)
			{
				throw std::runtime_error("The Content of the specify key don't exit in map");
			}
			return {};
		}
		else
		{
			return iterator->second;
		}
	}

	template <typename T>
	inline void setValue(std::string name, T value, std::unordered_map<std::string, T>& map, std::vector<T>& arr)
	{
		auto iterator = map.find(name);
		if (iterator == map.end())
		{
			map.insert(std::make_pair(name, value));
			arr.push_back(value);
		}
		else
		{
			iterator->second = value;
			auto iterator2 = std::find(arr.begin(), arr.end(), iterator->second);
			*iterator2 = value;
		}
	}
} //namespace kgs
#endif // !KGS_UTIL_H