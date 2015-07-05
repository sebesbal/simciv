#pragma once
#include "world_model.h"

namespace simciv
{

	struct Species
	{
		t_prods production; //< products materials form materials
		t_prods reproduction; //< cost of reproduction = const * self-preservation
	};

	struct Animal
	{
		Animal(Species& species): species(species) { }
		Species& species;
		std::vector<double> storage;
		// std::vector<Producer*> producers;
		Area* area;
	};

	class AnimalWorld: public WorldModel
	{
	public:
		virtual void create_map(int width, int height, int prod_count) override;
		void generate_species();
		void generate_animals();
	protected:
		void add_producers(Area* a, Species& species);
		void remove_producers(Area* a, Species& species);
		std::vector<Species> species;
	};

}