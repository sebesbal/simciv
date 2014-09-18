
#include <vector>

namespace simciv
{
	struct Road;
	struct AreaProd;
	const double max_price = 100000000000;

	struct Area
	{
		Area(int pc);
		void get_trans(int id, double& x, double& y);
		int x;
		int y;
		std::vector<Road*> _roads;
		std::vector<AreaProd> _prod;
	};

	struct AreaProd
	{
		AreaProd();
		double p;
		//double p_dem; // price demand
		//double p_sup; // supply

		double v_dem; // volume demand
		double v_sup; // volume supply
		double v;

		double prod_p_dem; // production
		double prod_v_dem;
		double prod_p_sup;
		double prod_v_sup;
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


	class WorldModel
	{
	public:
		void create_map(int width, int height, int prod_count);
		const std::vector<Road*>& roads() { return _roads; }
		const std::vector<Area*>& areas() { return _areas; }
		void end_turn();
		void end_turn_prod(int id);
		void add_supply(Area* area, int prod_id, double volume, double price);
		Area* get_area(int x, int y);
		int width() { return _width; }
		int height() { return _height; }
	protected:
		std::vector<Road*> _roads;
		std::vector<Area*> _areas;
		int _pc; ///< Product count
		int _width;
		int _height;
		void add_road(Area* a, Area* b);
	};


}