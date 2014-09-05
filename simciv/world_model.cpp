
#include "world_model.h"

namespace simciv
{
	Area::Area(int pc)
	{
		for (int i = 0; i < pc; ++i)
		{
			_prod.push_back(AreaProd());
		}
	}

	AreaProd::AreaProd(): 
		p(-1),
		p_dem(0),
		p_sup(max_price),
		v_dem(0),
		v_sup(0),
		prod_p_dem(0),
		prod_v_dem(0),
		prod_p_sup(max_price),
		prod_v_sup(0),
		v(0)
	{

	}

	Road::Road(int pc)
	{
		for (int i = 0; i < pc; ++i)
		{
			t.push_back(0);
		}
	}
}