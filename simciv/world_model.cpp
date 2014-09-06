
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

	void WorldModel::create_map(int width, int height, int prod_count)
	{
		_width = width;
		_height = height;
		_pc = prod_count;
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				Area* a = new Area(_pc);
				a->x = x;
				a->y = y;
				_areas.push_back(a);

				if (x > 0)
				{
					add_road(a, get_area(x - 1, y));
				}
				if (y > 0)
				{
					add_road(a, get_area(x, y - 1));
				}
			}
		}
	}

	void WorldModel::add_road(Area* a, Area* b)
	{
		Road* r = new Road(_pc);
		r->a = a;
		r->b = b;
		_roads.push_back(r);
		a->_roads.push_back(r);
		b->_roads.push_back(r);
	}

	void WorldModel::end_turn()
	{

	}

	void WorldModel::add_supply(Area* area, int prod_id, double volume, double price)
	{
		AreaProd& a = area->_prod[prod_id];
		if (volume < 0)
		{
			volume = -volume;
			a.prod_p_dem = (a.prod_p_dem * a.prod_v_dem + volume * price) / (a.prod_p_dem + volume);
			a.prod_v_dem += volume;
		}
		else
		{
			a.prod_p_sup = (a.prod_p_sup * a.prod_v_sup + volume * price) / (a.prod_p_sup + volume);
			a.prod_v_sup += volume;
		}
		
	}

	Area* WorldModel::get_area(int x, int y)
	{
		return _areas[y * _width + x];
	}
}