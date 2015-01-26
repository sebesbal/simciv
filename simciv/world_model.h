
#include <vector>

namespace simciv
{
	struct Road;
	struct AreaProd;
	struct Producer;
	const double max_price = 100000000000;

	struct Area
	{
		Area(int index, int pc);
		void get_trans(int id, double& x, double& y);
		int index;
		int x;
		int y;
		std::vector<Road*> _roads;
		std::vector<Producer*> supplies;
		std::vector<Producer*> consumers;
		//std::vector<AreaProd> _prod;
	};

	struct AreaProd
	{
		AreaProd();
		double p;
		double p_con; // price demand
		double p_sup; // supply

		double v_con; // volume demand
		double v_sup; // volume supply
		double v;

		double m_con; // the profit of the consumer's worst deals in this area
		double m_sup; // the profit of the suppliers's worst deals in this area
	};

	struct Producer
	{
		Area* area;
		double volume; // negative volume means consumer
		double free_volume;
		double price; // the current price
		double profit; // the worst profit of the producers deals
		double partner_price;
	};

	struct Road
	{
		Road(int pc, double t_price);
		Area* a;
		Area* b;
		std::vector<double> t; // transport
		double t_price;
		Area* other(Area* a) { return a == this->a ? b : this->a; }
	}; 

	struct Route
	{
		Producer* sup;
		Producer* dem;
		std::vector<Road*> roads;
		double volume;
		//double p_sup;
		//double p_con;
		double trans_price;
		double profit;
	};

	class WorldModel
	{
	public:
		WorldModel();
		void create_map(int width, int height, int prod_count);
		const std::vector<Road*>& roads() { return _roads; }
		const std::vector<Area*>& areas() { return _areas; }
		virtual void end_turn();
		virtual void add_supply(Area* area, int prod_id, double volume, double price) = 0;
		Area* get_area(int x, int y);
		int width() { return _width; }
		int height() { return _height; }
		AreaProd& get_prod(Area* a, int id);
		AreaProd& get_new_prod(Area* a, int id);
	protected:
		
		virtual void end_turn_prod(int id) = 0;
		std::vector<Road*> _roads;
		std::vector<Area*> _areas;
		std::vector<std::vector<AreaProd>>* _production;
		std::vector<std::vector<AreaProd>>* _new_production;
		int _pc; ///< Product count
		int _width;
		int _height;
		void add_road(Area* a, Area* b);
	};

	/// Producer-consumer pairs
	class World2: public WorldModel
	{
	public:
		virtual void add_supply(Area* area, int prod_id, double volume, double price) override;
		virtual void end_turn() override;
	protected:
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
		
		virtual void end_turn_prod(int id) override;
		void get_distances(Node* src, Node* g);
		Route* get_route(Node* src, Node* dst, Node* g);
		void update_routes();
		void routes_to_areas();
		void update_prices();
		std::vector<Producer*> _supplies;
		std::vector<Producer*> _consumers;
		std::vector<Route*> _routes; // "all" possible routes. volume > 0 means that the route is used
	};
}