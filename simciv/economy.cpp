
#include "economy.h"
#include <algorithm>

namespace simciv
{
	AreaProd::AreaProd(): 
		p(-1),
		p_con(max_price),
		p_sup(0),
		v_con(0),
		v_sup(0),
		v(0)
	{

	}

	ProductMap::ProductMap(WorldModel& world): _world(world), _production(new std::vector<AreaProd>()), _new_production(new std::vector<AreaProd>())
	{
		int n = world.areas().size();
		_production->resize(n);
		_new_production->resize(n);
		_area_consumers.resize(n);
		_area_supplies.resize(n);
	}

	void ProductMap::update()
	{
		update_routes();
		routes_to_areas();
		update_prices();
	}

	void ProductMap::update_routes()
	{
		auto& areas = _world.areas();
		int n = areas.size();
		Node* g = new Node[n];
		for (int i = 0; i < n; ++i)
		{
			Area* a = areas[i];
			g[i].area = a;
		}

		for (Transport* r: _routes)
		{
			delete r;
		}
		_routes.clear();

		for (Producer* p: _supplies)
		{
			Node& m = g[p->area->index];
			_world.get_distances(&m, g);
			for (Producer* q: _consumers)
			{
				Node& o = g[q->area->index];
				Transport* r = new Transport();
				r->route = _world.get_route(&m, &o, g);
				r->dem = q;
				r->sup = p;
				//r->profit = (q->price - p->price - r->trans_price) / r->trans_price;
				r->profit = q->price - p->price - r->trans_price;
				_routes.push_back(r);
			}
		}

		for (Producer* p: _supplies)
		{
			p->free_volume = p->volume;
			p->profit = max_price;
		}
		for (Producer* p: _consumers)
		{
			p->free_volume = - p->volume;
			p->profit = max_price;
		}

		std::sort(_routes.begin(), _routes.end(), [](Transport* a, Transport* b) {
			return a->profit > b->profit;
		});

		for (Transport* r: _routes)
		{
			double& v_sup = r->sup->free_volume;
			double& v_con = r->dem->free_volume;
			double v = std::min(v_sup, v_con);
			if (v > 0)
			{
				v_sup -= v;
				v_con -= v;
				r->volume = v;
				auto& sup = r->sup->profit;
				sup = std::min(sup, r->profit);
				auto& con = r->dem->profit;
				con = std::min(con, r->profit);
			}
		}

		for (Producer* p: _supplies)
		{
			p->partner_price = p->price + p->profit;
		}
		for (Producer* p: _consumers)
		{
			p->partner_price = std::max(0.0, p->price - p->profit);
		}

		delete g;
	}

	void ProductMap::routes_to_areas()
	{
		for (Road* r: _world.roads())
		{
			r->t[0] = 0;
		}

		for (Transport* route: _routes)
		{
			Area* a = route->sup->area;
			for (Road* r: route->route->roads)
			{
				Area* b = r->other(a);
				if (a == r->a)
				{
					r->t[0] += route->volume;
				}
				else
				{
					r->t[0] -= route->volume;
				}
				a = b;
			}
		}
	}

	void ProductMap::update_prices()
	{
		for (Area* a: _world.areas())
		{
			auto& p = get_new_prod(a);
			double new_supply_price = 0; // the highest price in this area what can a supplier use (to sell the product).
			auto& v = _area_consumers[a->index];
			if (v.size() > 0)
			{
				auto it = std::max_element(v.begin(), v.end(), [](Producer* a, Producer* b){ return a->partner_price < b->partner_price; });
				new_supply_price = (*it)->partner_price;
			}

			for (Road* r: a->_roads)
			{
				Area* b = r->other(a);
				auto& bp = get_prod(b);
				new_supply_price = std::max(new_supply_price, bp.p_sup - r->t_price);
			}
			p.p_sup = new_supply_price;

			double new_cons_price = max_price; // the lowest price in this area what can a supplier use (to sell the product).
			auto& u = _area_supplies[a->index];
			if (u.size() > 0)
			{
				auto it = std::min_element(u.begin(), u.end(), [](Producer* a, Producer* b){ return a->partner_price < b->partner_price; });
				new_cons_price = (*it)->partner_price;
			}

			for (Road* r: a->_roads)
			{
				Area* b = r->other(a);
				auto& bp = get_prod(b);
				new_cons_price = std::min(new_cons_price, bp.p_con + r->t_price);
			}
			p.p_con = new_cons_price;

			p.p = (p.p_con + p.p_sup) / 2;
		}

		//*_production = *_new_production;
		 std::swap(_production, _new_production);
	}

	void ProductMap::add_supply(Area* area, double volume, double price)
	{
		AreaProd& a = get_prod(area);
		Producer* p = new Producer();
		p->price = price;
		p->volume = volume;
		p->area = area;
		if (volume < 0)
		{
			// consumer
			_consumers.push_back(p);
			_area_consumers[area->index].push_back(p);
		}
		else
		{
			// producer
			_supplies.push_back(p);
			_area_supplies[area->index].push_back(p);
		}
	}
}