

#include "joustGlobalConstants.h"


Coord2D SCREEN_RESOLUTION = { .x = 1024, .y = 768 }; // Pull this from the app / if it ever updates in the game.c file
Bounds2D SCREEN_BOUNDS = { .topLeft = {.x = 0, .y = 0}, .botRight = {.x = 1024, .y = 768} };


// Just about everything below this point is coming from a sprite sheet


// Background Images
	// Title screen
const Coord2D BACKGROUND_TITLE_SIZE = { .x = 302, .y = 255 };
const Bounds2D BACKGROUND_TITLE_SPRITE_BOUNDS = {	.topLeft = {.x = 0, .y = 254}, 
													.botRight = {.x = 301, .y = 0} };
	// Wave background
const Coord2D BACKGROUND_WAVES_SIZE = { .x = 708, .y = 582 }; // 708 x 582 / 1024 x 768 / 2560 x 1600
const Bounds2D BACKGROUND_WAVES_SPRITE_BOUNDS = {	.topLeft = {.x = 6, .y = 1136}, 
													.botRight = {.x = 713, .y = 555} };
	// Hiscores image
const Coord2D BACKGROUND_HISCORES_SIZE = { .x = 510, .y = 120 };
const Bounds2D BACKGROUND_HISCORES_SPRITE_BOUNDS = {	.topLeft = {.x = 6, .y = 1262}, 
														.botRight = {.x = 515, .y = 1144} };


// Start locations
	// Consider an easier way to write these
const Coord2D SPAWN_LOCATIONS[] = { {.x = 500, .y = 150 }, 
									{.x = 100, .y = 150 }, 
									{.x = 250, .y = 150 } };
const uint8_t NUMBER_SPAWN_LOCATIONS = 3; // can this be determined by sizeof????? -> look into this during comment pass


// Player - Relative to entire sprite sheet
const Coord2D PLAYER_SIZE_GROUNDED = { .x = 39, .y = 54 };
const Coord2D PLAYER_SIZE_FLYING = { .x = 39, .y = 36 };

const uint8_t PLAYER_ANIMATION_IDLE_NUM_FRAMES = 1;
const float PLAYER_ANIMATION_IDLE_PIXELS_BETWEEN_FRAMES = 0.0f;
const Bounds2D PLAYER_ANIMATION_IDLE_FIRST_SPRITE_BOUNDS = {	.topLeft = {.x = 745, .y = 1136},
																.botRight = {.x = 781.7f, .y = 1084} };;

const uint8_t PLAYER_ANIMATION_RUNNING_NUM_FRAMES = 4;
const float PLAYER_ANIMATION_RUNNING_PIXELS_BETWEEN_FRAMES = 8.5f;
const Bounds2D PLAYER_ANIMATION_RUNNING_FIRST_SPRITE_BOUNDS = { .topLeft = {.x = 745, .y = 1076},
																.botRight = {.x = 781.5f, .y = 1024} };

const uint8_t PLAYER_ANIMATION_RUNSLOWING_NUM_FRAMES = 1;
const float PLAYER_ANIMATION_RUNSLOWING_PIXELS_BETWEEN_FRAMES = 0.0f;
const Bounds2D PLAYER_ANIMATION_RUNSLOWING_FIRST_SPRITE_BOUNDS = {	.topLeft = {.x = 745, .y = 1016},
																	.botRight = {.x = 782, .y = 970} };

const uint8_t PLAYER_ANIMATION_FLYING_NUM_FRAMES = 2;
const float PLAYER_ANIMATION_FLYING_PIXELS_BETWEEN_FRAMES = 7.5f;
const Bounds2D PLAYER_ANIMATION_FLYING_FIRST_SPRITE_BOUNDS = {	.topLeft = {.x = 745, .y = 962},
																.botRight = {.x = 782, .y = 928} };


// Enemies - Relative to entire sprite sheet
const Coord2D ENEMY_SIZE_GROUNDED = { .x = 39, .y = 54 };
const Coord2D ENEMY_SIZE_FLYING = { .x = 39, .y = 36 };

	// Bounder
