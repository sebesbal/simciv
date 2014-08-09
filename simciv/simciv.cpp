// simciv.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
#include "gclient.h"
#include "common.h"

using namespace std;

class World;


class Road;
const double max_price = 100000000000;


struct Area
{
	Area(): p(-1), p_dem(0), p_sup(max_price), v_dem(0), v_sup(0), prod_p_dem(0), prod_v_dem(0), prod_p_sup(max_price), prod_v_sup(0), v(0) { }

	double p;
	double p_dem; // price demand
	double p_sup; // supply

	double v_dem; // volume
	double v_sup;
	double v;

	double prod_p_dem; // production
	double prod_v_dem;
	double prod_p_sup;
	double prod_v_sup;

	int x;
	int y;
	vector<Road*> road;
};

struct Road
{
	Road(): t(0) { }
	Area* a;
	Area* b;
	double t; // transport
	Area* other(Area* a) { return a == this->a ? b : this->a; }
};

const int n = 10;
const double trans_price = 1.0;
const double scale = 100.0;

class World
{
public:
	World()
	{
		GD.create();
		GD.resetall();
		for (int y = 0; y < n; ++y)
		{
			for (int x = 0; x < n; ++x)
			{
				Area* a = new Area();
				a->x = x;
				a->y = y;
				area.push_back(a);

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

		Area* a = get_area(2, 5);
		a->p_dem = a->prod_p_dem = 100;
		a->prod_v_dem = 100;

		a = get_area(2, 8);
		a->p_sup = a->prod_p_sup = 10;
		a->prod_v_sup = 100;

		a = get_area(8, 5);
		a->p_sup = a->prod_p_sup = 10;
		a->prod_v_sup = 100;

		a = get_area(8, 2);
		a->p_dem = a->prod_p_dem = 100;
		a->prod_v_dem = 100;
	}
	~World()
	{
		GD.close();
	}
	void start()
	{
		//draw_roads();
		//return;

		for (int i = 0; i < 100; ++i)
		{
			step = i;
			for (int k = 0; k < 10; ++k)
			{
				iterate();
			}
			draw();
		}

		//for (int i = 0; i < 100; ++i)
		//{
		//	step = i;
		//	draw();
		//	iterate();
		//}
	}
private:
	int step;
	Area* get_area(int x, int y)
	{
		return area[y * n + x];
	}
	//double price[n][n];
	//double production[n][n];
	vector<Road*> road;
	vector<Area*> area;

	void add_road(Area* a, Area* b)
	{
		Road* r = new Road();
		r->a = a;
		r->b = b;
		road.push_back(r);
		a->road.push_back(r);
		b->road.push_back(r);
	}

	void add_road2(Area* a, Area* b)
	{
		add_road(a, b);
		add_road(b, a);
	}

	double price(double p_sup, double v_sup, double p_dem, double v_dem)
	{
		double v = v_sup + v_dem;
		if (p_sup == max_price || p_dem == 0)
		{
			return -1;
		}
		else if (v == 0)
		{
			return (p_sup + p_dem) / 2;
		}
		else
		{
			double x = v_dem / v - 0.5;
			double k = 6; // if the k is bigger, the sigmoid is "sharper"
			double d = tanh(k * x);
			double alpha = (d + 1) / 2;
			return alpha * p_dem + (1 - alpha) * p_sup;
		}
	}

	void iterate()
	{
		// modify transport
		for (Road* r: road)
		{
			Area* a = r->a;
			Area* b = r->b;
			if (a->p > 0 && b->p > 0)
			{
				double dp = b->p - a->p;
				if (dp > trans_price)
				{
					r->t += 0.1 * (dp - trans_price);
				}
				else if (dp < -trans_price)
				{
					r->t += 0.1 * (dp + trans_price);
				}
				else
				{
					r->t *= 0.8;
				}
			}
		}
		
		for (Area* a: area)
		{
			double v_sup = 0;
			double v_dem = 0;
			double m_sup = 0; // money
			double m_dem = 0;
			double min_p_sup = a->prod_p_sup;
			double max_p_dem = a->prod_p_dem;

			for (Road* r: a->road)
			{
				Area* b = r->other(a);
				double t = r->t;
				double dt = abs(t);

				if (t == 0)
				{
					if (v_sup == 0)
					{
						min_p_sup = min(min_p_sup, b->p_sup + trans_price);
					}
					if (v_dem == 0)
					{
						if (b->p_dem > 0)
						{
							max_p_dem = max(max_p_dem, b->p_dem + trans_price);
						}
					}
				}
				else if (!(t > 0 ^ r->a == a))
				{
					// a --> b
					v_dem += dt;
					m_dem += dt * (b->p_dem + trans_price);
				}
				else
				{
					// b --> a
					v_sup += dt;
					m_sup += dt * (b->p_sup + trans_price);
				}
			}

			v_sup += a->prod_v_sup;
			m_sup += a->prod_v_sup * a->prod_p_sup;

			v_dem += a->prod_v_dem;
			m_dem += a->prod_v_dem * a->prod_p_dem;

			// modify sup price
			if (v_sup == 0)
			{
				if (min_p_sup != max_price)
				{
					a->p_sup = min_p_sup;
				}
			}
			else
			{
				a->p_sup = m_sup / v_sup;
			}

			a->v = v_dem + v_sup;

			// modify dem price
			if (v_dem == 0)
			{
				if (a->p_dem == 0)
				{
					a->p_dem = max_p_dem;
				}
			}
			else
			{
				a->p_dem = m_dem / v_dem;
			}

			// modify price
			double new_p = price(a->p_sup, v_sup, a->p_dem, v_dem);
			double alpha = 0.1;
			a->p = (1 - alpha) * a->p + alpha * new_p;
		}
	}

	int price_to_color(double price)
	{
		// return hue2color(10 + 10 * price, 255);
		return hue2color(0xFF, 10 + price);
	}

	gdebug_client GD;
	void draw()
	{
		int c = 0xFF00FF;

		GD.text(0, 0, -20, c, "0: Price");
		GD.text(1, 0, -20, c, "1: Volume");
		GD.text(2, 0, -20, c, "2: Price Supply");
		GD.text(3, 0, -20, c, "3: Price Demand");

		//GD.clearbg(0, 0);
		for (Area* a: area)
		{
			GD.point(0, scale * a->x, scale * a->y, price_to_color(a->p), 3, step);
			GD.textnum(0, scale * a->x, scale * a->y + 5, c, a->p, step);

			GD.point(1, scale * a->x, scale * a->y, price_to_color(a->v), 3, step);
			GD.textnum(1, scale * a->x, scale * a->y + 5, c, a->v, step);

			GD.point(2, scale * a->x, scale * a->y, price_to_color(a->p_sup), 3, step);
			GD.textnum(2, scale * a->x, scale * a->y + 5, c, a->p_sup, step);

			GD.point(3, scale * a->x, scale * a->y, price_to_color(a->p_dem), 3, step);
			GD.textnum(3, scale * a->x, scale * a->y + 5, c, a->p_dem, step);

			double vx = 0;
			double vy = 0;
			for (Road* r: a->road)
			{
				double t = r->t;
				if (!(t > 0 ^ r->a == a))
				{
					Area* b = r->other(a);
					vx += (b->x - a->x) * abs(t);
					vy += (b->y - a->y) * abs(t);
				}
			}
			double s2 = 1;
			GD.vector(4, scale * a->x, scale * a->y, s2 * vx, s2 * vy, 0xFF, 1, step);
		}
		//for (Road* r: road)
		//{
		//	Area* a = r->a;
		//	Area* b = r->b;
		//	double t = r->transport;
		//	if (t > 0)
		//	{
		//		double s2 = 10.0;
		//		GD.vector(2, scale * a->x, scale * a->y, s2 * t);
		//	}
		//	GD.line(2, scale * a->x, scale * a->y, scale * b->x, scale * b->y, hue2color(0, 100 + r->transport), 1, step);
		//}
	}


	void draw_roads()
	{
		// GD.clearbg(0, 0x0);

		for (Road* r: road)
		{
			Area* a = r->a;
			Area* b = r->b;
			GD.line(0, scale * a->x, scale * a->y, scale * b->x, scale * b->y, 0xFFFFFF);
		}
		GD.flush();
	}
};


int _tmain(int argc, _TCHAR* argv[])
{
	World w;
	w.start();
	return 0;
}