#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

uint32_t convert_u8vec4_to_key(glm::u8vec4 v) {
    return (v[0] << 24) | (v[1] << 16) | (v[2] << 8) | v[3];
}

glm::u8vec4 convert_key_to_u8vec4(uint32_t v) {
    uint8_t id_0 = (v >> 24) & 0xff;
    uint8_t id_1 = (v >> 16) & 0xff;
    uint8_t id_2 = (v >> 8) & 0xff;
    uint8_t id_3 = v & 0xff;

    return glm::u8vec4(id_0, id_1, id_2, id_3);
}

PlayMode::PlayMode() {
    auto load_quad = [](std::unordered_map <uint32_t, std::array< uint8_t, 8 >> &palette_tile_map, std::vector< glm::u8vec4 > &data, int quartile) {
        if (data.size() != 16*16) {
            printf("Error data is size %lu\n", data.size());
        }
        int i_offset = 0;
        int j_offset = 0;
        switch (quartile) {
            case 1: { j_offset = 8; break; }
            case 2: { i_offset = 8; break; }
            case 3: { i_offset = 8; j_offset = 8; break; } 
            default: break;
        }

        for (int i = i_offset; i < i_offset + 8; ++i) { //get all colors used
            for (int j = j_offset; j < j_offset + 8; ++j) {
                uint32_t color_key = convert_u8vec4_to_key(data[i*16 + j]);
                if ((data[i*16 + j] != glm::u8vec4(0xff, 0xff, 0xff, 0xff)) &&
                    (palette_tile_map.find(color_key) == palette_tile_map.end())) {
                    std::array< uint8_t, 8 > bits;
                    std::fill(bits.begin(), bits.end(), 0);
                    palette_tile_map[color_key] = bits;
                }
            }
        } 

        for (int i = i_offset; i < i_offset + 8; ++i) {
            for (int j = j_offset; j < j_offset + 8; ++j) {
                if (data[i*16 + j] != glm::u8vec4(0xff, 0xff, 0xff, 0xff)) {
                    uint32_t color_key = convert_u8vec4_to_key(data[i*16 + j]);
                    if (palette_tile_map.find(color_key) == palette_tile_map.end()) {
                        printf("Error cannot find color: %x %x %x\n", data[i*16 + j][0], data[i*16 + j][1], data[i*16 + j][2]);
                    }
                    palette_tile_map[color_key][i - i_offset] |= (1 << (j - j_offset));
                }
            }
        }
	};

    auto test_path = data_path("main_forward_left.png");
    std::vector< glm::u8vec4 > data;
    glm::uvec2 size = glm::uvec2(16, 16);
    load_png(test_path, &size, &data, OriginLocation::LowerLeftOrigin);

    load_quad(miner.palette_tile_upperleft, data, 2);
    load_quad(miner.palette_tile_upperright, data, 3);
    load_quad(miner.palette_tile_bottomleft, data, 0);
    load_quad(miner.palette_tile_bottomright, data, 1);

    auto slime_path = data_path("basic_slime_front.png");
    std::vector< glm::u8vec4 > slime_data;
    load_png(slime_path, &size, &slime_data, OriginLocation::LowerLeftOrigin);

    load_quad(slime.palette_tile_upperleft, slime_data, 2);
    load_quad(slime.palette_tile_upperright, slime_data, 3);
    load_quad(slime.palette_tile_bottomleft, slime_data, 0);
    load_quad(slime.palette_tile_bottomright, slime_data, 1);

    auto rock_path = data_path("rock.png");
    std::vector< glm::u8vec4 > rock_data;
    load_png(rock_path, &size, &rock_data, OriginLocation::LowerLeftOrigin);

    load_quad(rock.palette_tile_upperleft, rock_data, 2);
    load_quad(rock.palette_tile_upperright, rock_data, 3);
    load_quad(rock.palette_tile_bottomleft, rock_data, 0);
    load_quad(rock.palette_tile_bottomright, rock_data, 1);

    {
        PPU466::Tile rock_wall_tile;
        rock_wall_tile.bit0 = rock.palette_tile_upperleft.begin()->second;
        ppu.tile_table[1] = rock_wall_tile;
        rock_wall_tile.bit0 = rock.palette_tile_upperright.begin()->second;
        ppu.tile_table[2] = rock_wall_tile;
        rock_wall_tile.bit0 = rock.palette_tile_bottomleft.begin()->second;
        ppu.tile_table[3] = rock_wall_tile;
        rock_wall_tile.bit0 = rock.palette_tile_bottomright.begin()->second;
        ppu.tile_table[4] = rock_wall_tile;

        palette_idx_map[rock.palette_tile_upperleft.begin()->first] = 1;

        ppu.palette_table[1] = {
            glm::u8vec4(0x00, 0x00, 0x00, 0x00),
            convert_key_to_u8vec4(rock.palette_tile_upperleft.begin()->first),
            glm::u8vec4(0x00, 0x00, 0x00, 0x00),
            glm::u8vec4(0x00, 0x00, 0x00, 0x00),
        };

        palette_idx_map[0xffffffff] = 0;
        PPU466::Tile background_tile;
        background_tile.bit0 = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        ppu.tile_table[0] = background_tile;
        ppu.palette_table[0] = {
            glm::u8vec4(0x00, 0x00, 0x00, 0x00),
            glm::u8vec4(0xff, 0xff, 0xff, 0xff),
            glm::u8vec4(0x00, 0x00, 0x00, 0x00),
            glm::u8vec4(0x00, 0x00, 0x00, 0x00),
        };
    }
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
    // jumping - rising phase that continue as long as the jump button is held down - descent phase when the jump button is lifted
    // uses length of time the jump button is held - retroactive (mario jump works)

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
            left.downs = 0;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
            right.downs = 0;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
            up.downs = 0;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
            down.downs = 0;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	constexpr float PlayerSpeed = 60.0f;
	if (left.pressed) player_at.x -= PlayerSpeed * elapsed;
	if (right.pressed) player_at.x += PlayerSpeed * elapsed;
	if (down.pressed) player_at.y -= PlayerSpeed * elapsed;
	if (up.pressed) player_at.y += PlayerSpeed * elapsed;

    player_at.x = std::min(player_at.x, ((float)PPU466::BackgroundWidth/2 - 4)*8);
    player_at.x = std::max(player_at.x, 16.0f);
    player_at.y = std::min(player_at.y, ((float)PPU466::BackgroundHeight/2 - 4)*8);
    player_at.y = std::max(player_at.y, 16.0f);

    // enemy movement
    constexpr float SlimeSpeed = 30.0f;

    slime_spawn_time += elapsed;
    if (slime_spawn_time >= rock_spawn_timer && slime_list.size() < 6) {
        slime_spawn_time = 0.0f;
        auto slime_pos = glm::vec2(rand() % (PPU466::BackgroundWidth/2 - 4)*8 + 16, rand() % (PPU466::BackgroundHeight/2 - 4)*8 + 16);
        slime_list.push_back(slime_pos);
    }

    //spawn rock
    rock_spawn_time += elapsed;
    if (rock_spawn_time >= rock_spawn_timer && rock_list.size() < 8) {
        rock_spawn_time = 0.0f;
        auto rock_pos = glm::vec2(rand() % (PPU466::BackgroundWidth/2 - 4)*8 + 16, rand() % (PPU466::BackgroundHeight/2 - 4)*8 + 16);
        rock_list.push_back(std::make_tuple(rock_pos, glm::vec2(0.f)));
    }

    auto move_rock = [this](glm::vec2 &obj1_pos, glm::vec2 &obj1_w_h, 
                            glm::vec2 &obj2_pos, glm::vec2 &obj2_w_h, 
                            glm::vec2 &obj2_vel) { // based on p0 code

		glm::vec2 min = glm::max(obj1_pos, obj2_pos);
		glm::vec2 max = glm::min(obj1_pos + obj1_w_h, obj2_pos + obj2_w_h);

		//if no overlap, no collision:
		if (min.x > max.x || min.y > max.y) return;

        // case on what buttons are pushed down
        obj2_vel.x += (float)right.downs * 1.0f;
        obj2_vel.x -= (float)left.downs * 1.0f;
        obj2_vel.y += (float)up.downs * 1.0f;
        obj2_vel.y -= (float)down.downs * 1.0f;
	};
    // check rock collisions

    for (auto it = std::next(rock_list.begin()); it != rock_list.end(); it++) { // move rock
        auto rock_p_v = (*it);
        auto rock_pos = std::get<0>(rock_p_v);
        auto rock_v = std::get<1>(rock_p_v);

        move_rock(player_at, player_w_h, rock_pos, rock_w_h, rock_v);
        rock_pos += rock_v;

        *it = std::make_tuple(rock_pos, rock_v);

        if (   rock_pos.x <= 16 
            || rock_pos.y <= 16 
            || rock_pos.x >=  (PPU466::BackgroundWidth/2 - 4)*8 
            || rock_pos.y >= (PPU466::BackgroundHeight/2 - 4)*8) {
            rock_list.erase(it--);
        }
    }

    auto rock_hit_slime = [](glm::vec2 &slime_pos, glm::vec2 &slime_w_h, 
                            glm::vec2 &rock_pos, glm::vec2 &rock_w_h) { // based on p0 code

		glm::vec2 min = glm::max(slime_pos, rock_pos);
		glm::vec2 max = glm::min(slime_pos + slime_w_h, rock_pos + rock_w_h);

		//if no overlap, no collision:
		if (min.x > max.x || min.y > max.y) return false;
        return true;
	};

    for (auto it = std::next(slime_list.begin()); it != slime_list.end(); it++) { // move slimes
        auto slime_pos = (*it);

        glm::vec2 slime_v = player_at - slime_pos;
        float slime_v_len = sqrt(slime_v.x*slime_v.x+slime_v.y*slime_v.y);
        slime_v /= slime_v_len;
        slime_pos += slime_v * SlimeSpeed * elapsed;

        *it = slime_pos;

        // check if hit by stone;
        bool hit = false;
        for (auto it = std::next(rock_list.begin()); it != rock_list.end(); it++) { // move rock
            auto rock_p_v = (*it);
            auto rock_pos = std::get<0>(rock_p_v);
            auto rock_v = std::get<1>(rock_p_v);

            if (rock_v == glm::vec2(0.f)) {
                continue;
            }
            if (rock_hit_slime(slime_pos, slime_w_h, rock_pos, rock_w_h))  {
                hit = true;
                break;
            }
        }

        if (hit) {
            slime_list.erase(it--);
        }
    }
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	ppu.background_color = glm::u8vec4(0x00, 0x00, 0x00, 0xff);
	for (uint32_t y = 0; y < PPU466::BackgroundHeight; y+=2) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; x +=2) {
            if (y == 0 || x == 0 || y == (PPU466::BackgroundHeight/2 - 2) || x == (PPU466::BackgroundWidth/2 - 2)) {
                ppu.background[x+PPU466::BackgroundWidth*y]       = 3 | (1 << 8);
                ppu.background[x+1+PPU466::BackgroundWidth*y]     = 4 | (1 << 8);
                ppu.background[x+PPU466::BackgroundWidth*(y+1)]   = 1 | (1 << 8);
                ppu.background[x+1+PPU466::BackgroundWidth*(y+1)] = 2 | (1 << 8);
            } else {
                ppu.background[x+PPU466::BackgroundWidth*y]       = 0;
                ppu.background[x+1+PPU466::BackgroundWidth*y]     = 0;
                ppu.background[x+PPU466::BackgroundWidth*(y+1)]   = 0;
                ppu.background[x+1+PPU466::BackgroundWidth*(y+1)] = 0;
            }
		}
	}

    ppu.background_position.x = 0;
	ppu.background_position.y = 0;

    auto draw_quad = [this](std::unordered_map <uint32_t, std::array< uint8_t, 8 >> &palette_tile_map, int quartile, glm::vec2 pos) {
        int x_offset = 0;
        int y_offset = 0;

        switch (quartile) {
            case 1: { x_offset = 8; break; }
            case 2: { y_offset = 8; break; }
            case 3: { x_offset = 8; y_offset = 8; break; } 
            default: break;
        }

        for (auto const& it : palette_tile_map) {
            if (sprite_idx >= 64) {
                printf("Reached limit of number of sprites\n");
                break;
            }
            if (palette_idx >= 8) {
                printf("Reached limit of number of palletes\n");
                break;
            }
            if (tile_idx >= 16 * 16) {
                printf("Reached limit of number of tiles\n");
                break;
            }

            auto p_palette = convert_key_to_u8vec4(it.first);
            auto p_tile = it.second;


            ppu.sprites[sprite_idx].x = int32_t(pos.x) + x_offset;
            ppu.sprites[sprite_idx].y = int32_t(pos.y) + y_offset;
            ppu.sprites[sprite_idx].index = tile_idx;

            if (palette_idx_map.find(it.first) == palette_idx_map.end()) {
                ppu.sprites[sprite_idx].attributes = palette_idx;
                ppu.palette_table[palette_idx] = {
                    glm::u8vec4(0x00, 0x00, 0x00, 0x00),
                    p_palette,
                    glm::u8vec4(0x00, 0x00, 0x00, 0x00),
                    glm::u8vec4(0x00, 0x00, 0x00, 0x00),
                };
                palette_idx_map[it.first] = palette_idx;
                palette_idx++;
            } else {
                ppu.sprites[sprite_idx].attributes = palette_idx_map[it.first];
            }

            PPU466::Tile tile;
            tile.bit0 = p_tile;
            ppu.tile_table[tile_idx] = tile;

            sprite_idx++;
            tile_idx++;
        }   	
	};

    draw_quad(miner.palette_tile_upperleft, 2, player_at);
    draw_quad(miner.palette_tile_upperright, 3, player_at);
    draw_quad(miner.palette_tile_bottomleft, 0, player_at);
    draw_quad(miner.palette_tile_bottomright, 1, player_at);

    for (auto it = std::next(slime_list.begin()); it != slime_list.end(); it++) {
        auto slime_pos = (*it);

        draw_quad(slime.palette_tile_upperleft, 2, slime_pos);
        draw_quad(slime.palette_tile_upperright, 3, slime_pos);
        draw_quad(slime.palette_tile_bottomleft, 0, slime_pos);
        draw_quad(slime.palette_tile_bottomright, 1, slime_pos);
    }


    for (auto it = std::next(rock_list.begin()); it != rock_list.end(); it++) {
        auto rock_p_v = (*it);
        auto rock_pos = std::get<0>(rock_p_v);

        draw_quad(rock.palette_tile_upperleft, 2, rock_pos);
        draw_quad(rock.palette_tile_upperright, 3, rock_pos);
        draw_quad(rock.palette_tile_bottomleft, 0, rock_pos);
        draw_quad(rock.palette_tile_bottomright, 1, rock_pos);
    }

    for (int i = sprite_idx; i < 64; ++i) { // hide other sprites
        ppu.sprites[i].x = 0;
        ppu.sprites[i].y = PPU466::ScreenHeight + 8;
    }
    
    sprite_idx = 0;
    palette_idx = 2;
    tile_idx = 5;

	//--- actually draw ---
	ppu.draw(drawable_size);
}