const uint8_t ENEMY_BOUNDER_ANIMATION_IDLE_NUM_FRAMES = 1;
const float ENEMY_BOUNDER_ANIMATION_IDLE_PIXELS_BETWEEN_FRAMES = 0.0f;
const Bounds2D ENEMY_BOUNDER_ANIMATION_IDLE_FIRST_SPRITE_BOUNDS = { .topLeft = {.x = 744, .y = 686},
																	.botRight = {.x = 782, .y = 633} };

const uint8_t ENEMY_BOUNDER_ANIMATION_RUNNING_NUM_FRAMES = 4;
const float ENEMY_BOUNDER_ANIMATION_RUNNING_PIXELS_BETWEEN_FRAMES = 8.0f;
const Bounds2D ENEMY_BOUNDER_ANIMATION_RUNNING_FIRST_SPRITE_BOUNDS = {	.topLeft = {.x = 745, .y = 686},
																		.botRight = {.x = 782, .y = 634} };

const uint8_t ENEMY_BOUNDER_ANIMATION_RUNSLOWING_NUM_FRAMES = 1;
const float ENEMY_BOUNDER_ANIMATION_RUNSLOWING_PIXELS_BETWEEN_FRAMES = 0.0f;
const Bounds2D ENEMY_BOUNDER_ANIMATION_RUNSLOWING_FIRST_SPRITE_BOUNDS = {	.topLeft = {.x = 880, .y = 746},
																			.botRight = {.x = 917, .y = 694} };

const uint8_t ENEMY_BOUNDER_ANIMATION_FLYING_NUM_FRAMES = 2;
const float ENEMY_BOUNDER_ANIMATION_FLYING_PIXELS_BETWEEN_FRAMES = 7.5f;
const Bounds2D ENEMY_BOUNDER_ANIMATION_FLYING_FIRST_SPRITE_BOUNDS = {	.topLeft = {.x = 745, .y = 626},
																		.botRight = {.x = 782, .y = 591.5f} };

	// Hunter
const uint8_t ENEMY_HUNTER_ANIMATION_IDLE_NUM_FRAMES = 1;
const float ENEMY_HUNTER_ANIMATION_IDLE_PIXELS_BETWEEN_FRAMES = 0.0f;
const Bounds2D ENEMY_HUNTER_ANIMATION_IDLE_FIRST_SPRITE_BOUNDS = {	.topLeft = {.x = 975, .y = 615},
																	.botRight = {.x = 1013, .y = 562} };

const uint8_t ENEMY_HUNTER_ANIMATION_RUNNING_NUM_FRAMES = 4;
const float ENEMY_HUNTER_ANIMATION_RUNNING_PIXELS_BETWEEN_FRAMES = 8.25f;
const Bounds2D ENEMY_HUNTER_ANIMATION_RUNNING_FIRST_SPRITE_BOUNDS = {	.topLeft = {.x = 976.5f, .y = 615},
																		.botRight = {.x = 1013, .y = 563} };

const uint8_t ENEMY_HUNTER_ANIMATION_RUNSLOWING_NUM_FRAMES = 1;
const float ENEMY_HUNTER_ANIMATION_RUNSLOWING_PIXELS_BETWEEN_FRAMES = 0.0f;
const Bounds2D ENEMY_HUNTER_ANIMATION_RUNSLOWING_FIRST_SPRITE_BOUNDS = {	.topLeft = {.x = 1111, .y = 557},
																			.botRight = {.x = 1148, .y = 505} };

const uint8_t ENEMY_HUNTER_ANIMATION_FLYING_NUM_FRAMES = 2;
const float ENEMY_HUNTER_ANIMATION_FLYING_PIXELS_BETWEEN_FRAMES = 7.75f;
const Bounds2D ENEMY_HUNTER_ANIMATION_FLYING_FIRST_SPRITE_BOUNDS = {	.topLeft = {.x = 976.5f, .y = 555},
																		.botRight = {.x = 1013, .y = 521} };

	// Shadow Lord
