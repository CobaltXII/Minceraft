/*

Minceraft

By CobaltXII

*/

#include "main.hpp"

// Format string.
char* formatString(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	size_t bufsz = vsnprintf(NULL, 0, fmt, args) + 1;
	va_end(args);
	char* buf = (char*)malloc(bufsz);
	if (!buf) {
		return NULL;
	}
	va_start(args, fmt);
	vsnprintf(buf, bufsz, fmt, args);
	va_end(args);
	return buf;
}

// Print usage information.

void print_usage(char** argv)
{
	std::cout << "Usage: " << argv[0] << " ..." << std::endl;

	std::cout << std::endl;

	std::cout << "    -s <path-to-level> [x_res y_res z_res]" << std::endl;

	std::cout << std::endl;

	std::cout << "        Play singleplayer. If a world save exists at <path-to-level>, the       " << std::endl;
	std::cout << "        world is loaded and the given dimensions (if any) are ignored. If not,  " << std::endl;
	std::cout << "        a new world is generated with the given dimensions and saved to         " << std::endl;
	std::cout << "        <path-to-level>.                                                        " << std::endl;

	std::cout << std::endl;

	std::cout << "    -q [x_res y_res z_res]" << std::endl;

	std::cout << std::endl;

	std::cout << "        Play quickplay. A new world is generated with the given dimensions (if  " << std::endl;
	std::cout << "        any) and is stored in memory for the duration of the gameplay.          " << std::endl;

	exit(16);
}

// The entry point.

