#include <tuple>
#include <vector>

// Set the id of the block_id of an accessor* at the given position if the 
// current id at the given position is id_air, and the given hitbox does not
// intersect with the block to be placed.

void set_id_safe_if_air_no_collision(accessor* out, hitbox player, unsigned int x, unsigned int y, unsigned int z, block_id id)
{
	if (out->the_world->get_id_safe(x, y, z) == id_air)
	{
		if (!hitbox_intersect(player, hitbox(x, y, z, 1.0f, 1.0f, 1.0f)))
		{
			out->set_id_safe(x, y, z, id);
		}
	}
}

// Plant a tree.

void plant_tree_accessor(accessor* out, hitbox player, unsigned int x, unsigned int y, unsigned int z, block_id tree_leaf, block_id tree_log)
{
	// It's grass, we may plant a tree if there are enough 
	// blocks available above the grass.

	if (rand() % 2 == 0)
	{
		// Original tree.

		if (!out->the_world->in_bounds(x, y - 6, z))
		{
			return;
		} 

		// This is the trunk.

		set_id_safe_if_air_no_collision(out, player, x, y - 1, z, tree_log);
		set_id_safe_if_air_no_collision(out, player, x, y - 2, z, tree_log);
		set_id_safe_if_air_no_collision(out, player, x, y - 3, z, tree_log);
		set_id_safe_if_air_no_collision(out, player, x, y - 4, z, tree_log);
		set_id_safe_if_air_no_collision(out, player, x, y - 5, z, tree_log);

		// This is the cross at the top.

		set_id_safe_if_air_no_collision(out, player, x, y - 6, z, tree_leaf);

		set_id_safe_if_air_no_collision(out, player, x - 1, y - 6, z, tree_leaf);
		set_id_safe_if_air_no_collision(out, player, x + 1, y - 6, z, tree_leaf);

		set_id_safe_if_air_no_collision(out, player, x, y - 6, z - 1, tree_leaf);
		set_id_safe_if_air_no_collision(out, player, x, y - 6, z + 1, tree_leaf);

		// This is the square at the layer second from the 
		// top.

		for (int j = -1; j <= 1; j++)
		{
			for (int k = -1; k <= 1; k++)
			{
				if (j == 0 && k == 0)
				{
					continue;
				}
				
				set_id_safe_if_air_no_collision(out, player, x + j, y - 5, z + k, tree_leaf);
			}
		}

		// These are the squares at the third and fourth 
		// layer from the top.

		for (int j = -2; j <= 2; j++)
		{
			for (int k = -2; k <= 2; k++)
			{
				if (j == 0 && k == 0)
				{
					continue;
				}

				set_id_safe_if_air_no_collision(out, player, x + j, y - 4, z + k, tree_leaf);
				set_id_safe_if_air_no_collision(out, player, x + j, y - 3, z + k, tree_leaf);
			}
		}
	}
	else
	{
		// Rounded tree.

		if (!out->the_world->in_bounds(x, y - 7, z))
		{
			return;
		} 

		// This is the trunk.

		set_id_safe_if_air_no_collision(out, player, x, y - 1, z, tree_log);
		set_id_safe_if_air_no_collision(out, player, x, y - 2, z, tree_log);
		set_id_safe_if_air_no_collision(out, player, x, y - 3, z, tree_log);
		set_id_safe_if_air_no_collision(out, player, x, y - 4, z, tree_log);
		set_id_safe_if_air_no_collision(out, player, x, y - 5, z, tree_log);
		set_id_safe_if_air_no_collision(out, player, x, y - 6, z, tree_log);

		// This is the cross at the top.

		set_id_safe_if_air_no_collision(out, player, x, y - 7, z, tree_leaf);

		set_id_safe_if_air_no_collision(out, player, x - 1, y - 7, z, tree_leaf);
		set_id_safe_if_air_no_collision(out, player, x + 1, y - 7, z, tree_leaf);

		set_id_safe_if_air_no_collision(out, player, x, y - 7, z - 1, tree_leaf);
		set_id_safe_if_air_no_collision(out, player, x, y - 7, z + 1, tree_leaf);

		// This is the square at the layer second from the 
		// top.

		for (int j = -1; j <= 1; j++)
		{
			for (int k = -1; k <= 1; k++)
			{
				if (j == 0 && k == 0)
				{
					continue;
				}
				
				set_id_safe_if_air_no_collision(out, player, x + j, y - 6, z + k, tree_leaf);
			}
		}

		// These are the squares at the third and fourth 
		// layer from the top.

		for (int j = -2; j <= 2; j++)
		{
			for (int k = -2; k <= 2; k++)
			{
				if (j == 0 && k == 0)
				{
					continue;
				}

				set_id_safe_if_air_no_collision(out, player, x + j, y - 5, z + k, tree_leaf);
				set_id_safe_if_air_no_collision(out, player, x + j, y - 4, z + k, tree_leaf);
			}
		}

		// This is the square at the fifth layer from the top.
		// It has it's corners removed.

		for (int j = -2; j <= 2; j++)
		{
			for (int k = -2; k <= 2; k++)
			{
				if (j == 0 && k == 0)
				{
					continue;
				}
				else if ((j == 2 && k == 2) || (j == 2 && k == -2) || (j == -2 && k == 2) || (j == -2 && k == -2))
				{
					continue;
				}
				
				set_id_safe_if_air_no_collision(out, player, x + j, y - 3, z + k, tree_leaf);
			}
		}
	}
}