const uint8_t ENEMY_SHADOWLORD_ANIMATION_IDLE_NUM_FRAMES = 1;
const float ENEMY_SHADOWLORD_ANIMATION_IDLE_PIXELS_BETWEEN_FRAMES = 0.0f;
const Bounds2D ENEMY_SHADOWLORD_ANIMATION_IDLE_FIRST_SPRITE_BOUNDS = {	.topLeft = {.x = 975, .y = 496},
																		.botRight = {.x = 1013, .y = 443} };

const uint8_t ENEMY_SHADOWLORD_ANIMATION_RUNNING_NUM_FRAMES = 4;
const float ENEMY_SHADOWLORD_ANIMATION_RUNNING_PIXELS_BETWEEN_FRAMES = 8.25f;
const Bounds2D ENEMY_SHADOWLORD_ANIMATION_RUNNING_FIRST_SPRITE_BOUNDS = {	.topLeft = {.x = 976.5f, .y = 496},
																			.botRight = {.x = 1013, .y = 444} };

const uint8_t ENEMY_SHADOWLORD_ANIMATION_RUNSLOWING_NUM_FRAMES = 1;
const float ENEMY_SHADOWLORD_ANIMATION_RUNSLOWING_PIXELS_BETWEEN_FRAMES = 0.0f;
const Bounds2D ENEMY_SHADOWLORD_ANIMATION_RUNSLOWING_FIRST_SPRITE_BOUNDS = {	.topLeft = {.x = 1111, .y = 437},
																				.botRight = {.x = 1148, .y = 385} };

const uint8_t ENEMY_SHADOWLORD_ANIMATION_FLYING_NUM_FRAMES = 2;
const float ENEMY_SHADOWLORD_ANIMATION_FLYING_PIXELS_BETWEEN_FRAMES = 7.75f;
const Bounds2D ENEMY_SHADOWLORD_ANIMATION_FLYING_FIRST_SPRITE_BOUNDS = { .topLeft = {.x = 976.5f, .y = 436},
																		.botRight = {.x = 1013, .y = 402} };


	// Relative to just the wave background
// Collision Boxes (in pixels) -> I'd like this to be more of a level editing thing, but for now we're using where the platforms are on the sprite sheet in relation to just the game background.
const Coord2D COLLBOX_BOT_TL = { .x = 0, .y = 471 }; // 117, 471
const Coord2D COLLBOX_BOT_BR = { .x = 2000, .y = 560 }; // 563, 560

const Coord2D COLLBOX_MIDLEFT_TL = { .x = 0, .y = 273 };
const Coord2D COLLBOX_MIDLEFT_BR = { .x = 146, .y = 293 };

const Coord2D COLLBOX_MID_TL = { .x = 243, .y = 342 };
const Coord2D COLLBOX_MID_BR = { .x = 395, .y = 362 };

const Coord2D COLLBOX_MIDRIGHT_TL = { .x = 474, .y = 249 };
const Coord2D COLLBOX_MIDRIGHT_BR = { .x = 611, .y = 278 };

const Coord2D COLLBOX_MIDRIGHT_WRAP_TL = { .x = 603, .y = 273 };
const Coord2D COLLBOX_MIDRIGHT_WRAP_BR = { .x = 707, .y = 290 };

const Coord2D COLLBOX_TOPLEFT_TL = { .x = 0, .y = 87 };
const Coord2D COLLBOX_TOPLEFT_BR = { .x = 71, .y = 104 };

const Coord2D COLLBOX_TOPMID_TL = { .x = 195, .y = 120 };
const Coord2D COLLBOX_TOPMID_BR = { .x = 404, .y = 146 };

const Coord2D COLLBOX_TOPRIGHT_TL = { .x = 594, .y = 87 };
const Coord2D COLLBOX_TOPRIGHT_BR = { .x = 707, .y = 104 };


// Number Sprites - Relative to the entire sprite sheet
const uint8_t NUMBER_SPRITE_PIXELS_BETWEEN_SPRITES = 18;

