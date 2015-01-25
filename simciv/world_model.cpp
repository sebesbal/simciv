
#include "world_model.h"
#include <queue>

using namespace std;

namespace simciv
{
	//const double trans_price = 1.0;

	Area::Area(int pc, int index): index(index)
	{
		//for (int i = 0; i < pc; ++i)
		//{
		//	_prod.push_back(AreaProd());
		//}
	}

	void Area::get_trans(int id, double& x, double& y)
	{
		double vx = 0;
		double vy = 0;
		Area* a = this;
		for (Road* r: _roads)
		{
			double t = r->t[id];
			if (!(t > 0 ^ r->a == a))
			{
				Area* b = r->other(a);
				vx += (b->x - a->x) * abs(t);
				vy += (b->y - a->y) * abs(t);
			}
		}
		x = vx;
		y = vy;
	}

	AreaProd::AreaProd(): 
		p(-1),
		p_con(0),
		p_sup(max_price),
		v_con(0),
		v_sup(0),
		v(0)
	{

	}

	Road::Road(int pc, double t_price): t_price(t_price)
	{
		for (int i = 0; i < pc; ++i)
		{
			t.push_back(0);
		}
	}

	WorldModel::WorldModel(): _production(new vector<std::vector<AreaProd>>()), _new_production(new vector<std::vector<AreaProd>>())
	{

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
				Area* a = new Area(_pc, _areas.size());
				a->x = x;
				a->y = y;
				_areas.push_back(a);

				std::vector<AreaProd> p;
				for (int i = 0; i < _pc; ++i)
				{
					p.push_back(AreaProd());
				}
				_production->push_back(p);

				if (x > 0)
				{
					add_road(a, get_area(x - 1, y));
					if (y > 0)
					{
						add_road(a, get_area(x - 1, y - 1));
					}
				}
				if (y > 0)
				{
					add_road(a, get_area(x, y - 1));
					if (x < width - 1)
					{
						add_road(a, get_area(x + 1, y - 1));
					}
				}
			}
		}
	}

	void WorldModel::add_road(Area* a, Area* b)
	{
		bool orto = a->x == b->x || a->y == b->y;
		Road* r = new Road(_pc, orto ? 1.0 : 1.414);
		r->a = a;
		r->b = b;
		_roads.push_back(r);
		a->_roads.push_back(r);
		b->_roads.push_back(r);
	}
	
	void WorldModel::end_turn()
	{
		for (int k = 0; k < 10; ++k)
		{
			for (int i = 0; i < _pc; ++i)
			{
				end_turn_prod(i);
			}
		}
	}
	
	double price(double p_sup, double v_sup, double p_con, double v_con)
	{
		double v = v_sup + v_con;
		if (p_sup == max_price || p_con == 0)
		{
			return -1;
		}
		else if (v < 0.5)
		{
			return (p_sup + p_con) / 2;
		}
		else
		{
			double x = v_con / v - 0.5;
			// double k = 6; // if the k is bigger, the sigmoid is "sharper"
			double k = 4;
			double d = tanh(k * x);
			double alpha = (d + 1) / 2;
			return alpha * p_con + (1 - alpha) * p_sup;
		}
	}

	const double trans_rate = 1.0;

	Area* WorldModel::get_area(int x, int y)
	{
		return _areas[y * _width + x];
	}

	AreaProd& WorldModel::get_prod(Area* a, int id)
	{
		return (*_production)[a->index][id];
	}

	AreaProd& WorldModel::get_new_prod(Area* a, int id)
	{
		return (*_new_production)[a->index][id];
	}

	void World2::end_turn()
	{
		for (int i = 0; i < _pc; ++i)
		{
			end_turn_prod(i);
		}
	}

	void World2::end_turn_prod(int id)
	{
		static int k = 0;
		if (k++ % 20 == 0)
		{
			update_routes();
			routes_to_areas();
			update_prices();
		}
	}

	void World2::get_distances(Node* src, Node* g)
	{
		int nn = _areas.size();

		for (int i = 0; i < nn; ++i)
		{
			Node& n = g[i];
			n.color = 0;
			n.d = std::numeric_limits<double>::max();
			n.parent = NULL;
		}

		std::vector<Node*> Q;
		src->color = 1;
		src->d = 0;

		Q.push_back(src);
		std::push_heap(Q.begin(), Q.end());

		auto pr = [](Node* a, Node* b) {
			return a->d > b->d;
		};

		while (Q.size() > 0)
		{
			Node* n = Q.front();
			std::pop_heap(Q.begin(), Q.end(), pr);
			Q.pop_back();

			n->color = 2;
			Area* a = n->area;

			for (Road* r: a->_roads)
			{
				Area* b = r->other(a);
				Node* m = &g[b->index];
				if (m->color < 2) // if m is not visited yet
				{
					double new_d = n->d + r->t_price;
					if (new_d < m->d)
					{
						m->d = new_d;
						m->parent = r;
						if (m->color == 0)
						{
							m->color = 1;
							Q.push_back(m);
							std::push_heap(Q.begin(), Q.end(), pr);
						}
						else
						{
							std::make_heap(Q.begin(), Q.end(), pr);
						}
					}
				}
			}
		}
	}

	Route* World2::get_route(Node* src, Node* dst, Node* g)
	{
		Route* route = new Route();
		Area* a = dst->area;
		route->trans_price = 0;
		//route->p_con = a->_prod[0].p;
		//route->dem = dst;

		while (dst->parent)
		{
			Road* r = dst->parent;
			route->roads.push_back(r);
			route->trans_price += r->t_price;
			a = r->other(a);
			dst = &g[a->index];
		}

		//route->sup = a;
		//route->p_sup = a->_prod[0].p;
		auto& v = route->roads;
		std::reverse(v.begin(), v.end());

		//route->profit = route->p_con - route->p_sup - route->trans_price;
		route->volume = 0;

		return route;
	}

	void World2::update_routes()
	{
		int n = _areas.size();
		Node* g = new Node[n];
		for (int i = 0; i < n; ++i)
		{
			Area* a = _areas[i];
			g[i].area = a;
		}

		for (Route* r: _routes)
		{
			delete r;
		}
		_routes.clear();

		for (Producer* p: _supplies)
		{
			Node& m = g[p->area->index];
			get_distances(&m, g);
			for (Producer* q: _consumers)
			{
				Node& o = g[q->area->index];
				Route* r = get_route(&m, &o, g);
				r->dem = q;
				r->sup = p;
				r->profit = (q->price - p->price - r->trans_price) / r->trans_price;
				_routes.push_back(r);
			}
		}

		for (Producer* p: _supplies)
		{
			p->free_volume = p->volume;
		}
		for (Producer* p: _consumers)
		{
			p->free_volume = - p->volume;
		}

		std::sort(_routes.begin(), _routes.end(), [](Route* a, Route* b) {
			return a->profit > b->profit;
		});

		for (Route* r: _routes)
		{
			double& v_sup = r->sup->free_volume;
			double& v_con = r->dem->free_volume;
			double v = std::min(v_sup, v_con);
			v_sup -= v;
			v_con -= v;
			r->volume = v;
		}

		delete g;
	}

	void World2::routes_to_areas()
	{
		for (Road* r: _roads)
		{
			r->t[0] = 0;
		}

		for (Route* route: _routes)
		{
			Area* a = route->sup->area;
			for (Road* r: route->roads)
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

	void World2::update_prices()
	{
		for (Area* a: _areas)
		{
			double new_supply_price = 0; // the highest price in this area what can a supplier use (to sell the product).
			double new_supply_profit = 0;
			auto& v = a->consumers;
			if (v.size() > 0)
			{
				auto it = std::max_element(v.begin(), v.end(), [](Producer* a, Producer* b){ return a->price < b->price; });
				new_supply_price = (*it)->price;

			}

			for (Road* r: a->_roads)
			{
				Area* b = r->other(a);
				auto& bp = get_prod(b, 0);
				new_supply_price = std::max(new_supply_price, bp.p_sup + r->t_price * bp.m_sup);
			}
			get_new_prod(a, 0).p_sup = new_supply_price;
		}

		//*_production = *_new_production;
		 std::swap(_production, _new_production);
	}

	void World2::add_supply(Area* area, int prod_id, double volume, double price)
	{
		AreaProd& a = get_prod(area, prod_id);
		Producer* p = new Producer();
		p->price = price;
		p->volume = volume;
		p->area = area;
		if (volume < 0)
		{
			// consumer
			_consumers.push_back(p);
			area->consumers.push_back(p);
		}
		else
		{
			// producer
			_supplies.push_back(p);
			area->supplies.push_back(p);
		}
	}
}