
#include <string>
#include <vector>

/*

namespace simciv
{
	struct Area
	{
		int id;
		double x;
		double y;
		std::vector<Road*> roads;
	};

	struct Road
	{
		Area* a;
		Area* b;
		double cost_ab;
		double cost_ba;
	};

	struct Source
	{
		Area* area;
		double storage;
		double price;
		double production; // negative if consumes
	};

	struct Product
	{
		std::string name;
		std::vector<Source> sources; // producers and consumers
	};

	struct Step
	{
		Road* road;
		bool a_to_b;
	};

	struct Transport
	{
		Product* product;
		std::vector<Step> route;
		double amount;
		double cost;
	};

	class World
	{
		std::vector<Area> areas;
		std::vector<Road> roads;
		std::vector<Product> products;
		std::vector<Transport> transports;
	private:
		void iteration(Product& product);
	};
}

*/