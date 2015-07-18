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
				int area_index = rand() % _areas.size();
				create_animal(_areas[area_index], s);
			}
		}
	}

	Animal* AnimalWorld::create_animal(Area* a, Species& species)
	{
		Animal* ani = new Animal(species);
		animals.push_back(ani);
		return ani;
	}

	void AnimalWorld::move_animal(Animal* ani, Area* new_area)
	{
		remove_producers(ani->area, ani->species);
		add_producers(new_area, ani->species);
		ani->area = new_area;
	}

	void AnimalWorld::add_producers(Area* a, Species& species)
	{
		for (int prod_id = 0; prod_id < _pc; ++prod_id)
		{
			add_prod(a, prod_id, species.production[prod_id], 0);
		}
	}

	void AnimalWorld::remove_producers(Area* a, Species& species)
	{
		for (int prod_id = 0; prod_id < _pc; ++prod_id)
		{
			remove_prod(a, prod_id, species.production[prod_id], 0);
		}
	}
}