#ifndef KGS_MATERIAL_H
#define KGS_MATERIAL_H

#include <memory>
#include "graphics/global.hpp"
#include "graphics/util/util.hpp"
#include "graphics/material/pass.hpp"
#include "graphics/texture/texture.hpp"
#include "graphics/material/material_data.hpp"

namespace kgs
{
	class Material
	{
	public:
		Material();
		~Material();

		int32_t getPassIndex(std::string name);
		std::string getPassName(int32_t index);
		std::shared_ptr<Pass> getPass(std::string name);
		void setPass(std::string name, std::shared_ptr<Pass> pass);
		void apply();
	private:
		//--compositions
		uint32_t m_renderQueue;
		//--compositions

		//--aggregations
		std::vector<std::shared_ptr<Pass>> m_arrPasses;
		std::unordered_map<std::string, std::shared_ptr<Pass>> m_mapPasses;
		//--aggregations

		//tool methods
		
	};
}

#endif // !KGS_MATERIAL_H