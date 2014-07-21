// simciv.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
#include "gclient.h"
#include "common.h"

using namespace std;

class World;


class Road;


struct Area
{
	Area(): price(0), production(0), storage(0), ideal_storage(0) { }
	double price;
	double production;
	int x;
	int y;
	vector<Road*> road;
	double storage;
	double ideal_storage;
};

struct Road
{
	Road(): transport(0) { }
	Area* a;
	Area* b;
	double transport;
	Area* other(Area* a) { return a == this->a ? b : this->a; }
};

const int n = 10;
const double trans_price = 5.0;
const double scale = 30.0;

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

		Area* a = get_area(2, 2);
		a->production = -1;
		a->ideal_storage = 100;
		a->storage = 100;

		Area* b = get_area(8, 8);
		b->production = 1;
		b->ideal_storage = 0;
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

	void iterate()
	{
		for (Area* a: area)
		{
			a->storage += a->production;
			for (Road* r: a->road)
			{
				if (r->a == a)
				{
					a->storage -= r->transport;
				}
				else
				{
					a->storage += r->transport;
				}
			}

			a->price += 0.1 * (a->ideal_storage - a->storage);

			double p = 0;
			for (Road* r: a->road)
			{
				Area* b = r->other(a);
				if (b->price < p)
				{
					p = b->price;
				}
			}
			a->price = 0.5 * a->price + 0.5 * (p + trans_price);
		}

		for (Road* r: road)
		{
			Area* a = r->a;
			Area* b = r->b;
			a->storage -= r->transport;
			b->storage += r->transport;

			double p_diff = b->price - a->price;
			if (p_diff > trans_price)
			{
				r->transport += 0.01 * (p_diff - trans_price);
			}
			else if (p_diff < -trans_price)
			{
				r->transport += 0.01 * (p_diff + trans_price);
			}
			else
			{
				r->transport = 0;
			}
		}
	}

	gdebug_client GD;
	void draw()
	{
		for (Area* a: area)
		{
			GD.point(0, scale * a->x, scale * a->y, hue2color(0, 100 + a->storage), 3, step);
			GD.point(1, scale * a->x, scale * a->y, hue2color(0, 100 + a->price), 3, step);
			double vx = 0;
			double vy = 0;
			for (Road* r: a->road)
			{
				double t = r->transport;
				if (!(t > 0 ^ r->a == a))
				{
					Area* b = r->other(a);
					vx += (b->x - a->x) * abs(t);
					vy += (b->y - a->y) * abs(t);
				}
			}
			double s2 = 10.0;
			GD.vector(2, scale * a->x, scale * a->y, s2 * vx, s2 * vy, 0xFF, 1, step);
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