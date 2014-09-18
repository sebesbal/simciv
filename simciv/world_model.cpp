
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
		p(0),
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

	const double trans_rate = 1.00;

	void WorldModel::end_turn_prod(int id)
	{
		// modify transport
		int n = 5;
		for (int i = 0; i < n; ++i)
		for (Road* r: _roads)
		{
			double trans_price = r->t_price;
			double trans_price2 = trans_rate;
			const double eps = 0.001;

			AreaProd& a = r->a->_prod[id];
			AreaProd& b = r->b->_prod[id];

			if (a.p > 0 || b.p > 0)
			{
				double dp = b.p - a.p;
				double dv = a.v - b.v;

				if (dp > trans_price2 && dv > 0)
				{
					// r->t[id] += std::min(0.01 * (dp - trans_price2) * (1), 1.0);
					r->t[id] += 0.01 * (dp - trans_price2) * dv;
				}
				else if (dp < -trans_price2 && dv < 0)
				{
					r->t[id] += 0.01 * (dp + trans_price2) * (- dv);
				}
				else if (abs(dp) < trans_price && abs(dp) > trans_price)
				{
					// Skip
					// r->t[id] *= 0.1;
				}
				else
				{
					r->t[id] *= 0.1;
					//if (i == n - 1)
					//{
					//	if (abs(r->t[id]) > 1)
					//	{
					//		r->t[id] *= 0.99;
					//	}
					//	else
					//	{
					//		r->t[id] *= 0.5;
					//	}
					//}
				}
			}
		}

		//for (int i = 0; i < 5; ++i)
		for (Area* area: _areas)
		{
			AreaProd& a = area->_prod[id];
			double v_dem = 0; // volume
			double v_sup = 0; // volume
			double m = 0; // money
			double sum_v = 0;

			for (Road* r: area->_roads)
			{
				double trans_price = r->t_price * trans_rate;
				Area* area_b = r->other(area);
				AreaProd& b = area_b->_prod[id];

				double t = r->t[id];
				double dt = abs(t);
				double dv = b.v - a.v;
				//double b_v = b.v_dem + b.v_sup;
				//sum_v += b_v;

				if (t == 0)
				{
					// Skip
				}
				//else if (dv < 0)
				{
					// b-ben nagyobb a hiány mint a-ban, a-->b
					m += b.v_dem * (b.p - trans_price);
					sum_v += b.v_dem;
				}
				//else
				{
					// b --> a
					m += b.v_sup * (b.p + trans_price);
					sum_v += b.v_sup;
				}

				const double f = 0.0;
				if (! (t > 0 ^ r->a == area))
				{
					// a --> b
					v_dem += dt;
					sum_v += f * dt;
					m += f * dt * (b.p - trans_price);
				}
				else
				{
					// b --> a
					v_sup += dt;
					sum_v += f * dt;
					m += f * dt * (b.p + trans_price);
				}
			}

			v_sup += a.prod_v_sup;
			m += a.prod_v_sup * a.prod_p_sup;
			sum_v += a.prod_v_sup;
			a.v_sup = v_sup;

			v_dem += a.prod_v_dem;
			m += a.prod_v_dem * a.prod_p_dem;
			sum_v += a.prod_v_dem;
			a.v_dem = v_dem;

			a.v = v_sup - v_dem;

			if (sum_v == 0)
			{
				// Skip
			}
			else
			{
				double new_p = m / sum_v;
				if (new_p > 0)
				{
					double alpha = 0.5;
					a.p = (1 - alpha) * a.p + alpha * new_p;
				}
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
			a.prod_p_dem = price;
			a.prod_v_dem += volume;
		}
		else
		{
			// a.p_sup = a.prod_p_sup = (a.prod_p_sup * a.prod_v_sup + volume * price) / (a.prod_p_sup + volume);
			a.prod_p_sup = price;
			a.prod_v_sup += volume;
		}
		
	}

	Area* WorldModel::get_area(int x, int y)
	{
		return _areas[y * _width + x];
	}
}