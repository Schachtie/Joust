#pragma once

#include "object.h"
#include "player.h"


typedef enum enemyType_t {
	ENEMYTYPE_BOUNDER,
	ENEMYTYPE_HUNTER,
	ENEMYTYPE_SHADOWLORD,
	ENEMYTYPE_EGG
} EnemyType;

typedef struct enemy_t Enemy;


typedef void (*EnemyActionCB)(const char*);


void enemySetEnemyActionCB(EnemyActionCB cb);
void enemyClearEnemyActionCB();

void enemyInitAnimations(const SpriteSheet* const sheet);
void enemyDeinitAnimations();

Enemy* enemyNew(Coord2D startPos, Coord2D size, EnemyType type);
void enemyDelete(Object* enemy);

void enemySetPlayerReference(const Player* player);
void enemyClearPlayerReference();

EnemyType enemyGetType(Enemy* enemy);