int main(int argc, char** argv)
{
	// Print usage information if the command line was not used correctly. 
	// Otherwise, parse the gamemode.

	int gamemode;

	if (argc == 1)
	{
		// An argc of 1 means that only the executable path was passed from 
		// the command line.

		print_usage(argv);
	}
	else if (std::string(argv[1]) != "-s" && std::string(argv[1]) != "-q")
	{
		// The only allowed arguments are -s and -q.

		print_usage(argv);
	}
	else if (std::string(argv[1]) == "-s")
	{
		// Singleplayer mode.

		gamemode = 1;
	}
	else if (std::string(argv[1]) == "-q")
	{
		// Quickplay mode.

		gamemode = 2;
	}

	// Initialize SDL.

	if (SDL_Init(SDL_INIT_EVERYTHING))
	{
		std::cout << "Could not initialize SDL." << std::endl;

		exit(1);
	}

	// The dimensions of the SDL_Window*.

	int sdl_x_res = 960;
	int sdl_y_res = 540;

	// Create the SDL_Window*.

	SDL_Window* sdl_window = SDL_CreateWindow
	(
		"Minceraft 0.5.08",

		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,

		sdl_x_res,
		sdl_y_res,

		SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL
	);

	// Make sure the SDL_Window* was created successfully.

	if (!sdl_window)
	{
		std::cout << "Could not create the SDL_Window*." << std::endl;

		exit(2);
	}

	// Request OpenGL 3.3 (core profile).

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// Request double-buffering.

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// Request a 24-bit depth buffer.

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	// Enable vertical retrace synchronization.

	SDL_GL_SetSwapInterval(1);

	// Create the SDL_GLContext.

	SDL_GLContext gl_context = SDL_GL_CreateContext(sdl_window);

	if (!gl_context)
	{
		std::cout << "Could not create the SDL_GLContext." << std::endl;

		exit(3);
	}

	// Load all OpenGL 3.3 (core profile) functions using GLAD.

	if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) 
	{
        std::cout << "Could not load OpenGL functions using GLAD." << std::endl;

        exit(4);
    }

    // Make sure the OpenGL version that was provided is greater than or equal
    // to OpenGL 3.3.

    if (GLVersion.major * 10 + GLVersion.minor < 33)
    {
    	std::cout << "Could not load OpenGL 3.3 using GLAD. Received OpenGL " << GLVersion.major << "." << GLVersion.minor << " instead." << std::endl;

        exit(5);
    }

    // Set up depth clamping.

    glEnable(GL_DEPTH_CLAMP);

    // Set up backface culling.

    glCullFace(GL_BACK);

	glFrontFace(GL_CW);

    // Load the block texture array.

    GLuint block_texture_array = load_block_texture_array();

    // Load all the GUI textures.
    gui_texture gui_buttons = 			load_gui("buttons");
    gui_texture gui_crosshair =			load_gui("crosshair");
    gui_texture gui_dark =				load_gui("dark");
    gui_texture gui_font = 				load_gui("font");
    gui_texture gui_hightlight = 		load_gui("highlight");
    gui_texture gui_hotbar = 			load_gui("hotbar");
    gui_texture gui_hotbar_selection = 	load_gui("hotbar_selection");
    gui_texture gui_inventory = 		load_gui("inventory");
    gui_texture gui_inventory_slider = 	load_gui("inventory_slider");
    gui_texture gui_logo = 				load_gui("logo");
    gui_texture gui_tooltip = 			load_gui("tooltip");

    // Initialize the GUI drivers.
    gui_init();
    gui2_init();
    gui3_init();

    // Load the block face_info* array.

    load_block_face_info_array();

    // Load the block shader program.

    GLuint block_shader_program = load_program("../glsl/block_vertex.glsl", "../glsl/block_fragment.glsl");

    // Load the quad shader programs.
    GLuint quad_shader_program = load_program("../glsl/quad_vertex.glsl", "../glsl/quad_fragment.glsl");
    GLuint item_shader_program = load_program("../glsl/item_vertex.glsl", "../glsl/item_fragment.glsl");
    GLuint text_shader_program = load_program("../glsl/text_vertex.glsl", "../glsl/text_fragment.glsl");

    // Create an empty pointer to a world*.

    world* the_world = nullptr;

    // Define variables to hold the looking direction of the player.

    float rot_x_deg = 0.0f;
    float rot_y_deg = 0.0f;

    // Define variables to hold the position and velocity of the player.

    float player_x = 0.0f;
    float player_y = 0.0f;
    float player_z = 0.0f;

    float player_vx = 0.0f;
    float player_vy = 0.0f;
    float player_vz = 0.0f; 

    // Define the player's inventory.

    block_id player_inventory[9] =
    {
    	id_crafting_table,

    	id_furnace,

    	id_potatoes_0,

    	id_fire,

    	id_oak_sapling,

    	id_birch_sapling,

    	id_stone_slab,

    	id_cobblestone,

    	id_wet_farmland
    };

    // Define the player's selected block.

    int player_selection = 0;

    // Define the player's acceleration coefficient.

	float acceleration = 0.0176f;

	// Define the player's friction coefficient.

	float friction = 0.8f;

    // When the block_timer is 0, a block may be destroyed or placed.

    int block_timer = 0;

    // Parse the command line arguments based on the gamemode.

    std::string path_to_level;

    if (gamemode == 1)
	{
		// Singleplayer mode.

		unsigned int x_res;
		unsigned int y_res;
		unsigned int z_res;

		if (argc == 3)
		{
			// Singleplayer mode with a default world.

			x_res = 256;
			y_res = 256;
			z_res = 256;
		}
		else if (argc == 6)
		{
			// Singleplayer mode with a custom world.

			x_res = std::stoi(std::string(argv[3]));
			y_res = std::stoi(std::string(argv[4]));
			z_res = std::stoi(std::string(argv[5]));

			// Make sure that x_res, y_res and z_res are multiples of 16.

			if (x_res % 16 != 0 || y_res % 16 != 0 || z_res % 16 != 0)
			{
				std::cout << "x_res, y_res and z_res must be multiples of 16." << std::endl;

				exit(17);
			}
		}
		else
		{
			// Unknown usage of singleplayer mode.

			print_usage(argv);
		}

		// Parse the <path-to-level> argument.

		path_to_level = std::string(argv[2]);

		// Check if a world at path_to_level exists.

		if (std::ifstream(path_to_level).good())
    	{
    		// The world exists, so load it.

    		load_world_from_file
	    	(
	    		the_world, 

	    		player_x,
	    		player_y,
	    		player_z,

	    		path_to_level
	    	);
    	}
    	else
    	{
    		// The world does not exist, so generate a new world using the 
    		// dimensions stored in *_res.

			the_world = allocate_world(x_res, y_res, z_res);

			// Generate a new world using the current time as the seed.

	    	generate_world(the_world, time(NULL));

	    	// Spawn the player at the top center.

	    	player_x = float(the_world->x_res) / 2.0f;

	    	player_y = 0.0f;

	    	player_z = float(the_world->z_res) / 2.0f;

	    	// Save the world to path_to_level.

	    	save_world_to_file
			(
				the_world, 

				player_x,
				player_y,
				player_z,

				path_to_level
			);
    	}
	}
	else if (gamemode == 2)
	{
		// Quickplay mode.

		unsigned int x_res;
		unsigned int y_res;
		unsigned int z_res;

		if (argc == 2)
		{
			// Quickplay mode with a default world.

			x_res = 128;
			y_res = 128;
			z_res = 128;
		}
		else if (argc == 5)
		{
			// Quickplay mode with a custom world.

			x_res = std::stoi(std::string(argv[2]));
			y_res = std::stoi(std::string(argv[3]));
			z_res = std::stoi(std::string(argv[4]));

			// Make sure that x_res, y_res and z_res are multiples of 16.

			if (x_res % 16 != 0 || y_res % 16 != 0 || z_res % 16 != 0)
			{
				std::cout << "x_res, y_res and z_res must be multiples of 16." << std::endl;

				exit(17);
			}
		}
		else
		{
			// Unknown usage of quickplay mode.

			print_usage(argv);
		}

		// Allocate an empty world of the given size.

		the_world = allocate_world(x_res, y_res, z_res);

		// Generate a new world using the current time as the seed.

    	generate_world(the_world, time(NULL));

    	// Spawn the player at the top center.

    	player_x = float(the_world->x_res) / 2.0f;

    	player_y = 0.0f;

    	player_z = float(the_world->z_res) / 2.0f;
	}

    // Allocate a new accessor* from the_world.

    accessor* the_accessor = allocate_accessor(the_world);

    // Define the reach distance.

    float reach_distance = 8.0f;

    // Define the maximum chunk updates per frame.

    unsigned int max_chunk_updates = 16;

    // Create variables to store the position of the mouse pointer, the state 
    // of the mouse buttons, and the relative mouse mode.

    int sdl_mouse_x = 0;
    int sdl_mouse_y = 0;

    bool sdl_mouse_l = false;
    bool sdl_mouse_r = false;

    bool sdl_mouse_relative = false;

    // The sdl_iteration counter is incremented every frame.

    unsigned long long sdl_iteration = 0;

    // Enter the main loop.

    bool sdl_running = true;

    // Raw settings.
    float option_music_raw = 0.5f;
    float option_sound_raw = 0.5f;
    bool option_invert_mouse = false;
    float option_sensitivity_raw = 0.5f;
    float option_render_distance_raw = 1.0f;
   	bool option_view_bobbing = true;
   	float option_fov_raw = 1.0f;

    // State machine.
    bool is_inventory_open = false;
    int inventory_scroll = 0;
    int inventory_scroll_max = 9;
    float lock_deg_x = 0.0f;
    float lock_deg_y = 0.0f;
    int lock_mouse_x = 0;
    int lock_mouse_y = 0;
    bool has_selected_item = false;
    block_id selected_item = id_null;
    bool is_options_open = false;
    enum {
    	OPT_PAUSE,
    	OPT_OPTIONS
    } option_screen;

    while (sdl_running)
    {
    	float option_music = option_music_raw;
    	float option_sound = option_sound_raw;
    	float option_sensitivity = option_sensitivity_raw * 1.9f + 0.1f;
    	int option_render_distance = int(option_render_distance_raw * 31.0f + 1.0f);
    	float option_fov = option_fov_raw * 80.0f + 30.0f;
    	float view_distance = option_render_distance * 16.0f;

    	bool sdl_mouse_l_old = sdl_mouse_l;
    	bool sdl_mouse_r_old = sdl_mouse_r;

    	// Remember the time at the start of the frame. At the end of the 
    	// frame, this timestamp will be used to cap the framerate.

    	auto frame_start_time = std::chrono::high_resolution_clock::now();

    	// Get the size of the sdl_window*, in case it was resized.

    	SDL_GetWindowSize(sdl_window, &sdl_x_res, &sdl_y_res);

    	// Poll events.

    	SDL_Event e;

    	while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				// The application was quit.

				sdl_running = false;
			}
			else if (e.type == SDL_MOUSEMOTION)
			{
				// The mouse moved.

				if (sdl_mouse_relative && !is_inventory_open && !is_options_open)
				{
					sdl_mouse_x += e.motion.xrel;
					sdl_mouse_y += e.motion.yrel;

					if (sdl_mouse_y > sdl_y_res - 1)
					{
						sdl_mouse_y = sdl_y_res - 1;
					}
					else if (sdl_mouse_y < 0)
					{
						sdl_mouse_y = 0;
					}
				}
				else
				{
					sdl_mouse_x = e.motion.x;
					sdl_mouse_y = e.motion.y;
				}
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN)
			{
				// A mouse button was pressed.

				if (e.button.button == SDL_BUTTON_LEFT)
				{
					sdl_mouse_l = true;
				}
				else if (e.button.button == SDL_BUTTON_RIGHT)
				{
					sdl_mouse_r = true;
				}
			}
			else if (e.type == SDL_MOUSEBUTTONUP)
			{
				// A mouse button was released.

				if (e.button.button == SDL_BUTTON_LEFT)
				{
					sdl_mouse_l = false;
				}
				else if (e.button.button == SDL_BUTTON_RIGHT)
				{
					sdl_mouse_r = false;
				}
			}
			else if (e.type == SDL_KEYDOWN)
			{
				// A key was pressed.

				SDL_Keycode key = e.key.keysym.sym;

				if (key == SDLK_ESCAPE)
				{
					if (sdl_mouse_relative)
					{
						SDL_SetRelativeMouseMode(SDL_FALSE);

						sdl_mouse_relative = SDL_GetRelativeMouseMode();
					}
					else
					{
						sdl_running = false;
					}
				}
				else if (key >= SDLK_1 && key <= SDLK_9) {
					player_selection = key - SDLK_1;
				} else if (key == SDLK_e) {
					if (!is_options_open) {
						is_inventory_open = !is_inventory_open;
						if (is_inventory_open) {
							lock_deg_x = rot_x_deg;
							lock_deg_y = rot_y_deg;
							lock_mouse_x = sdl_mouse_x;
							lock_mouse_y = sdl_mouse_y;
							if (sdl_mouse_relative) {
								SDL_SetRelativeMouseMode(SDL_FALSE);
							}
							SDL_WarpMouseInWindow(sdl_window, sdl_x_res / 2, sdl_y_res / 2);
							sdl_mouse_x = sdl_x_res / 2;
							sdl_mouse_y = sdl_y_res / 2;
						} else {
							sdl_mouse_x = lock_mouse_x;
							sdl_mouse_y = lock_mouse_y;
							if (sdl_mouse_relative) {
								SDL_SetRelativeMouseMode(SDL_TRUE);
							}
							has_selected_item = false;
						}
					}
				} else if (key == SDLK_o) {
					if (!is_inventory_open) {
						is_options_open = !is_options_open;
						if (is_options_open) {
							lock_deg_x = rot_x_deg;
							lock_deg_y = rot_y_deg;
							lock_mouse_x = sdl_mouse_x;
							lock_mouse_y = sdl_mouse_y;
							if (sdl_mouse_relative) {
								SDL_SetRelativeMouseMode(SDL_FALSE);
							}
							SDL_WarpMouseInWindow(sdl_window, sdl_x_res / 2, sdl_y_res / 2);
							sdl_mouse_x = sdl_x_res / 2;
							sdl_mouse_y = sdl_y_res / 2;
							option_screen = OPT_PAUSE;
						} else {
							sdl_mouse_x = lock_mouse_x;
							sdl_mouse_y = lock_mouse_y;
							if (sdl_mouse_relative) {
								SDL_SetRelativeMouseMode(SDL_TRUE);
							}
						}
					}
				}
			} else if (e.type == SDL_MOUSEWHEEL) {
				if (is_inventory_open) {
					if (e.wheel.y > 0) {
						inventory_scroll++;
					} else if (e.wheel.y < 0) {
						inventory_scroll--;
					}
					if (inventory_scroll < 0)
						inventory_scroll = 0;
					else if (inventory_scroll > inventory_scroll_max)
						inventory_scroll = inventory_scroll_max;
				} else {
					if (e.wheel.y > 0) {
						player_selection = (player_selection + 1) % 9;
					} else if (e.wheel.y < 0) {
						player_selection = (player_selection + 8) % 9;
					}
				}
			}
		}

		bool sdl_mouse_l_pressed = sdl_mouse_l && !sdl_mouse_l_old;
		bool sdl_mouse_r_pressed = sdl_mouse_r && !sdl_mouse_r_old;

		// Enable relative mouse mode when the left mouse button is down and
		// relative mouse mode is currently off.

		if (sdl_mouse_relative == false && sdl_mouse_l == true && !is_inventory_open && !is_options_open)
		{
			SDL_SetRelativeMouseMode(SDL_TRUE);

			sdl_mouse_relative = SDL_GetRelativeMouseMode();
		}

		// Calculate the looking direction of the camera.

		float rot_x_deg_want = (float(sdl_mouse_y) - (float(sdl_y_res) / 2.0f)) / float(sdl_y_res) * 180.0f;
		float rot_y_deg_want = (float(sdl_mouse_x) - (float(sdl_x_res) / 2.0f)) / float(sdl_x_res) * 360.0f;

		float camera_smoothing = 1.0f;

		rot_x_deg += (rot_x_deg_want - rot_x_deg) / camera_smoothing;
		rot_y_deg += (rot_y_deg_want - rot_y_deg) / camera_smoothing;

		if (is_inventory_open || is_options_open) {
			rot_x_deg = lock_deg_x;
			rot_y_deg = lock_deg_y;
		}

		// Get the keyboard state.

		const Uint8* keys = SDL_GetKeyboardState(NULL);

		// Boost when shift is pressed.
		float speed_multiplier = 1.0f;
		if (keys[SDL_SCANCODE_LSHIFT]) {
			speed_multiplier = 5.0f;
		}

		// Handle player forward movement (walking).

		if (keys[SDL_SCANCODE_W])
		{
			player_vx -= sin(glm::radians(-rot_y_deg)) * acceleration * speed_multiplier;
			player_vz -= cos(glm::radians(-rot_y_deg)) * acceleration * speed_multiplier;
		}
		else if (keys[SDL_SCANCODE_S])
		{
			player_vx += sin(glm::radians(-rot_y_deg)) * acceleration * speed_multiplier;
			player_vz += cos(glm::radians(-rot_y_deg)) * acceleration * speed_multiplier;
		}

		// Handle player perpendicular movement (strafing).

		if (keys[SDL_SCANCODE_A])
		{
			player_vx -= sin(glm::radians(-rot_y_deg + 90.0f)) * acceleration * speed_multiplier;
			player_vz -= cos(glm::radians(-rot_y_deg + 90.0f)) * acceleration * speed_multiplier;
		}
		else if (keys[SDL_SCANCODE_D])
		{
			player_vx += sin(glm::radians(-rot_y_deg + 90.0f)) * acceleration * speed_multiplier;
			player_vz += cos(glm::radians(-rot_y_deg + 90.0f)) * acceleration * speed_multiplier;
		}

		// Create a list of hitboxes of nearby blocks.

		std::vector<hitbox> near_hitboxes;

		int b_res = 3;

		for (int x = -b_res; x <= b_res; x++)
		for (int y = -b_res; y <= b_res; y++)
		for (int z = -b_res; z <= b_res; z++)
		{
			block_id current_block = the_world->get_id_safe(player_x + x, player_y + y, player_z + z);

			if (is_not_permeable_mob(current_block))
			{
				if (is_slab(current_block))
				{
					near_hitboxes.push_back(hitbox(floor(player_x + x), floor(player_y + y) + 0.5f, floor(player_z + z), 1.0f, 0.5f, 1.0f));
				}
				else
				{
					near_hitboxes.push_back(hitbox(floor(player_x + x), floor(player_y + y), floor(player_z + z), 1.0f, 1.0f, 1.0f));
				}
			}
		}

		// Define the player's hitbox.

		hitbox player_hitbox = hitbox(player_x, player_y, player_z, 0.6f, 1.8f, 0.6f);

		// Do collision detection and response.

		collision_data player_collision = do_collision_detection_and_response(player_hitbox, near_hitboxes, player_vx, player_vy, player_vz);

		// Update the player's position.

		player_x = player_hitbox.x;
		player_y = player_hitbox.y;
		player_z = player_hitbox.z;

		// Handle player upwards movement (jumping).

		if (keys[SDL_SCANCODE_SPACE])
		{
			if (player_collision.collision_y == -1)
			{
				player_vy -= 0.1536f * speed_multiplier;
			}
		}

		// Multiply the player's velocity by the player's friction constant.

		player_vx *= friction;
		player_vz *= friction;

		player_vy += 0.008f;

		// Interact with the world.

		if ((sdl_mouse_l || sdl_mouse_r) && block_timer == 0 && !is_inventory_open && !is_options_open && sdl_mouse_relative)
		{
			// The ray begins at the player's 'eye'.

			float px = player_x + player_hitbox.xr / 2.0f;

			float py = player_y + 0.2f;

			float pz = player_z + player_hitbox.zr / 2.0f;

			// Find the direction of the ray.

			float ix = -sin(glm::radians(-rot_y_deg));
			float iy = -tan(glm::radians(-rot_x_deg));
			float iz = -cos(glm::radians(-rot_y_deg));

			// Find the length of the ray.

			float i_len = sqrt(ix * ix + iy * iy + iz * iz);

			// Store the total distance travelled by the ray.

			float total_distance = 0.0f;

			// Raymarch the ray.

			while (true)
			{
				// Find the current block's block_id.

				block_id current_block = the_world->get_id_safe(px, py, pz);

				// Check if the block can be destroyed or built on.

				if (is_not_permeable_ray(current_block))
				{
					// Check for slab non-collisions.

					if (is_slab(current_block) && py < floor(py) + 0.5f)
					{
						goto increment_ray;
					}

					if (sdl_mouse_l)
					{
						// The block_id to be placed.

						block_id place_id = player_inventory[player_selection];

						// Check if the current block is a slab.

						if (is_slab(current_block) && place_id == current_block)
						{
							// Change the current block into it's double slab 
							// form, if the player does not intersect with the
							// new double slab.

							if (!hitbox_intersect(player_hitbox, hitbox(floor(px), floor(py), floor(pz), 1.0f, 1.0f, 1.0f)))
							{
								the_accessor->set_id_safe(px, py, pz, slab_to_double_slab(current_block));

								block_timer = 10;

								break;
							}
						}
						else
						{
							// Place a block.

							px -= ix * 0.001f;
							py -= iy * 0.001f;
							pz -= iz * 0.001f;	

							// Can't place blocks inside other blocks.

							if (is_not_permeable_ray(the_world->get_id_safe(px, py, pz)))
							{
								break;
							}
							else if 
							(
								is_slab(place_id) ||

								is_crop(place_id) ||

								is_cross(place_id) ||

								is_fire(place_id)
							)
							{
								if (the_world->get_id_safe(px, py, pz) == id_water)
								{
									// Can't place slabs, crops, crosses or
									// fire in id_water.

									break;
								}
							}

							// Find the hitbox of the block that is going to
							// be placed.

							hitbox new_block;

							if (is_slab(place_id))
							{
								new_block = hitbox(floor(px), floor(py) + 0.5, floor(pz), 1.0f, 0.5f, 1.0f);
							}
							else
							{
								new_block = hitbox(floor(px), floor(py), floor(pz), 1.0f, 1.0f, 1.0f);
							}

							// Check if the hitbox of the block that is going
							// to be placed does not intersect with the 
							// player.

							if (!hitbox_intersect(player_hitbox, new_block))
							{
								// The player is not inside the block that is 
								// going to be placed, so place the block.

								if (place_id == id_dandelion || place_id == id_rose)
								{
									// Can only place id_dandelion and id_rose
									// on id_grass or id_dirt.

									block_id flower_below = the_world->get_id_safe(px, py + 1, pz);

									if (flower_below == id_grass || flower_below == id_dirt)
									{
										the_accessor->set_id_safe(px, py, pz, place_id);

										block_timer = 10;
									}
								}
								else if (place_id == id_oak_sapling || place_id == id_birch_sapling)
								{
									// Can only place id_oak_sapling and 
									// id_birch_sapling on id_grass or 
									// id_dirt.

									block_id sapling_below = the_world->get_id_safe(px, py + 1, pz);

									if (sapling_below == id_grass || sapling_below == id_dirt)
									{
										the_accessor->set_id_safe(px, py, pz, place_id);

										block_timer = 10;

										// Trees must grow!

										if (place_id == id_oak_sapling)
										{
											the_world->growing_plants.push_back(growing_plant(px, py, pz, id_oak_sapling, growing_oak_sapling, growth_tree()));
										}
										else if (place_id == id_birch_sapling)
										{
											the_world->growing_plants.push_back(growing_plant(px, py, pz, id_birch_sapling, growing_birch_sapling, growth_tree()));
										}
									}
								}
								else if (place_id == id_reeds)
								{
									// Can only place id_reeds on id_sand or
									// id_reeds.

									block_id reeds_below = the_world->get_id_safe(px, py + 1, pz);

									if (reeds_below == id_sand || reeds_below == id_reeds)
									{
										the_accessor->set_id_safe(px, py, pz, place_id);

										block_timer = 10;
									}
								}
								else if (is_crop(place_id))
								{
									// Crops can only be placed on farmland.

									block_id crops_below = the_world->get_id_safe(px, py + 1, pz);

									if (crops_below == id_dry_farmland || crops_below == id_wet_farmland)
									{
										the_accessor->set_id_safe(px, py, pz, place_id);

										block_timer = 10;

										// Plants must grow!

										if (place_id == id_wheat_0)
										{
											the_world->growing_plants.push_back(growing_plant(px, py, pz, id_wheat_0, growing_wheat, growth_8()));
										}
										else if (place_id == id_carrots_0)
										{
											the_world->growing_plants.push_back(growing_plant(px, py, pz, id_carrots_0, growing_carrots, growth_4()));
										}
										else if (place_id == id_potatoes_0)
										{
											the_world->growing_plants.push_back(growing_plant(px, py, pz, id_potatoes_0, growing_potatoes, growth_4()));
										}
										else if (place_id == id_beetroots_0)
										{
											the_world->growing_plants.push_back(growing_plant(px, py, pz, id_beetroots_0, growing_beetroots, growth_4()));
										}
									}
								}
								else if (place_id == id_fire)
								{
									// Fire can only be placed on top of solid
									// blocks; that is, blocks that are not
									// slabs, crops, crosses, fire, id_air or
									// id_null.

									block_id fire_below = the_world->get_id_safe(px, py + 1, pz);

									if 
									(
										!is_slab(fire_below) && 

										!is_crop(fire_below) && 

										!is_cross(fire_below) && 

										!is_fire(fire_below) &&

										fire_below != id_air &&

										fire_below != id_null
									)
									{
										the_accessor->set_id_safe(px, py, pz, place_id);

										block_timer = 10;

										// Fire must burn!

										the_world->burning_fires.push_back(burning_fire(px, py, pz, fire_timer()));
									}
								}
								else
								{
									the_accessor->set_id_safe(px, py, pz, place_id);

									block_timer = 10;
								}

								// id_dry_farmland and id_wet_farmland are 
								// reverted to id_dirt when anything other 
								// than crops are placed on top of them.

								block_id farmland_below = the_world->get_id_safe(px, py + 1, pz);

								if ((farmland_below == id_dry_farmland || farmland_below == id_wet_farmland) && !is_crop(place_id))
								{
									the_accessor->set_id_safe(px, py + 1, pz, id_dirt);
								}

								break;
							}
						}
					}
					else
					{
						// Removing blocks.

						the_accessor->set_id_safe(px, py, pz, id_air);

						if 
						(
							the_world->get_id_safe(px + 1, py, pz) == id_water ||
							the_world->get_id_safe(px - 1, py, pz) == id_water ||

							the_world->get_id_safe(px, py, pz + 1) == id_water ||
							the_world->get_id_safe(px, py, pz - 1) == id_water ||

							the_world->get_id_safe(px, py - 1, pz) == id_water
						)
						{
							// Water flooding.

							the_accessor->set_id_safe(px, py, pz, id_water);
						}

						// Flowers, saplings, crops and mushrooms cannot exist
						// if there is nothing below them.

						block_id plant_above = the_world->get_id_safe(px, py - 1, pz);

						if 
						(
							plant_above == id_dandelion || 

							plant_above == id_rose || 

							plant_above == id_red_mushroom || 

							plant_above == id_brown_mushroom || 

							plant_above == id_oak_sapling || 

							plant_above == id_birch_sapling ||

							is_crop(plant_above))
						{
							the_world->set_id_safe(px, py - 1, pz, id_air);
						}

						// Reeds cannot exist if there is nothing below them.

						int reeds_offset = 1;

						while (true)
						{
							block_id reeds_above = the_world->get_id_safe(px, py - reeds_offset, pz);

							if (reeds_above != id_reeds)
							{
								break;
							}
							else
							{
								the_world->set_id_safe(px, py - reeds_offset, pz, id_air);
							}

							reeds_offset++;
						}

						block_timer = 10;

						break;
					}
				}

				increment_ray:

				px += ix * 0.001f;
				py += iy * 0.001f;
				pz += iz * 0.001f;

				total_distance += i_len * 0.001f;

				if (total_distance > reach_distance)
				{
					break;
				}
			}
		}

		// Update all growing plants.

		for (std::vector<growing_plant>::iterator growing_plant_iterator = the_world->growing_plants.begin(); growing_plant_iterator != the_world->growing_plants.end();) 
		{
			growing_plant& the_growing_plant = *growing_plant_iterator;

			if (the_growing_plant.timer == 0)
			{
				// Ready to grow into the next phase (if any).

				block_id plant_id = the_world->get_id_safe
				(
					the_growing_plant.x, 
					the_growing_plant.y, 
					the_growing_plant.z
				);

				if (plant_id != the_growing_plant.should_id)
				{
					the_growing_plant.done = true;
				}
				else if (the_growing_plant.type == growing_wheat)
				{
					if (plant_id < id_wheat_7)
					{
						the_accessor->set_id_safe
						(
							the_growing_plant.x,
							the_growing_plant.y,
							the_growing_plant.z,

							block_id(plant_id + 1)
						);

						the_growing_plant.should_id = block_id(plant_id + 1);

						if (plant_id < id_wheat_6)
						{
							the_growing_plant.timer = growth_8();
						}
					}
					else
					{
						the_growing_plant.done = true;
					}
				}
				else if (the_growing_plant.type == growing_carrots)
				{
					if (plant_id < id_carrots_3)
					{
						the_accessor->set_id_safe
						(
							the_growing_plant.x,
							the_growing_plant.y,
							the_growing_plant.z,

							block_id(plant_id + 1)
						);

						the_growing_plant.should_id = block_id(plant_id + 1);

						if (plant_id < id_carrots_2)
						{
							the_growing_plant.timer = growth_4();
						}
					}
					else
					{
						the_growing_plant.done = true;
					}
				}
				else if (the_growing_plant.type == growing_potatoes)
				{
					if (plant_id < id_potatoes_3)
					{
						the_accessor->set_id_safe
						(
							the_growing_plant.x,
							the_growing_plant.y,
							the_growing_plant.z,

							block_id(plant_id + 1)
						);

						the_growing_plant.should_id = block_id(plant_id + 1);

						if (plant_id < id_potatoes_2)
						{
							the_growing_plant.timer = growth_4();
						}
					}
					else
					{
						the_growing_plant.done = true;
					}
				}
				else if (the_growing_plant.type == growing_beetroots)
				{
					if (plant_id < id_beetroots_3)
					{
						the_accessor->set_id_safe
						(
							the_growing_plant.x,
							the_growing_plant.y,
							the_growing_plant.z,

							block_id(plant_id + 1)
						);

						the_growing_plant.should_id = block_id(plant_id + 1);

						if (plant_id < id_beetroots_2)
						{
							the_growing_plant.timer = growth_4();
						}
					}
					else
					{
						the_growing_plant.done = true;
					}
				}
				else if (the_growing_plant.type == growing_oak_sapling)
				{
					the_accessor->set_id_safe
					(
						the_growing_plant.x,
						the_growing_plant.y,
						the_growing_plant.z,

						id_air
					);

					the_growing_plant.y++;

					plant_tree_accessor
					(
						the_accessor, 

						player_hitbox,

						the_growing_plant.x, 
						the_growing_plant.y, 
						the_growing_plant.z, 

						id_oak_leaves, 

						id_oak_log
					);

					the_growing_plant.done = true;
				}
				else if (the_growing_plant.type == growing_birch_sapling)
				{
					the_accessor->set_id_safe
					(
						the_growing_plant.x,
						the_growing_plant.y,
						the_growing_plant.z,

						id_air
					);

					the_growing_plant.y++;

					plant_tree_accessor
					(
						the_accessor, 

						player_hitbox,

						the_growing_plant.x, 
						the_growing_plant.y, 
						the_growing_plant.z, 

						id_birch_leaves, 

						id_birch_log
					);

					the_growing_plant.done = true;
				}
			}
			else
			{
				the_growing_plant.timer--;
			}

			// Remove growing plants that are done growing.

			if (the_growing_plant.done)
			{
				growing_plant_iterator = the_world->growing_plants.erase(growing_plant_iterator);
			}
			else
			{
				++growing_plant_iterator;
			}
		}

		// Update all burning fires. Keep an extra vector for fire spreading.

		std::vector<burning_fire> new_fires;

		for (std::vector<burning_fire>::iterator burning_fire_iterator = the_world->burning_fires.begin(); burning_fire_iterator != the_world->burning_fires.end();) 
		{
			burning_fire& the_burning_fire = *burning_fire_iterator;

			if (the_burning_fire.timer == 0)
			{
				// Check if the burning fire has already been extinguished.

				block_id fire_id = the_world->get_id_safe
				(
					the_burning_fire.x, 
					the_burning_fire.y, 
					the_burning_fire.z
				);

				if (fire_id != id_fire)
				{
					the_burning_fire.done = true;
				}
				else
				{
					// Extinguish the fire.

					the_accessor->set_id_safe
					(
						the_burning_fire.x, 
						the_burning_fire.y, 
						the_burning_fire.z,

						id_air
					);
				}
			}
			else
			{
				if (rand() % 2048 == 0)
				{
					block_id neighbor_id = the_world->get_id_safe(the_burning_fire.x + 1, the_burning_fire.y, the_burning_fire.z);

					block_id below_id = the_world->get_id_safe(the_burning_fire.x + 1, the_burning_fire.y + 1, the_burning_fire.z);

					if (neighbor_id == id_air && !is_slab(below_id) && !is_crop(below_id) && !is_cross(below_id) && !is_fire(below_id) &&below_id != id_air && below_id != id_null)
					{
						the_accessor->set_id_safe(the_burning_fire.x + 1, the_burning_fire.y, the_burning_fire.z, id_fire);

						new_fires.push_back(burning_fire(the_burning_fire.x + 1, the_burning_fire.y, the_burning_fire.z, fire_timer()));
					}
				}

				if (rand() % 2048 == 0)
				{
					block_id neighbor_id = the_world->get_id_safe(the_burning_fire.x - 1, the_burning_fire.y, the_burning_fire.z);

					block_id below_id = the_world->get_id_safe(the_burning_fire.x - 1, the_burning_fire.y + 1, the_burning_fire.z);

					if (neighbor_id == id_air && !is_slab(below_id) && !is_crop(below_id) && !is_cross(below_id) && !is_fire(below_id) &&below_id != id_air && below_id != id_null)
					{
						the_accessor->set_id_safe(the_burning_fire.x - 1, the_burning_fire.y, the_burning_fire.z, id_fire);

						new_fires.push_back(burning_fire(the_burning_fire.x - 1, the_burning_fire.y, the_burning_fire.z, fire_timer()));
					}
				}

				if (rand() % 2048 == 0)
				{
					block_id neighbor_id = the_world->get_id_safe(the_burning_fire.x, the_burning_fire.y, the_burning_fire.z + 1);

					block_id below_id = the_world->get_id_safe(the_burning_fire.x, the_burning_fire.y + 1, the_burning_fire.z + 1);

					if (neighbor_id == id_air && !is_slab(below_id) && !is_crop(below_id) && !is_cross(below_id) && !is_fire(below_id) &&below_id != id_air && below_id != id_null)
					{
						the_accessor->set_id_safe(the_burning_fire.x, the_burning_fire.y, the_burning_fire.z + 1, id_fire);

						new_fires.push_back(burning_fire(the_burning_fire.x, the_burning_fire.y, the_burning_fire.z + 1, fire_timer()));
					}
				}

				if (rand() % 2048 == 0)
				{
					block_id neighbor_id = the_world->get_id_safe(the_burning_fire.x, the_burning_fire.y, the_burning_fire.z - 1);

					block_id below_id = the_world->get_id_safe(the_burning_fire.x, the_burning_fire.y + 1, the_burning_fire.z - 1);

					if (neighbor_id == id_air && !is_slab(below_id) && !is_crop(below_id) && !is_cross(below_id) && !is_fire(below_id) &&below_id != id_air && below_id != id_null)
					{
						the_accessor->set_id_safe(the_burning_fire.x, the_burning_fire.y, the_burning_fire.z - 1, id_fire);

						new_fires.push_back(burning_fire(the_burning_fire.x, the_burning_fire.y, the_burning_fire.z - 1, fire_timer()));
					}
				}

				the_burning_fire.timer--;
			}

			// Remove burning fires that are done burning.

			if (the_burning_fire.done)
			{
				burning_fire_iterator = the_world->burning_fires.erase(burning_fire_iterator);
			}
			else
			{
				++burning_fire_iterator;
			}
		}

		// Add the new_fires to the_world->burning_fires.

		for (auto& the_burning_fire: new_fires)
		{
			the_world->burning_fires.push_back(the_burning_fire);
		}

		// Update all modified chunks.

		unsigned int chunk_updates = 0;

		for (int i = 0; i < the_accessor->chunk_count; i++)
		{
			chunk*& the_chunk = the_accessor->the_chunks[i];

			if (the_chunk->modified)
			{
				chunk* new_chunk = allocate_chunk
				(
					the_world,

					the_chunk->x,
					the_chunk->y,
					the_chunk->z,

					the_chunk->x_res,
					the_chunk->y_res,
					the_chunk->z_res
				);

				deallocate_chunk(the_chunk);

				the_chunk = new_chunk;

				chunk_updates++;
			}

			if (chunk_updates == max_chunk_updates)
			{
				break;
			}
		}

		// Clear the OpenGL context to the default sky color.

		glClearColor
		(
			186.0f / 255.0f, 
			214.0f / 255.0f, 
			254.0f / 255.0f, 

			1.0f
		);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Enable depth testing.

		glEnable(GL_DEPTH_TEST);

		// Enable backface culling.

		glEnable(GL_CULL_FACE);

		// Bind the block shader program to the current state.

		glUseProgram(block_shader_program);

		// Calculate the aspect ratio.

		float aspect_ratio = (float)sdl_x_res / (float)sdl_y_res;

		// Calculate the projection matrix.

		glm::mat4 matrix_projection = glm::perspective(glm::radians(option_fov), aspect_ratio, 0.128f, 1024.0f);

		// Calculate the view matrix.

		glm::mat4 matrix_view = glm::mat4(1.0f);

		// Calculate the view bobbing factor.

		float view_bobbing = sqrt(player_vx * player_vx + player_vz * player_vz) / 0.0640f;

		if (view_bobbing < 0.0128f)
		{
			view_bobbing = 0.0f;
		}

		if (!option_view_bobbing) {
			view_bobbing = 0.0f;
		}

		// Rotate the view matrix.

		matrix_view = glm::rotate(matrix_view, glm::radians(rot_x_deg + sin(SDL_GetTicks() / 80.0f) * view_bobbing), glm::vec3(1.0f, 0.0f, 0.0f));

		matrix_view = glm::rotate(matrix_view, glm::radians(rot_y_deg), glm::vec3(0.0f, 1.0f, 0.0f));

		// Calculate the eye vector.

		float player_x_res = player_hitbox.xr;
		float player_y_res = player_hitbox.yr;
		float player_z_res = player_hitbox.zr;

		glm::vec3 eye_vector = glm::vec3
		(
			// Centered around the player's position plus the player's X 
			// resolution.

			-player_x - player_x_res / 2.0f,

			// The small offset of 0.2f is used so that the player's 'eye' is
			// not rubbing against the ceiling.

			player_y + 0.2f,

			// Centered around the player's position plus the player's Z 
			// resolution.

			-player_z - player_z_res / 2.0f
		);

		// Generate the model matrix using the eye vector.

		glm::mat4 matrix_model = glm::translate(glm::mat4(1.0f), eye_vector);

		// Pass the matrices to the block_shader_program.

		glUniformMatrix4fv(glGetUniformLocation(block_shader_program, "matrix_projection"), 1, GL_FALSE, &matrix_projection[0][0]);

		glUniformMatrix4fv(glGetUniformLocation(block_shader_program, "matrix_view"), 1, GL_FALSE, &matrix_view[0][0]);

		glUniformMatrix4fv(glGetUniformLocation(block_shader_program, "matrix_model"), 1, GL_FALSE, &matrix_model[0][0]);

		// Pass the fog distance to the block_shader_program.

		glUniform1f(glGetUniformLocation(block_shader_program, "fog_distance"), view_distance * view_distance / 8.0f);

		// Pass the current time (in seconds) to the block_shader_program.

		glUniform1f(glGetUniformLocation(block_shader_program, "time_in_seconds"), SDL_GetTicks() / 1000.0f);

		// Bind the block_texture_array to the current state.

		glBindTexture(GL_TEXTURE_2D_ARRAY, block_texture_array);

		// Render all of the chunks' vertex arrays in the_accessor.

		for (int i = 0; i < the_accessor->chunk_count; i++)
		{
			chunk* the_chunk = the_accessor->the_chunks[i];

			float ccx = the_chunk->x + (the_chunk->x_res / 2);
			float ccy = the_chunk->y + (the_chunk->y_res / 2);
			float ccz = the_chunk->z + (the_chunk->z_res / 2);

			float dx = ccx - player_x;
			float dy = ccy - player_y;
			float dz = ccz - player_z;

			if (dx * dx + dy * dy + dz * dz < view_distance * view_distance)
			{
				render_chunk(the_chunk);
			}
		}

		// Disable writing to the depth buffer.

		glDepthMask(GL_FALSE);

		// Enable alpha blending.

		glEnable(GL_BLEND);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Render all of the chunks' water vertex arrays in the_accessor.

		for (int i = 0; i < the_accessor->chunk_count; i++)
		{
			chunk* the_chunk = the_accessor->the_chunks[i];

			float ccx = the_chunk->x + (the_chunk->x_res / 2);
			float ccy = the_chunk->y + (the_chunk->y_res / 2);
			float ccz = the_chunk->z + (the_chunk->z_res / 2);

			float dx = ccx - player_x;
			float dy = ccy - player_y;
			float dz = ccz - player_z;

			if (dx * dx + dy * dy + dz * dz < view_distance * view_distance)
			{
				render_chunk_water(the_chunk);
			}
		}

		// Enable writing to the depth buffer.

		glDepthMask(GL_TRUE);

		// Disable alpha blending.

		glDisable(GL_BLEND);

		// Unbind the block_texture_array from the current state.

		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

		// Unbind the block shader program from the current state.

		glUseProgram(0);

		// Disable backface culling.

		glDisable(GL_CULL_FACE);

		// Disable depth testing.

		glDisable(GL_DEPTH_TEST);

		// Render GUI. Don't try to understand it!
		{
			glEnable(GL_BLEND);

			float gui_w = sdl_x_res;
			float gui_h = sdl_y_res;
			float gui_scale = std::max(1.0f, std::round(gui_w / (gui_hotbar.w * 3.0f)));

			glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);

			// Crosshair.
			if (!is_inventory_open) {
				glUseProgram(quad_shader_program);
				gui_init_frame(gui_w, gui_h);
				gui(
					gui_w / 2.0f - (gui_crosshair.w * gui_scale) / 2.0f,
					gui_h / 2.0f - (gui_crosshair.h * gui_scale) / 2.0f,
					gui_scale, gui_crosshair
				);
				gui_draw_all(gui_crosshair);
			}

			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Hotbar.
			{
				glUseProgram(quad_shader_program);
				gui_init_frame(gui_w, gui_h);
				gui(
					gui_w / 2.0f - (gui_hotbar.w * gui_scale) / 2.0f,
					gui_h - (gui_hotbar.h * gui_scale),
					gui_scale, gui_hotbar
				);
				gui_draw_all(gui_hotbar);
			}

			// Items.
			{
				glUseProgram(item_shader_program);
				gui2_init_frame(gui_w, gui_h);
				for (int i = 0; i < 9; i++) {
					if (player_inventory[i] != id_air) {
						gui2(
							gui_w / 2.0f - (gui_hotbar.w * gui_scale) / 2.0f + (20.0f * gui_scale) * float(i) + (3.0f * gui_scale),
							gui_h - (gui_hotbar.h * gui_scale) + (3.0f * gui_scale),
							gui_scale, player_inventory[i]
						);
					}
				}
				gui2_draw_all(block_texture_array);
			}

			// Hotbar selection.
			{
				glUseProgram(quad_shader_program);
				gui_init_frame(gui_w, gui_h);
				gui(
					gui_w / 2.0f - (gui_hotbar.w * gui_scale) / 2.0f - gui_scale + (20.0f * gui_scale) * float(player_selection),
					gui_h - (gui_hotbar.h * gui_scale) - gui_scale,
					gui_scale, gui_hotbar_selection
				);
				gui_draw_all(gui_hotbar_selection);
			}

			// Inventory.
			if (is_inventory_open) {
				glUseProgram(quad_shader_program);

				// Darken.
				gui_init_frame(gui_w, gui_h);
				gui(0.0f, 0.0f, gui_w / gui_scale, gui_h / gui_scale, gui_scale);
				gui_draw_all(gui_dark);

				{
					// Container.
					float x = gui_w / 2.0f - (gui_inventory.w * gui_scale) / 2.0f;
					float y = gui_h / 2.0f - (gui_inventory.h * gui_scale) / 2.0f;
					float w = gui_inventory.w * gui_scale;
					float h = gui_inventory.h * gui_scale;
					gui_init_frame(gui_w, gui_h);
					gui(x, y, gui_scale, gui_inventory);
					gui_draw_all(gui_inventory);

					if (sdl_mouse_l_pressed && has_selected_item && !(sdl_mouse_x >= x && sdl_mouse_x <= x + w && sdl_mouse_y >= y && sdl_mouse_y <= y + h)) {
						has_selected_item = false;
					}
				}

				// Slider.
				gui_init_frame(gui_w, gui_h);
				gui(
					gui_w / 2.0f - (gui_inventory.w * gui_scale) / 2.0f + (156.0f * gui_scale),
					gui_h / 2.0f - (gui_inventory.h * gui_scale) / 2.0f + (18.0f * gui_scale) + (std::round(145.0f * float(inventory_scroll) / float(inventory_scroll_max)) * gui_scale),
					gui_scale, gui_inventory_slider
				);
				gui_draw_all(gui_inventory_slider);

				// Items.
				{
					std::string tooltip = "";
					float tooltip_x = 0.0f;
					float tooltip_y = 0.0f;
					float tooltip_item_x = 0.0f;
					float tooltip_item_y = 0.0f;
					float tooltip_spacing = 2.0f;

					glUseProgram(item_shader_program);
					gui2_init_frame(gui_w, gui_h);

					// Creative items.
					for (int i = 0; i < 72; i++) {
						float p = float(i % 8);
						float q = float(i / 8);
						if (i + inventory_scroll * 8 + 1 < int(id_null)) {
							float x = gui_w / 2.0f - (gui_inventory.w * gui_scale) / 2.0f + (8.0f * gui_scale) + (p * 18.0f * gui_scale);
							float y = gui_h / 2.0f - (gui_inventory.h * gui_scale) / 2.0f + (18.0f * gui_scale) + (q * 18.0f * gui_scale);
							float w = 16.0f * gui_scale;
							float h = 16.0f * gui_scale;
							gui2(x, y, gui_scale, block_id(i + inventory_scroll * 8 + 1));
							if (sdl_mouse_x >= x && sdl_mouse_x <= x + w &&
								sdl_mouse_y >= y && sdl_mouse_y <= y + h) {
								tooltip = block_id_to_block_name[i + inventory_scroll * 8 + 1];
								tooltip_x = float(sdl_mouse_x) + tooltip_spacing;
								tooltip_y = float(sdl_mouse_y) + tooltip_spacing;
								tooltip_item_x = x;
								tooltip_item_y = y;
								if (sdl_mouse_l_pressed) {
									has_selected_item = true;
									selected_item = block_id(i + inventory_scroll * 8 + 1);
								}
							}
						}
					}

					// Hotbar items.
					for (int i = 0; i < 9; i++) {
						float x = gui_w / 2.0f - (gui_inventory.w * gui_scale) / 2.0f + (8.0f * gui_scale) + (float(i) * 18.0f * gui_scale);
						float y = gui_h / 2.0f - (gui_inventory.h * gui_scale) / 2.0f + (184.0f * gui_scale);
						float w = 16.0f * gui_scale;
						float h = 16.0f * gui_scale;
						if (player_inventory[i] != id_air) {
							gui2(x, y, gui_scale, player_inventory[i]);
						}
						if (sdl_mouse_x >= x && sdl_mouse_x <= x + w &&
							sdl_mouse_y >= y && sdl_mouse_y <= y + h) {
							if (player_inventory[i] != id_air) {
								tooltip = block_id_to_block_name[player_inventory[i]];
								tooltip_x = float(sdl_mouse_x) + tooltip_spacing;
								tooltip_y = float(sdl_mouse_y) + tooltip_spacing;
								tooltip_item_x = x;
								tooltip_item_y = y;
							}
							if (sdl_mouse_l_pressed) {
								if (has_selected_item) {
									if (player_inventory[i] == id_air) {
										player_inventory[i] = selected_item;
										has_selected_item = false;
									} else {
										std::swap(player_inventory[i], selected_item);
									}
								} else if (player_inventory[i] != id_air) {
									has_selected_item = true;
									selected_item = player_inventory[i];
									player_inventory[i] = id_air;
								}
							}
						}
					}
					gui2_draw_all(block_texture_array);

					// Tooltip.
					if (!tooltip.empty()) {
						glUseProgram(quad_shader_program);

						// Item highlight.
						gui_init_frame(gui_w, gui_h);
						gui(tooltip_item_x, tooltip_item_y, 16.0f, 16.0f, gui_scale);
						gui_draw_all(gui_hightlight);

						if (!has_selected_item) {
							// Tooltip box.
							gui_init_frame(gui_w, gui_h);
							gui(tooltip_x - tooltip_spacing * gui_scale, tooltip_y - tooltip_spacing * gui_scale, gui3_measure(tooltip) + tooltip_spacing * 2.0f, 8.0f + tooltip_spacing * 2.0f, gui_scale);
							gui_draw_all(gui_tooltip);

							// Tooltip.
							glUseProgram(text_shader_program);
							gui3_init_frame(gui_w, gui_h);
							gui3_shadowed_string(tooltip_x, tooltip_y, gui_scale, tooltip);
							gui3_draw_all(gui_font);
						}
					}

					// Selected item.
					if (has_selected_item) {
						float block_offset = 0.0f;

						glUseProgram(item_shader_program);
						gui2_init_frame(gui_w, gui_h);
						gui2(
							float(sdl_mouse_x) + block_offset,
							float(sdl_mouse_y) + block_offset,
							gui_scale, selected_item
						);
						gui2_draw_all(block_texture_array);
					}
				}
			}

			// Options.
			if (is_options_open) {
				glUseProgram(quad_shader_program);

				// Darken.
				gui_init_frame(gui_w, gui_h);
				gui(0.0f, 0.0f, gui_w / gui_scale, gui_h / gui_scale, gui_scale);
				gui_draw_all(gui_dark);

				// Render buttons.
				gui4_init_frame(
					gui_w, gui_h,
					sdl_mouse_x, sdl_mouse_y,
					sdl_mouse_l_pressed, sdl_mouse_r_pressed,
					sdl_mouse_l, sdl_mouse_r
				);

				if (option_screen == OPT_PAUSE) {
					// Buttons.
					float button_small_spacing_px = 5.0f;
					float button_large_spacing_px = 30.0f;

					float button_total_height_px = 0.0f;
					button_total_height_px += GUI_BUTTON_H + button_small_spacing_px;
					button_total_height_px += GUI_BUTTON_H + button_large_spacing_px;
					button_total_height_px += GUI_BUTTON_H;
					float button_offset = -button_total_height_px / 2.0f;

					button_result bresult0 = gui4_button(
						"Back to game",
						gui_w / 2.0f - (gui_buttons.w * gui_scale) / 2.0f,
						gui_h / 2.0f + button_offset * gui_scale,
						GUI_BUTTON_W,
						gui_scale
					);
					button_offset += GUI_BUTTON_H + button_small_spacing_px;
					button_result bresult1 = gui4_button_disabled(
						"Save and quit to title...",
						gui_w / 2.0f - (gui_buttons.w * gui_scale) / 2.0f,
						gui_h / 2.0f + button_offset * gui_scale,
						GUI_BUTTON_W,
						gui_scale
					);
					button_offset += GUI_BUTTON_H + button_large_spacing_px;
					button_result bresult2 = gui4_button(
						"Options...",
						gui_w / 2.0f - (gui_buttons.w * gui_scale) / 2.0f,
						gui_h / 2.0f + button_offset * gui_scale,
						GUI_BUTTON_W,
						gui_scale
					);
					gui4_draw_all(gui_buttons, gui_font, quad_shader_program, text_shader_program);

					// Handle buttons.
					if (button_pressed(bresult0)) {
						is_options_open = false;
						sdl_mouse_x = lock_mouse_x;
						sdl_mouse_y = lock_mouse_y;
						if (sdl_mouse_relative) {
							SDL_SetRelativeMouseMode(SDL_TRUE);
						}

						// Set mouse button states to false because you don't want
						// to interact right after exiting.
						sdl_mouse_l = false;
						sdl_mouse_r = false;
					}
					if (button_pressed(bresult1)) {
						// no-op
					}
					if (button_pressed(bresult2)) {
						// Switch screens.
						option_screen = OPT_OPTIONS;
					}

					// Text.
					glUseProgram(text_shader_program);
					gui3_init_frame(gui_w, gui_h);
					gui3_shadowed_string(gui_w / 2.0f - gui3_measure("Game menu") / 2.0f * gui_scale, gui_h / 2.0f - (button_total_height_px + GUI_BUTTON_H * 4.0f) / 2.0f * gui_scale, gui_scale, "Game menu");
					gui3_draw_all(gui_font);
				} else if (option_screen == OPT_OPTIONS) {
					// Buttons.
					float button_small_spacing_px = 5.0f;
					float button_large_spacing_px = 30.0f;
					float medium_width = GUI_BUTTON_W * 0.8f;

					float button_total_height_px = 0.0f;
					button_total_height_px += GUI_BUTTON_H + button_small_spacing_px;
					button_total_height_px += GUI_BUTTON_H + button_small_spacing_px;
					button_total_height_px += GUI_BUTTON_H + button_small_spacing_px;
					button_total_height_px += GUI_BUTTON_H + button_large_spacing_px;
					button_total_height_px += GUI_BUTTON_H + button_small_spacing_px;
					button_total_height_px += GUI_BUTTON_H;
					float button_offset = -button_total_height_px / 2.0f;

					gui4_slider("Music: " + std::to_string(int(option_music * 100.0f)) + "%", gui_w / 2.0f - medium_width * gui_scale - (button_small_spacing_px * gui_scale) / 2.0f, gui_h / 2.0f + button_offset * gui_scale, medium_width, gui_scale, &option_music_raw);
					gui4_slider("Sound: " + std::to_string(int(option_sound * 100.0f)) + "%", gui_w / 2.0f + (button_small_spacing_px * gui_scale) / 2.0f, gui_h / 2.0f + button_offset * gui_scale, medium_width, gui_scale, &option_sound_raw);
					button_offset += GUI_BUTTON_H + button_small_spacing_px;
					gui4_toggle("Invert mouse", gui_w / 2.0f - medium_width * gui_scale - (button_small_spacing_px * gui_scale) / 2.0f, gui_h / 2.0f + button_offset * gui_scale, medium_width, gui_scale, &option_invert_mouse);
					gui4_slider("Sensitivity: " + std::to_string(int(option_sensitivity * 100.0f)) + "%", gui_w / 2.0f + (button_small_spacing_px * gui_scale) / 2.0f, gui_h / 2.0f + button_offset * gui_scale, medium_width, gui_scale, &option_sensitivity_raw);
					button_offset += GUI_BUTTON_H + button_small_spacing_px;
					gui4_slider("Render distance: " + std::to_string(option_render_distance) + " chunks", gui_w / 2.0f - medium_width * gui_scale - (button_small_spacing_px * gui_scale) / 2.0f, gui_h / 2.0f + button_offset * gui_scale, medium_width, gui_scale, &option_render_distance_raw);
					gui4_toggle("View bobbing", gui_w / 2.0f + (button_small_spacing_px * gui_scale) / 2.0f, gui_h / 2.0f + button_offset * gui_scale, medium_width, gui_scale, &option_view_bobbing);
					button_offset += GUI_BUTTON_H + button_small_spacing_px;
					gui4_slider("FOV: " + std::to_string(int(option_fov)), gui_w / 2.0f - medium_width * gui_scale - (button_small_spacing_px * gui_scale) / 2.0f, gui_h / 2.0f + button_offset * gui_scale, medium_width, gui_scale, &option_fov_raw);
					gui4_button_disabled("Super secret settings", gui_w / 2.0f + (button_small_spacing_px * gui_scale) / 2.0f, gui_h / 2.0f + button_offset * gui_scale, medium_width, gui_scale);

					button_offset += GUI_BUTTON_H + button_large_spacing_px;
					button_result bresult_controls = gui4_button_disabled(
						"Controls",
						gui_w / 2.0f - (gui_buttons.w * gui_scale) / 2.0f,
						gui_h / 2.0f + button_offset * gui_scale,
						GUI_BUTTON_W,
						gui_scale
					);
					button_offset += GUI_BUTTON_H + button_small_spacing_px;
					button_result bresult_done = gui4_button(
						"Done",
						gui_w / 2.0f - (gui_buttons.w * gui_scale) / 2.0f,
						gui_h / 2.0f + button_offset * gui_scale,
						GUI_BUTTON_W,
						gui_scale
					);
					gui4_draw_all(gui_buttons, gui_font, quad_shader_program, text_shader_program);

					if (button_pressed(bresult_done)) {
						option_screen = OPT_PAUSE;
					}

					// Text.
					glUseProgram(text_shader_program);
					gui3_init_frame(gui_w, gui_h);
					gui3_shadowed_string(gui_w / 2.0f - gui3_measure("Game menu") / 2.0f * gui_scale, gui_h / 2.0f - (button_total_height_px + GUI_BUTTON_H * 4.0f) / 2.0f * gui_scale, gui_scale, "Game menu");
					gui3_draw_all(gui_font);
				} 
			}

			glDisable(GL_BLEND);
			glUseProgram(0);
		}

		// Swap the back buffer to the front.

		SDL_GL_SwapWindow(sdl_window);

		// Decrement the block timer, if the block timer is not equal to 0.

		if (block_timer != 0)
		{
			block_timer--;
		}

		// Cap the framerate to 60 Hz. It might be beneficial to regenerate
		// chunks instead of sleeping.

		float frame_elapsed_time = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - frame_start_time).count();

		if (frame_elapsed_time < 1000.0f / 60.0f)
		{
			int frame_sleep_time = round(1000.0f / 60.0f - frame_elapsed_time);

			std::this_thread::sleep_for(std::chrono::milliseconds(frame_sleep_time));
		}

		// Increment the iteration counter. Print the framerate every 60 
		// iterations.

		sdl_iteration++;

		if (sdl_iteration % 60 == 0)
		{
			std::cout << "Running at " << 1000.0f / frame_elapsed_time << " Hz" << std::endl;
		}
    }

    // Save the world to the specified save file, if the gamemode is 1 
    // (singleplayer).

    if (gamemode == 1)
    {
	    save_world_to_file
		(
			the_world, 

			player_x,
			player_y,
			player_z,

			path_to_level
		);
    }

    // Destroy all Minceraft related objects.

    deallocate_world(the_world);

    deallocate_accessor(the_accessor);

    // Destroy all OpenGL related objects.

    glDeleteTextures(1, &block_texture_array);

    glDeleteProgram(block_shader_program);

    SDL_GL_DeleteContext(gl_context);

    // Destroy all SDL related objects.

    SDL_DestroyWindow(sdl_window);

    SDL_Quit();

    // Exit cleanly.

    exit(EXIT_SUCCESS);
}