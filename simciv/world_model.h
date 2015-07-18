#pragma once
#include <vector>

namespace simciv
{
	struct Road;
	struct AreaProd;
	struct Producer;
	class ProductMap;
	class Route;
	const double max_price = 100000000000;

	typedef std::vector<double> t_prods;

	struct Area
	{
		Area(int index, int pc);
		void get_trans(int id, double& x, double& y);
		int index;
		int x;
		int y;
		std::vector<Road*> _roads;
	};

	struct Road
	{
		Road(int pc, double t_price);
		Area* a;
		Area* b;
		t_prods t; // transport
		double t_price;
		Area* other(Area* a) { return a == this->a ? b : this->a; }
	};

	struct Node
	{
		Node(): area(NULL), parent(NULL), color(0), d(0) {}
		Road* parent;
		Area* area;
		double d;
		int color; // 0 = black, unvisited, 1 = gray, opened, 2 = white, visited
	};
		
	class NodeComparator
	{
	public:
		bool operator()(const Node* a, const Node* b)
		{
			return (a->d > b->d);
		}
	};

	struct Route
	{
		std::vector<Road*> roads;
		double trans_price;
	};

	/// The map. Graph of areas and roads.
	class WorldModel
	{
	public:
		WorldModel();
		virtual void create_map(int width, int height, int prod_count);
		const std::vector<Road*>& roads() { return _roads; }
		const std::vector<Area*>& areas() { return _areas; }
		virtual void end_turn();
		virtual void add_prod(Area* area, int prod_id, double volume, double price);
		virtual void remove_prod(Area* area, int prod_id, double volume, double price);
		Area* get_area(int x, int y);
		int width() { return _width; }
		int height() { return _height; }
		AreaProd& get_prod(Area* a, int id);
		void get_distances(Node* src, Node* g);
		Route* get_route(Node* src, Node* dst, Node* g);
		std::vector<ProductMap*>& products() { return _products; }
	protected:
		std::vector<ProductMap*> _products;
		std::vector<Road*> _roads;
		std::vector<Area*> _areas;
		int _pc; ///< Product count
		int _width;
		int _height;
		void add_road(Area* a, Area* b);
	};
}