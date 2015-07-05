#include "animals.h"

namespace simciv
{
	const int species_count = 5;

	void AnimalWorld::create_map(int width, int height, int prod_count)
	{
		WorldModel::create_map(width, height, prod_count);
		generate_species();
		generate_animals();
	}

	void AnimalWorld::generate_species()
	{
		for (int i = 0; i < species_count; ++i)
		{
			Species s;
			for (int j = 0; j < _products.size(); ++j)
			{
				double d = (double)rand() / RAND_MAX;
				s.production.push_back(2 * d - 1);
				double e = (double)rand() / RAND_MAX;
				s.reproduction.push_back(e);
			}
			species.push_back(s);
		}
	}

	void AnimalWorld::generate_animals()
	{
		for (auto& s: species)
		{
			for (int i = 0; i < 10; ++i)
			{
				Animal a(s);
				int area_index = rand() % _areas.size();
				a.area = _areas[area_index];
			}
		}
	}

	void AnimalWorld::add_producers(Area* a, Species& species)
	{
		for (int prod_id = 0; prod_id < _pc; ++prod_id)
		{
			auto& p = get_prod(a, prod_id);

		}
		
	}

	void AnimalWorld::remove_producers(Area* a, Species& species)
	{

	}
}