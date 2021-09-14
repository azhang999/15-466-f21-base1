#include "PPU466.hpp"
#include "Mode.hpp"
#include "load_save_png.hpp"
#include "data_path.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <unordered_map>
#include <math.h>
#include <tuple>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

    Button prev_left, prev_right, prev_down, prev_up;

    struct Miner {
        std::unordered_map <uint32_t, std::array< uint8_t, 8 >> palette_tile_upperleft;
        std::unordered_map <uint32_t, std::array< uint8_t, 8 >> palette_tile_upperright;
        std::unordered_map <uint32_t, std::array< uint8_t, 8 >> palette_tile_bottomleft;
        std::unordered_map <uint32_t, std::array< uint8_t, 8 >> palette_tile_bottomright;
    };

    struct Slime {
        std::unordered_map <uint32_t, std::array< uint8_t, 8 >> palette_tile_upperleft;
        std::unordered_map <uint32_t, std::array< uint8_t, 8 >> palette_tile_upperright;
        std::unordered_map <uint32_t, std::array< uint8_t, 8 >> palette_tile_bottomleft;
        std::unordered_map <uint32_t, std::array< uint8_t, 8 >> palette_tile_bottomright;
    };

    struct Fire {
        std::unordered_map <uint32_t, std::array< uint8_t, 8 >> palette_tile_upperleft;
        std::unordered_map <uint32_t, std::array< uint8_t, 8 >> palette_tile_upperright;
        std::unordered_map <uint32_t, std::array< uint8_t, 8 >> palette_tile_bottomleft;
        std::unordered_map <uint32_t, std::array< uint8_t, 8 >> palette_tile_bottomright;
    };

    struct Rock {
        std::unordered_map <uint32_t, std::array< uint8_t, 8 >> palette_tile_upperleft;
        std::unordered_map <uint32_t, std::array< uint8_t, 8 >> palette_tile_upperright;
        std::unordered_map <uint32_t, std::array< uint8_t, 8 >> palette_tile_bottomleft;
        std::unordered_map <uint32_t, std::array< uint8_t, 8 >> palette_tile_bottomright;
    };

    std::unordered_map <uint32_t, uint32_t> palette_idx_map;
    int sprite_idx = 0;
    int palette_idx = 2;
    int tile_idx = 5;

    Miner miner;
    Slime slime;
    Fire fire;
    Rock rock;
    // glm::vec2 rock_pos = glm::vec2(50, 50);
    glm::vec2 rock_w_h = glm::vec2(16, 16);
    glm::vec2 rock_v = glm::vec2(0.0f);

    glm::vec2 player_w_h = glm::vec2(16, 16);
    
    glm::vec2 slime_w_h = glm::vec2(16, 16);

    std::vector<std::tuple<glm::vec2, glm::vec2>> rock_list = {std::make_tuple(glm::vec2(0.f), glm::vec2(0.f))};
    float rock_spawn_timer = 8.0f;
    float rock_spawn_time = 0.0f;

    std::vector<glm::vec2> slime_list = {glm::vec2(0.f)};
    float slime_spawn_timer = 5.0f;
    float slime_spawn_time = 0.0f;

	//some weird background animation:
	float background_fade = 0.0f;

	//player position:
	glm::vec2 player_at = glm::vec2(0.0f);

	//----- drawing handled by PPU466 -----

	PPU466 ppu;
};
