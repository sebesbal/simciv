
#include "world_model.h"
#include <queue>
#include "economy.h"

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

	Road::Road(int pc, double t_price): t_price(t_price)
	{
		for (int i = 0; i < pc; ++i)
		{
			t.push_back(0);
		}
	}

	WorldModel::WorldModel()
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

				//std::vector<AreaProd> p;
				//for (int i = 0; i < _pc; ++i)
				//{
				//	p.push_back(AreaProd());
				//}
				//_production->push_back(p);
				//_new_production->push_back(p);

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

		for (int i = 0; i < prod_count; ++i)
		{
			_products.push_back( new ProductMap(*this) );
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
	
	const double trans_rate = 1.0;

	Area* WorldModel::get_area(int x, int y)
	{
		return _areas[y * _width + x];
	}

	AreaProd& WorldModel::get_prod(Area* a, int id)
	{
		return _products[id]->get_prod(a);
		//return (*_production)[a->index][id];
	}

	void WorldModel::end_turn()
	{
		static int k = 0;
		if (k++ % 1 == 0)
		{
			for (ProductMap* product: _products)
			{
				product->update();
			} 
		}
	}

	void WorldModel::get_distances(Node* src, Node* g)
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

	Route* WorldModel::get_route(Node* src, Node* dst, Node* g)
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
		//route->volume = 0;

		return route;
	}

	void WorldModel::add_supply(Area* area, int prod_id, double volume, double price)
	{
		_products[prod_id]->add_supply(area, volume, price);
	}
}