const Coord2D NUMBER_SPRITE_YELLOW_0_TL = { .x = 6.7f, .y = 371 };
const Coord2D NUMBER_SPRITE_YELLOW_0_BR = { .x = 20, .y = 352 };

const Coord2D NUMBER_SPRITE_BLUE_0_TL = { .x = 6.7f, .y = 344 };
const Coord2D NUMBER_SPRITE_BLUE_0_BR = { .x = 20, .y = 325 };

const Coord2D NUMBER_SPRITE_WHITE_0_TL = { .x = 6.7f, .y = 317 };
const Coord2D NUMBER_SPRITE_WHITE_0_BR = { .x = 20, .y = 298 };


// Score Board Location - Relative to just the wave background
const Coord2D SCOREBOARD_TL = { .x = 165, .y = 489 };
const Coord2D SCOREBOARD_BR = { .x = 265, .y = 512 };


// Lives Display Location - Relative to just the wave background
const Coord2D LIVES_DISPLAY_TL = { .x = 273, .y = 489 };
const Coord2D LIVES_DISPLAY_BR = { .x = 332, .y = 512 };


// Spawn Locations - Relative to just the wave background - These are just one pixel with (the top of the location) to use to determine the center pixel
const Bounds2D SPAWN_BOTTOM_LOCATION = {	.topLeft = {.x = 276, .y = 471},
											.botRight = {.x = 344, .y = 471} };

const Bounds2D SPAWN_LEFT_LOCATION = {	.topLeft = {.x = 27, .y = 273},
										.botRight = {.x = 92, .y = 273} };

const Bounds2D SPAWN_RIGHT_LOCATION = { .topLeft = {.x = 525, .y = 249},
										.botRight = {.x = 593, .y = 249} };

const Bounds2D SPAWN_TOP_LOCATION = {	.topLeft = {.x = 243, .y = 120},
										.botRight = {.x = 308, .y = 120} };


// Animation Timers
const uint32_t ANIMATION_SPEED_FLAP = 150;
const uint32_t ANIMATION_SPEED_RUN = 5000;


// Word Sprites
Coord2D WORDS_WAVE_SIZE = { .x = 0, .y = 0 };
const Bounds2D WORDS_SPRITE_WAVE = {	.topLeft = {.x = 6, .y = 188},
								.botRight = {.x = 80, .y = 168.5f} };

Coord2D WORDS_PREPARE_SIZE = { .x = 0, .y = 0 };
const Bounds2D WORDS_SPRITE_PREPARE = {	.topLeft = {.x = 6, .y = 161},
									.botRight = {.x = 290, .y = 141} };

Coord2D WORDS_GAMEOVER_SIZE = { .x = 0, .y = 0 };
const Bounds2D WORDS_SPRITE_GAMEOVER = {	.topLeft = {.x = 6, .y = 134},
									.botRight = {.x = 290, .y = 114} };

Coord2D WORDS_BUZZARD_SIZE = { .x = 0, .y = 0 };
const Bounds2D WORDS_SPRITE_BUZZARD = {	.topLeft = {.x = 6, .y = 107},
									.botRight = {.x = 218, .y = 87} };

Coord2D WORDS_SURVIVAL_SIZE = { .x = 0, .y = 0 };
const Bounds2D WORDS_SPRITE_SURVIVAL = {	.topLeft = {.x = 6, .y = 80},
									.botRight = {.x = 236, .y = 60} };

Coord2D WORDS_EGGWAVE_SIZE = { .x = 0, .y = 0 };
const Bounds2D WORDS_SPRITE_EGGWAVE = {	.topLeft = {.x = 6, .y = 53},
									.botRight = {.x = 152, .y = 33} };

Coord2D WORDS_BEWARE_SIZE = { .x = 0, .y = 0 };
const Bounds2D WORDS_SPRITE_BEWARE = { .topLeft = {.x = 6, .y = 26},
								.botRight = {.x = 698, .y = 6} };
