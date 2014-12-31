
#include "world_model.h"

namespace simciv
{
	//const double trans_price = 1.0;

	Area::Area(int pc)
	{
		for (int i = 0; i < pc; ++i)
		{
			_prod.push_back(AreaProd());
		}
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

	Road::Road(int pc, double t_price): t_price(t_price)
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
	
	double price(double p_sup, double v_sup, double p_dem, double v_dem)
	{
		double v = v_sup + v_dem;
		if (p_sup == max_price || p_dem == 0)
		{
			return -1;
		}
		else if (v < 0.5)
		{
			return (p_sup + p_dem) / 2;
		}
		else
		{
			double x = v_dem / v - 0.5;
			// double k = 6; // if the k is bigger, the sigmoid is "sharper"
			double k = 4;
			double d = tanh(k * x);
			double alpha = (d + 1) / 2;
			return alpha * p_dem + (1 - alpha) * p_sup;
		}
	}

	const double trans_rate = 1.0;

	void WorldModel::end_turn_prod(int id)
	{
		// modify transport
		int n = 5;
		for (int i = 0; i < n; ++i)
		for (Road* r: _roads)
		{
			double trans_price = r->t_price;
			double trans_price2 = trans_rate * trans_price;

			AreaProd& a = r->a->_prod[id];
			AreaProd& b = r->b->_prod[id];
			if (a.p > 0 && b.p > 0)
			{
				double dp = b.p - a.p;
				double dv = b.v - a.v;

				if (dp > trans_price2)
				{
					r->t[id] += std::min(0.01 * (dp - trans_price2), 0.01);
					// r->t[id] += 0.01 * (dp - trans_price2);
				}
				else if (dp < -trans_price2)
				{
					r->t[id] += std::max(0.01 * (dp + trans_price2), -0.01);
					//r->t[id] += 0.01 * (dp + trans_price2);
				}
				else if (abs(dp) < trans_price && abs(dp) > trans_price)
				{
					// Skip
				}
				else
				{
					if (i == n - 1)
					{
						if (abs(r->t[id]) > 1)
						{
							r->t[id] *= 0.99;
						}
						else
						{
							r->t[id] *= 0.5;
						}
					}
				}
			}
		}

		//for (int i = 0; i < 5; ++i)
		for (Area* area: _areas)
		{
			AreaProd& a = area->_prod[id];
			double v_sup = 0;
			double v_dem = 0;
			double m_sup = 0; // money
			double m_dem = 0;
			double min_p_sup = a.p_sup;// std::min(a.p_sup, a.prod_p_sup);
			double max_p_dem = a.p_dem;// std::max(a.p_dem, a.prod_p_dem);
			double sum_p = 0;

			for (Road* r: area->_roads)
			{
				double trans_price = trans_rate * r->t_price;
				Area* area_b = r->other(area);
				AreaProd& b = area_b->_prod[id];
				sum_p += b.p;

				double t = r->t[id];
				double dt = abs(t);

				if (t == 0)
				{
					if (v_sup == 0)
					{
						min_p_sup = std::min(min_p_sup, b.p_sup + trans_price);
					}
					if (v_dem == 0)
					{
						if (b.p_dem > 0)
						{
							max_p_dem = std::max(max_p_dem, b.p_dem - trans_price);
						}
					}
				}
				else if (!(t > 0 ^ r->a == area))
				{
					// a --> b
					v_dem += dt;
					m_dem += dt * (b.p_dem - trans_price);
				}
				else
				{
					// b --> a
					v_sup += dt;
					m_sup += dt * (b.p_sup + trans_price);
				}
			}

			sum_p = 0.5 * sum_p / 9 + 0.5 * a.p;

			v_sup += a.prod_v_sup;
			m_sup += a.prod_v_sup * a.prod_p_sup;

			v_dem += a.prod_v_dem;
			m_dem += a.prod_v_dem * a.prod_p_dem;

			double beta = 0.02;
			// modify sup price
			if (v_sup == 0)
			{
				if (min_p_sup != max_price)
				{
					a.p_sup = min_p_sup;
				}
			}
			else
			{
				a.p_sup = (1 - beta) * a.p_sup + beta * m_sup / v_sup;
			}

			a.v = v_sup - v_dem;
			a.v_sup = v_sup;
			a.v_dem = v_dem;

			// modify dem price
			if (v_dem == 0)
			{
				if (a.p_dem == 0)
				{
					// a.p_dem = max_p_dem;
					a.p_dem = max_p_dem;
				}
				else
				{
					a.p_dem *= (1 - beta);
				}
			}
			else
			{
				a.p_dem = (1 - beta) * a.p_dem + beta * m_dem / v_dem;
			}

			// modify price
			double new_p = price(a.p_sup, v_sup, a.p_dem, v_dem);
			if (new_p != -1)
			{
				// double alpha = 0.02;
				double alpha = 0.02;
				a.p = (1 - alpha) * a.p + alpha * new_p;
				// a.p = (1 - alpha) * sum_p + alpha * new_p;
			}
		}
	}

	void WorldModel::add_supply(Area* area, int prod_id, double volume, double price)
	{
		AreaProd& a = area->_prod[prod_id];
		if (volume < 0)
		{
			volume = -volume;
			//a.p_dem =	a.prod_p_dem = (a.prod_p_dem * a.prod_v_dem + volume * price) / (a.prod_p_dem + volume);
			a.p_dem = a.prod_p_dem = price;
			a.prod_v_dem += volume;
		}
		else
		{
			// a.p_sup = a.prod_p_sup = (a.prod_p_sup * a.prod_v_sup + volume * price) / (a.prod_p_sup + volume);
			a.p_sup = a.prod_p_sup = price;
			a.prod_v_sup += volume;
		}
		
	}

	Area* WorldModel::get_area(int x, int y)
	{
		return _areas[y * _width + x];
	}
}