// Generate a world using a seed.

void generate_world(world* out, unsigned int seed)
{
	// Use rand() and RAND_MAX for white noise.

	srand(seed);

	// Use FastNoise for fractal Simplex noise.

	FastNoise noise;

	noise.SetSeed(seed);

	noise.SetNoiseType(FastNoise::SimplexFractal);

	noise.SetFractalOctaves(2);

	// A small easter egg: sometimes mushroom worlds will spawn!

	bool mushroom_world = rand() % 1024 == 0;

	// Generate the base terrain.

	float frequency = 2.0f;

	for (float x = 0.0f; x < float(out->x_res); x += 1.0f)
	for (float y = 0.0f; y < float(out->y_res); y += 1.0f)
	for (float z = 0.0f; z < float(out->z_res); z += 1.0f)
	{
		if (pow(y / float(out->y_res), 1.1024f) + noise.GetValueFractal(x * frequency, y * frequency, z * frequency) * 0.60f > 0.5f)
		{
			out->set_id(int(x), int(y), int(z), id_stone);
		}
	}

	// Set the highground blocks.

	for (float x = 0.0f; x < float(out->x_res); x += 1.0f)
	for (float z = 0.0f; z < float(out->z_res); z += 1.0f)
	for (float y = 0.0f; y < float(out->y_res); y += 1.0f)
	{
		if (out->get_id(int(x), int(y), int(z)) != id_air)
		{
			if (y / float(out->y_res) > 0.6f)
			{
				if (noise.GetValueFractal(x * frequency, y * frequency) >= -0.2f)
				{
					out->set_id_safe(int(x), int(y), int(z), id_sand);

					out->set_id_safe_if_not_air(int(x), int(y) + 1, int(z), id_sandstone);
					out->set_id_safe_if_not_air(int(x), int(y) + 2, int(z), id_sandstone);
					out->set_id_safe_if_not_air(int(x), int(y) + 3, int(z), id_sandstone);
				}
				else
				{
					out->set_id_safe(int(x), int(y), int(z), id_gravel);

					out->set_id_safe_if_not_air(int(x), int(y) + 1, int(z), id_gravel);
					out->set_id_safe_if_not_air(int(x), int(y) + 2, int(z), id_gravel);
					out->set_id_safe_if_not_air(int(x), int(y) + 3, int(z), id_gravel);
				}
			}
			else
			{
				if (mushroom_world)
				{
					out->set_id_safe(int(x), int(y), int(z), id_mycelium);
				}
				else
				{
					out->set_id_safe(int(x), int(y), int(z), id_grass);
				}

				out->set_id_safe_if_not_air(int(x), int(y) + 1, int(z), id_dirt);
				out->set_id_safe_if_not_air(int(x), int(y) + 2, int(z), id_dirt);
				out->set_id_safe_if_not_air(int(x), int(y) + 3, int(z), id_dirt);
			}

			break;
		}
	}

	// Create beaches and top-level water.

	std::vector<std::tuple<unsigned int, unsigned int, unsigned int>> flood_water;

	for (float x = 0.0f; x < float(out->x_res); x += 1.0f)
	for (float z = 0.0f; z < float(out->z_res); z += 1.0f)
	for (float y = 0.0f; y < float(out->y_res); y += 1.0f)
	{
		block_id current_block = out->get_id(int(x), int(y), int(z));

		if (current_block != id_air)
		{
			if ((current_block == id_grass || current_block == id_mycelium) && y / float(out->y_res) > 0.5f)
			{
				out->set_id(int(x), int(y), int(z), id_sand);

				// Plant reeds, perhaps?

				if (out->get_id_safe(int(x), int(y) - 1, int(z)) == id_air && rand() % 2 == 0)
				{
					int reed_height = rand() % 4 + 1;

					for (int i = 1; i < reed_height + 1; i++)
					{
						out->set_id_safe_if_air(int(x), int(y) - i, int(z), id_reeds);
					}
				}
			}

			break;
		}
		else if (y / float(out->y_res) > 0.5f)
		{
			flood_water.push_back(std::tuple<unsigned int, unsigned int, unsigned int>(x, y, z));

			out->set_id(int(x), int(y), int(z), id_water);
			
			break;
		}
	}

	// Create submerged beaches and flood water.

	while (flood_water.size() > 0)
	{
		unsigned int x = std::get<0>(flood_water[flood_water.size() - 1]);
		unsigned int y = std::get<1>(flood_water[flood_water.size() - 1]);
		unsigned int z = std::get<2>(flood_water[flood_water.size() - 1]);

		flood_water.pop_back();

		// Right neighbor.

		block_id right = out->get_id_safe(x + 1, y, z);

		if (right == id_air)
		{
			out->set_id(x + 1, y, z, id_water);

			flood_water.push_back(std::tuple<unsigned int, unsigned int, unsigned int>(x + 1, y, z));
		}
		else if (right != id_water)
		{
			out->set_id_safe(x + 1, y, z, id_sand);
		}

		// Left neighbor.

		block_id left = out->get_id_safe(x - 1, y, z);

		if (left == id_air)
		{
			out->set_id(x - 1, y, z, id_water);

			flood_water.push_back(std::tuple<unsigned int, unsigned int, unsigned int>(x - 1, y, z));
		}
		else if (left != id_water)
		{
			out->set_id_safe(x - 1, y, z, id_sand);
		}

		// Front neighbor.

		block_id front = out->get_id_safe(x, y, z + 1);

		if (front == id_air)
		{
			out->set_id(x, y, z + 1, id_water);

			flood_water.push_back(std::tuple<unsigned int, unsigned int, unsigned int>(x, y, z + 1));
		}
		else if (front != id_water)
		{
			out->set_id_safe(x, y, z + 1, id_sand);
		}

		// Back neighbor.

		block_id back = out->get_id_safe(x, y, z - 1);

		if (back == id_air)
		{
			out->set_id(x, y, z - 1, id_water);

			flood_water.push_back(std::tuple<unsigned int, unsigned int, unsigned int>(x, y, z - 1));
		}
		else if (back != id_water)
		{
			out->set_id_safe(x, y, z - 1, id_sand);
		}

		// Bottom neighbor.

		block_id bottom = out->get_id_safe(x, y + 1, z);

		if (bottom == id_air)
		{
			out->set_id(x, y + 1, z, id_water);

			flood_water.push_back(std::tuple<unsigned int, unsigned int, unsigned int>(x, y + 1, z));
		}
		else if (bottom != id_water)
		{
			out->set_id_safe(x, y + 1, z, id_sand);
		}
	}

	// Plant trees.

	for (int i = 0; i < out->x_res * out->z_res / 32; i++)
	{
		tree:

		int x = rand() % out->x_res;
		int z = rand() % out->z_res;

		block_id tree_leaf;

		block_id tree_log;

		if (rand() % 3 == 0)
		{
			tree_leaf = id_birch_leaves;

			tree_log = id_birch_log;
		}
		else
		{
			tree_leaf = id_oak_leaves;

			tree_log = id_oak_log;
		}

		// Find the highest layer.

		for (int y = 0; y < out->y_res; y++)
		{
			if (out->get_id(x, y, z) != id_air)
			{
				// Okay, found something that is not air.

				if (out->get_id(x, y, z) == id_grass)
				{
					// It's grass, we may plant a tree if there are enough 
					// blocks available above the grass.

					if (rand() % 2 == 0)
					{
						// Original tree.

						if (!out->in_bounds(x, y - 6, z))
						{
							goto tree;
						} 

						// This is the trunk.

						out->set_id_safe_if_air(x, y - 1, z, tree_log);
						out->set_id_safe_if_air(x, y - 2, z, tree_log);
						out->set_id_safe_if_air(x, y - 3, z, tree_log);
						out->set_id_safe_if_air(x, y - 4, z, tree_log);
						out->set_id_safe_if_air(x, y - 5, z, tree_log);

						// This is the cross at the top.

						out->set_id_safe_if_air(x, y - 6, z, tree_leaf);

						out->set_id_safe_if_air(x - 1, y - 6, z, tree_leaf);
						out->set_id_safe_if_air(x + 1, y - 6, z, tree_leaf);

						out->set_id_safe_if_air(x, y - 6, z - 1, tree_leaf);
						out->set_id_safe_if_air(x, y - 6, z + 1, tree_leaf);

						// This is the square at the layer second from the 
						// top.

						for (int j = -1; j <= 1; j++)
						{
							for (int k = -1; k <= 1; k++)
							{
								if (j == 0 && k == 0)
								{
									continue;
								}
								
								out->set_id_safe_if_air(x + j, y - 5, z + k, tree_leaf);
							}
						}

						// These are the squares at the third and fourth 
						// layer from the top.

						for (int j = -2; j <= 2; j++)
						{
							for (int k = -2; k <= 2; k++)
							{
								if (j == 0 && k == 0)
								{
									continue;
								}

								out->set_id_safe_if_air(x + j, y - 4, z + k, tree_leaf);
								out->set_id_safe_if_air(x + j, y - 3, z + k, tree_leaf);
							}
						}
					}
					else
					{
						// Rounded tree.

						if (!out->in_bounds(x, y - 7, z))
						{
							goto tree;
						} 

						// This is the trunk.

						out->set_id_safe_if_air(x, y - 1, z, tree_log);
						out->set_id_safe_if_air(x, y - 2, z, tree_log);
						out->set_id_safe_if_air(x, y - 3, z, tree_log);
						out->set_id_safe_if_air(x, y - 4, z, tree_log);
						out->set_id_safe_if_air(x, y - 5, z, tree_log);
						out->set_id_safe_if_air(x, y - 6, z, tree_log);

						// This is the cross at the top.

						out->set_id_safe_if_air(x, y - 7, z, tree_leaf);

						out->set_id_safe_if_air(x - 1, y - 7, z, tree_leaf);
						out->set_id_safe_if_air(x + 1, y - 7, z, tree_leaf);

						out->set_id_safe_if_air(x, y - 7, z - 1, tree_leaf);
						out->set_id_safe_if_air(x, y - 7, z + 1, tree_leaf);

						// This is the square at the layer second from the 
						// top.

						for (int j = -1; j <= 1; j++)
						{
							for (int k = -1; k <= 1; k++)
							{
								if (j == 0 && k == 0)
								{
									continue;
								}
								
								out->set_id_safe_if_air(x + j, y - 6, z + k, tree_leaf);
							}
						}

						// These are the squares at the third and fourth 
						// layer from the top.

						for (int j = -2; j <= 2; j++)
						{
							for (int k = -2; k <= 2; k++)
							{
								if (j == 0 && k == 0)
								{
									continue;
								}

								out->set_id_safe_if_air(x + j, y - 5, z + k, tree_leaf);
								out->set_id_safe_if_air(x + j, y - 4, z + k, tree_leaf);
							}
						}

						// This is the square at the fifth layer from the top.
						// It has it's corners removed.

						for (int j = -2; j <= 2; j++)
						{
							for (int k = -2; k <= 2; k++)
							{
								if (j == 0 && k == 0)
								{
									continue;
								}
								else if ((j == 2 && k == 2) || (j == 2 && k == -2) || (j == -2 && k == 2) || (j == -2 && k == -2))
								{
									continue;
								}
								
								out->set_id_safe_if_air(x + j, y - 3, z + k, tree_leaf);
							}
						}
					}
				}

				break;
			}
		}
	}

	// Plant large mushrooms.

	int mushroom_count = out->x_res * out->z_res / 1024;

	if (mushroom_world)
	{
		mushroom_count = out->z_res * out->z_res / 32;
	}

	for (int i = 0; i < mushroom_count; i++)
	{
		mushroom:

		int x = rand() % out->x_res;
		int z = rand() % out->z_res;

		for (int y = 0; y < out->y_res; y++)
		{
			if (out->get_id(x, y, z) != id_air)
			{
				// Okay, found something that is not air.

				if (out->get_id(x, y, z) == id_grass || out->get_id(x, y, z) == id_mycelium)
				{
					// It's grass, we may plant a mushroom if there are enough
					// blocks available above the grass.

					if (!out->in_bounds(x, y - 6, z))
					{
						goto mushroom;
					}

					if (rand() % 5 != 0)
					{
						// Red mushroom. This is the trunk.

						out->set_id_safe_if_air(x, y - 1, z, id_mushroom_stem);
						out->set_id_safe_if_air(x, y - 2, z, id_mushroom_stem);
						out->set_id_safe_if_air(x, y - 3, z, id_mushroom_stem);
						out->set_id_safe_if_air(x, y - 4, z, id_mushroom_stem);
						out->set_id_safe_if_air(x, y - 5, z, id_mushroom_stem);

						// These are the five squares.

						for (int j = -1; j <= 1; j++)
						{
							for (int k = -1; k <= 1; k++)
							{
								// Top square.

								out->set_id_safe_if_air(x + j, y - 6, z + k, id_red_mushroom_block);

								// Left square.

								out->set_id_safe_if_air(x - 2, y - 4 + j, z + k, id_red_mushroom_block);

								out->set_id_safe_if_air(x - 1, y - 4 + j, z + k, id_air);

								// Right square.

								out->set_id_safe_if_air(x + 2, y - 4 + j, z + k, id_red_mushroom_block);

								out->set_id_safe_if_air(x + 1, y - 4 + j, z + k, id_air);

								// Front square.

								out->set_id_safe_if_air(x + k, y - 4 + j, z - 2, id_red_mushroom_block);

								out->set_id_safe_if_air(x + k, y - 4 + j, z - 1, id_air);

								// Back square.

								out->set_id_safe_if_air(x + k, y - 4 + j, z + 2, id_red_mushroom_block);

								out->set_id_safe_if_air(x + k, y - 4 + j, z + 1, id_air);
							}
						}
					}
					else
					{
						// Brown mushroom. This is the trunk.

						out->set_id_safe_if_air(x, y - 1, z, id_mushroom_stem);
						out->set_id_safe_if_air(x, y - 2, z, id_mushroom_stem);
						out->set_id_safe_if_air(x, y - 3, z, id_mushroom_stem);
						out->set_id_safe_if_air(x, y - 4, z, id_mushroom_stem);
						out->set_id_safe_if_air(x, y - 5, z, id_mushroom_stem);

						for (int j = -3; j <= 3; j++)
						{
							for (int k = -3; k <= 3; k++)
							{
								if ((j == 3 && k == 3) || (j == 3 && k == -3) || (j == -3 && k == 3) || (j == -3 && k == -3))
								{
									continue;
								}

								out->set_id_safe_if_air(x + j, y - 6, z + k, id_brown_mushroom_block);
							}
						}
					}
				}

				break;
			}
		}
	}

	// Plant flowers.

	for (int i = 0; i < out->x_res * out->z_res / 512; i++)
	{
		// Pick a place to plant a patch of flowers.

		int x_ = rand() % out->x_res;
		int z_ = rand() % out->z_res;

		// Choose a type of flower to plant.

		block_id flower;

		if (rand() % 2 == 0)
		{
			flower = id_dandelion;
		}
		else
		{
			flower = id_rose;
		}

		// Plant 16 potential flowers in the patch.

		for (int j = 0; j < 16; j++)
		{
			// Choose a place to plant an individual flower.

			int x = x_ + (rand() % 6) - (rand() % 6);
			int z = z_ + (rand() % 6) - (rand() % 6);

			if (!out->in_bounds(x, 0, z))
			{
				// Try again, this spot is out of bounds.

				continue;
			}

			// Find the highest grass block and plant a flower there.

			for (int y = 0; y < out->y_res; y++)
			{
				block_id current_block = out->get_id(x, y, z);

				if (current_block != id_air)
				{
					if (current_block == id_grass)
					{
						out->set_id_safe(x, y - 1, z, flower);
					}

					break;
				}
			}
		}
	}

	// Plant mushrooms.

	for (int i = 0; i < out->x_res * out->z_res / 128; i++)
	{
		// Pick a place to plant a patch of mushrooms.

		int x_ = rand() % out->x_res;
		int y_ = rand() % out->y_res;
		int z_ = rand() % out->z_res;

		// Choose a type of mushroom to plant.

		block_id mushroom;

		if (rand() % 2 == 0)
		{
			mushroom = id_red_mushroom;
		}
		else
		{
			mushroom = id_brown_mushroom;
		}

		// Plant 24 potential mushrooms in the patch.

		for (int j = 0; j < 24; j++)
		{
			// Choose a place to plant an individual mushroom.

			int x = x_ + (rand() % 6) - (rand() % 6);
			int y = y_ + (rand() % 6) - (rand() % 6);	
			int z = z_ + (rand() % 6) - (rand() % 6);

			if (!out->in_bounds(x, y, z) || !out->in_bounds(x, y + 1, z))
			{
				// Try again, this spot is out of bounds.

				continue;
			}

			if (out->get_id(x, y + 1, z) == id_stone || out->get_id(x, y + 1, z) == id_grass || out->get_id(x, y + 1, z) == id_dirt)
			{
				out->set_id_safe_if_air(x, y, z, mushroom);
			}
		}
	}

	// Plant pumpkins and melons.

	for (int i = 0; i < out->x_res * out->z_res / 256; i++)
	{
		// Pick a place to plant a patch of pumpkins or melons.

		int x_ = rand() % out->x_res;
		int z_ = rand() % out->z_res;

		// Choose a type of block to plant.

		block_id block;

		if (rand() % 2 == 0)
		{
			block = id_pumpkin;
		}
		else
		{
			block = id_melon;
		}

		// Plant 8 potential pumpkins or melons in the patch.

		for (int j = 0; j < 8; j++)
		{
			// Choose a place to plant an individual pumpkin or melon.

			int x = x_ + (rand() % 4) - (rand() % 4);
			int z = z_ + (rand() % 4) - (rand() % 4);

			if (!out->in_bounds(x, 0, z))
			{
				// Try again, this spot is out of bounds.

				continue;
			}

			// Find the highest grass block and plant a pumpkin or melon 
			// there.

			for (int y = 0; y < out->y_res; y++)
			{
				block_id current_block = out->get_id(x, y, z);

				if (current_block != id_air)
				{
					if (current_block == id_grass)
					{
						out->set_id_safe(x, y - 1, z, block);
					}

					break;
				}
			}
		}
	}

	// Add bedrock. This will fill the bottom layer with 100% bedrock, and the
	// second from bottom layer with ~50% bedrock.

	for (int x = 0; x < out->x_res; x++)
	for (int z = 0; z < out->z_res; z++)
	{
		out->set_id(x, out->y_res - 1, z, id_bedrock);

		if (rand() % 2 == 0)
		{
			out->set_id(x, out->y_res - 2, z, id_bedrock);
		}
	}

	// Propagate skylight.

	propagate_skylight(out);
}