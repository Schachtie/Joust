#pragma once

#include <stdlib.h>

#include "object.h"
#include "entity.h"


typedef struct player_t Player;


typedef void (*PlayerEventCB)(void);
typedef void (*PlayerEnemyKilledCB)(Object* enemy);
typedef void (*PlayerActionCB)(const char*);


void playerSetEnemyKilledCB(PlayerEnemyKilledCB cb);
void playerClearEnemyKilledCB();

void playerSetPlayerKilledCB(PlayerEventCB cb);
void playerClearPlayerKilledCB();

void playerSetPlayerActionCB(PlayerActionCB cb);
void playerClearPlayerActionCB();

void playerInitAnimations(const SpriteSheet* const sheet);
void playerDeinitAnimations();

Player* playerNew(Coord2D startPos, Coord2D size);
void playerDelete(Object* player);

Object* playerGetObject(Player* player);
bool playerGetGroundedState(const Player* const player);
Coord2D playerGetPositionFlying(const Player* const player);
Coord2D playerGetPositionGrounded(const Player* const player);
uint8_t playerGetLives(const Player* const player);
bool playerAddLife(Player* player);
void playerResetLives(Player* player);
