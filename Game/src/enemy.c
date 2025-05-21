#include <math.h>
#include <stdlib.h>

#include "enemy.h"
#include "random.h"
#include "animation.h"
#include "joustGlobalConstants.h"


#define ENEMY_ANIM_WING_UP_FRAME	1
#define ENEMY_ANIM_WING_DOWN_FRAME	0


typedef struct enemy_t {
	Entity				entity;
	
	EnemyType			enemyType;

	uint32_t			_msPerDirection;
	uint32_t			_msDirectionCounter;
	uint32_t			_msPerFlap;
	uint32_t			_msFlapCounter;

		// Note that these are all also in player. If there is time to refactor, please do so! (There is not.)
	const Animation*	_animation;
	uint8_t				_amCurFrame;
	uint32_t			_amSpeedMSIdle;
	uint32_t			_amSpeedMSRun;
	uint32_t			_amSpeedMSRunSlow;
	uint32_t			_amSpeedMSFly;
	uint32_t			_amTimerIdle;
	uint32_t			_amTimerRun;
	uint32_t			_amTimerRunSlow;
	uint32_t			_amTimerFly;

	bool				_currentDirection; // This is for turning the AI around smoothly [false - left | true - right]
	bool				_intendedDirection;

	float				_sightRadius;
} Enemy;


static const uint32_t _MS_PER_FLAP_MIN = 350;
static const uint32_t _MS_PER_FLAP_MAX = 850;

static const uint32_t _MS_PER_DIRECTION_MIN = 1000;
static const uint32_t _MS_PER_DIRECTION_MAX = 10000;

static const float _SIGHT_RADIUS_BOUNDER = 200.0f;
static const float _SIGHT_RADIUS_HUNTER = 400.0f;
static const float _SIGHT_RADIUS_SHADOWLORD = 600.0f;

static const Coord2D _TERMINAL_VELOCITY_BOUNDER = { .x = 100.0f, .y = 100.0f };
static const Coord2D _TERMINAL_VELOCITY_HUNTER = { .x = 200.0f, .y = 200.0f };
static const Coord2D _TERMINAL_VELOCITY_SHADOWLORD = { .x = 300.0f, .y = 300.0f };


static Animation* _animBounderIdle = NULL;
static Animation* _animBounderRunning = NULL;
static Animation* _animBounderRunSlowing = NULL;
static Animation* _animBounderFlying = NULL;

static Animation* _animHunterIdle = NULL;
static Animation* _animHunterRunning = NULL;
static Animation* _animHunterRunSlowing = NULL;
static Animation* _animHunterFlying = NULL;

static Animation* _animShadowLordIdle = NULL;
static Animation* _animShadowLordRunning = NULL;
static Animation* _animShadowLordRunSlowing = NULL;
static Animation* _animShadowLordFlying = NULL;


static const Player* _playerReference = NULL;


static EnemyActionCB _enemyActionCB = NULL;


static void _enemyTriggerEnemyActionCB(const char* action);


// =============== vTable ===============
static void _enemyDraw(Object* obj);
static void _enemyUpdate(Object* obj, uint32_t milliseconds);
static void _enemyCollide(Object* thisObj, Object* otherObj, Collision collision); //now deprecated
static ObjVtable _enemyVtable = {
	enemyDelete,
	_enemyDraw,
	_enemyUpdate,
	_enemyCollide
};


/// <summary>
/// Initializes all enemy animations.
/// </summary>
/// <param name="sheet"></param>
void enemyInitAnimations(const SpriteSheet* const sheet)
{
	_animBounderIdle = animationNew(ENEMY_BOUNDER_ANIMATION_IDLE_NUM_FRAMES, sheet, ENEMY_BOUNDER_ANIMATION_IDLE_FIRST_SPRITE_BOUNDS, ENEMY_BOUNDER_ANIMATION_IDLE_PIXELS_BETWEEN_FRAMES, 0);
	_animBounderRunning = animationNew(ENEMY_BOUNDER_ANIMATION_RUNNING_NUM_FRAMES, sheet, ENEMY_BOUNDER_ANIMATION_RUNNING_FIRST_SPRITE_BOUNDS, ENEMY_BOUNDER_ANIMATION_RUNNING_PIXELS_BETWEEN_FRAMES, 0);
	_animBounderRunSlowing = animationNew(ENEMY_BOUNDER_ANIMATION_RUNSLOWING_NUM_FRAMES, sheet, ENEMY_BOUNDER_ANIMATION_RUNSLOWING_FIRST_SPRITE_BOUNDS, ENEMY_BOUNDER_ANIMATION_RUNSLOWING_PIXELS_BETWEEN_FRAMES, 0);
	_animBounderFlying = animationNew(ENEMY_BOUNDER_ANIMATION_FLYING_NUM_FRAMES, sheet, ENEMY_BOUNDER_ANIMATION_FLYING_FIRST_SPRITE_BOUNDS, ENEMY_BOUNDER_ANIMATION_FLYING_PIXELS_BETWEEN_FRAMES, 0);

	_animHunterIdle = animationNew(ENEMY_HUNTER_ANIMATION_IDLE_NUM_FRAMES, sheet, ENEMY_HUNTER_ANIMATION_IDLE_FIRST_SPRITE_BOUNDS, ENEMY_HUNTER_ANIMATION_IDLE_PIXELS_BETWEEN_FRAMES, 0);
	_animHunterRunning = animationNew(ENEMY_HUNTER_ANIMATION_RUNNING_NUM_FRAMES, sheet, ENEMY_HUNTER_ANIMATION_RUNNING_FIRST_SPRITE_BOUNDS, ENEMY_HUNTER_ANIMATION_RUNNING_PIXELS_BETWEEN_FRAMES, 0);
	_animHunterRunSlowing = animationNew(ENEMY_HUNTER_ANIMATION_RUNSLOWING_NUM_FRAMES, sheet, ENEMY_HUNTER_ANIMATION_RUNSLOWING_FIRST_SPRITE_BOUNDS, ENEMY_HUNTER_ANIMATION_RUNSLOWING_PIXELS_BETWEEN_FRAMES, 0);
	_animHunterFlying = animationNew(ENEMY_HUNTER_ANIMATION_FLYING_NUM_FRAMES, sheet, ENEMY_HUNTER_ANIMATION_FLYING_FIRST_SPRITE_BOUNDS, ENEMY_HUNTER_ANIMATION_FLYING_PIXELS_BETWEEN_FRAMES, 0);

	_animShadowLordIdle = animationNew(ENEMY_SHADOWLORD_ANIMATION_IDLE_NUM_FRAMES, sheet, ENEMY_SHADOWLORD_ANIMATION_IDLE_FIRST_SPRITE_BOUNDS, ENEMY_SHADOWLORD_ANIMATION_IDLE_PIXELS_BETWEEN_FRAMES, 0);
	_animShadowLordRunning = animationNew(ENEMY_SHADOWLORD_ANIMATION_RUNNING_NUM_FRAMES, sheet, ENEMY_SHADOWLORD_ANIMATION_RUNNING_FIRST_SPRITE_BOUNDS, ENEMY_SHADOWLORD_ANIMATION_RUNNING_PIXELS_BETWEEN_FRAMES, 0);
	_animShadowLordRunSlowing = animationNew(ENEMY_SHADOWLORD_ANIMATION_RUNSLOWING_NUM_FRAMES, sheet, ENEMY_SHADOWLORD_ANIMATION_RUNSLOWING_FIRST_SPRITE_BOUNDS, ENEMY_SHADOWLORD_ANIMATION_RUNSLOWING_PIXELS_BETWEEN_FRAMES, 0);
	_animShadowLordFlying = animationNew(ENEMY_SHADOWLORD_ANIMATION_FLYING_NUM_FRAMES, sheet, ENEMY_SHADOWLORD_ANIMATION_FLYING_FIRST_SPRITE_BOUNDS, ENEMY_SHADOWLORD_ANIMATION_FLYING_PIXELS_BETWEEN_FRAMES, 0);
}

/// <summary>
/// Deletes all enemy animations and sets their references to NULL.
/// </summary>
void enemyDeinitAnimations()
{
	animationDelete(_animBounderIdle);
	_animBounderIdle = NULL;

	animationDelete(_animBounderRunning);
	_animBounderRunning = NULL;

	animationDelete(_animBounderRunSlowing);
	_animBounderRunSlowing = NULL;

	animationDelete(_animBounderFlying);
	_animBounderFlying = NULL;


	animationDelete(_animHunterIdle);
	_animHunterIdle = NULL;

	animationDelete(_animHunterRunning);
	_animHunterRunning = NULL;

	animationDelete(_animHunterRunSlowing);
	_animHunterRunSlowing = NULL;

	animationDelete(_animHunterFlying);
	_animHunterFlying = NULL;


	animationDelete(_animShadowLordIdle);
	_animShadowLordIdle = NULL;

	animationDelete(_animShadowLordRunning);
	_animShadowLordRunning = NULL;

	animationDelete(_animShadowLordRunSlowing);
	_animShadowLordRunSlowing = NULL;

	animationDelete(_animShadowLordFlying);
	_animShadowLordFlying = NULL;
}


/// <summary>
/// Creates a new enemy object based on the passed in type.
/// </summary>
/// <param name="startPos"></param>
/// <param name="size"></param>
/// <param name="type"></param>
/// <returns></returns>
Enemy* enemyNew(Coord2D startPos, Coord2D size, EnemyType type)
{
	Enemy* enemy = (Enemy*)malloc(sizeof(Enemy));
	if (enemy != NULL)
	{
		entityInit(&enemy->entity, &_enemyVtable, startPos, size);

		enemy->entity.awake = true;
		enemy->entity.collresp = COLLRESP_ENEMY;
		enemy->enemyType = type;

		enemy->entity.flyingSize = ENEMY_SIZE_FLYING;
		enemy->entity.flyingPosition = startPos;

		enemy->_msDirectionCounter = 0;
		enemy->_msFlapCounter = 0;
		enemy->_msPerDirection = randGetInt(_MS_PER_DIRECTION_MIN, _MS_PER_DIRECTION_MAX);
		enemy->_msPerFlap = randGetInt(_MS_PER_FLAP_MIN, _MS_PER_FLAP_MAX);
		enemy->_currentDirection = (randGetInt(0, 2) == 0) ? false : true;
		enemy->_intendedDirection = enemy->_currentDirection;

		enemy->_animation = NULL;
		enemy->_amCurFrame = 0;
		enemy->_amSpeedMSIdle = 0;
		enemy->_amSpeedMSRun = ANIMATION_SPEED_RUN;
		enemy->_amSpeedMSRunSlow = 0;
		enemy->_amSpeedMSFly = ANIMATION_SPEED_FLAP;
		enemy->_amTimerIdle = 0;
		enemy->_amTimerRun = 0;
		enemy->_amTimerRunSlow = 0;
		enemy->_amTimerFly = 0;

		switch (enemy->enemyType)
		{
			case ENEMYTYPE_BOUNDER:
			{
				enemy->_animation = _animBounderIdle;

				enemy->_sightRadius = _SIGHT_RADIUS_BOUNDER;
				enemy->entity.terminalVelocity = _TERMINAL_VELOCITY_BOUNDER;
				break;
			}
			case ENEMYTYPE_HUNTER:
			{
				enemy->_animation = _animHunterIdle;

				enemy->_sightRadius = _SIGHT_RADIUS_HUNTER;
				enemy->entity.terminalVelocity = _TERMINAL_VELOCITY_HUNTER;
				break;
			}
			case ENEMYTYPE_SHADOWLORD:
			{
				enemy->_animation = _animShadowLordIdle;

				enemy->_sightRadius = _SIGHT_RADIUS_SHADOWLORD;
				enemy->entity.terminalVelocity = _TERMINAL_VELOCITY_SHADOWLORD;
				break;
			}
			default:
			{
				break;
			}
		}
	}
	return enemy;
}

/// <summary>
/// Deletes the passed in enemy object.
/// </summary>
/// <param name="enemy"></param>
void enemyDelete(Object* enemy)
{
	if (enemy != NULL)
	{
		entityDeinit(&((Enemy*)enemy)->entity);
	}
	free(enemy);
}


/// <summary>
/// Draws the enemy onto the screen based on its internal state.
/// </summary>
/// <param name="obj"></param>
static void _enemyDraw(Object* obj)
{
	Enemy* enemy = (Enemy*)obj;

	// Get the appropriate animations based on the enemy type
	const Animation* animIdle = NULL;
	const Animation* animRun = NULL;
	const Animation* animRunSlow = NULL;
	const Animation* animFly = NULL;
	switch (enemy->enemyType)
	{
		case ENEMYTYPE_BOUNDER:
		{
			animIdle = _animBounderIdle;
			animRun = _animBounderRunning;
			animRunSlow = _animBounderRunSlowing;
			animFly = _animBounderFlying;
			break;
		}
		case ENEMYTYPE_HUNTER:
		{
			animIdle = _animHunterIdle;
			animRun = _animHunterRunning;
			animRunSlow = _animHunterRunSlowing;
			animFly = _animHunterFlying;
			break;
		}
		case ENEMYTYPE_SHADOWLORD:
		{
			animIdle = _animShadowLordIdle;
			animRun = _animShadowLordRunning;
			animRunSlow = _animShadowLordRunSlowing;
			animFly = _animShadowLordFlying;
			break;
		}
		default:
		{
			assert(false);
			break;
		}
	}

	// Get the appropriate animation speed/timer based on the enemy's current animation
	uint32_t animationSpeed = 0;
	uint32_t* animationTimer = NULL;
	Coord2D enemySize = enemy->entity.obj.size;
	Coord2D enemyPos = enemy->entity.obj.position;
	if (enemy->_animation == animIdle)
	{
		animationSpeed = enemy->_amSpeedMSIdle;
		animationTimer = &enemy->_amTimerIdle;
	}
	else if (enemy->_animation == animRun)
	{
		animationSpeed = enemy->_amSpeedMSRun;
		animationTimer = &enemy->_amTimerRun;
	}
	else if (enemy->_animation == animRunSlow)
	{
		animationSpeed = enemy->_amSpeedMSRunSlow;
		animationTimer = &enemy->_amTimerRunSlow;
	}
	else if (enemy->_animation == animFly)
	{
		animationSpeed = enemy->_amSpeedMSFly;
		animationTimer = &enemy->_amTimerFly;
		enemySize = enemy->entity.flyingSize;
		enemyPos = enemy->entity.flyingPosition;
	}
	assert(animationTimer != NULL);

	// Check if the animation should move to the next sprite frame
	if (*animationTimer >= animationSpeed)
	{
		*animationTimer = 0;
		if (++enemy->_amCurFrame >= enemy->_animation->numFrames)
		{
			if (enemy->_animation == animFly)
			{
				enemy->_amCurFrame = ENEMY_ANIM_WING_UP_FRAME;
			}
			else
			{
				enemy->_amCurFrame = 0;
			}
		}
	}

	// Draw the current sprite frame
	animationDraw(enemy->_animation, enemy->_amCurFrame, enemyPos, enemySize, !enemy->_currentDirection);


	// Check for other side of screen redraw here
		// Approaching left wall
	if (enemyPos.x - enemySize.x / 2 < enemy->entity.gameBounds.topLeft.x)
	{
		float overlap = enemy->entity.gameBounds.topLeft.x - (enemyPos.x - enemySize.x / 2);
		Coord2D wrapDrawPos = { .x = enemy->entity.gameBounds.botRight.x + (enemySize.x / 2 - overlap), .y = enemyPos.y };
		animationDraw(enemy->_animation, enemy->_amCurFrame, wrapDrawPos, enemySize, !enemy->_currentDirection);
	}
		// Approaching right wall
	else if (enemyPos.x + enemySize.x / 2 > enemy->entity.gameBounds.botRight.x)
	{
		float overlap = (enemyPos.x + enemySize.x / 2) - enemy->entity.gameBounds.botRight.x;
		Coord2D wrapDrawPos = { .x = enemy->entity.gameBounds.topLeft.x - (enemySize.x / 2 - overlap), .y = enemyPos.y };
		animationDraw(enemy->_animation, enemy->_amCurFrame, wrapDrawPos, enemySize, !enemy->_currentDirection);
	}
}

/// <summary>
/// Updates the enemy based on its internal state. This involves setting the specific animation if necessary, updating the enemy's position and velocity, and performing AI behavior.
///		<para>
/// AI behavior is random unless the player is within sight of the enemy, in which case the enemy moves towards (and above) the player.
///		</para>
/// </summary>
/// <param name="obj"></param>
/// <param name="milliseconds"></param>
static void _enemyUpdate(Object* obj, uint32_t milliseconds)
{
	Enemy* enemy = (Enemy*)obj;

	// Set the references to the enemy's specific animations
	const Animation* animIdle = NULL;
	const Animation* animRun = NULL;
	const Animation* animRunSlow = NULL;
	const Animation* animFly = NULL;
	switch (enemy->enemyType)
	{
		case ENEMYTYPE_BOUNDER:
		{
			animIdle = _animBounderIdle;
			animRun = _animBounderRunning;
			animRunSlow = _animBounderRunSlowing;
			animFly = _animBounderFlying;
			break;
		}
		case ENEMYTYPE_HUNTER:
		{
			animIdle = _animHunterIdle;
			animRun = _animHunterRunning;
			animRunSlow = _animHunterRunSlowing;
			animFly = _animHunterFlying;
			break;
		}
		case ENEMYTYPE_SHADOWLORD:
		{
			animIdle = _animShadowLordIdle;
			animRun = _animShadowLordRunning;
			animRunSlow = _animShadowLordRunSlowing;
			animFly = _animShadowLordFlying;
			break;
		}
		default:
		{
			break;
		}
	}

	// Radius check to player
	Coord2D playerPosition = playerGetPositionGrounded(_playerReference);
	Coord2D enemyPosition = enemy->entity.obj.position;
		// If player/enemy are flying get their respective positions
	if (playerGetGroundedState(_playerReference) == false)
	{
		playerPosition = playerGetPositionFlying(_playerReference);
	}
	if (enemy->entity.isGrounded == false)
	{
		enemyPosition = enemy->entity.flyingPosition;
	}

	float distanceFromPlayerSquared = (float)pow(playerPosition.x - enemyPosition.x, 2) + (float)pow(playerPosition.y - enemyPosition.y, 2);
	if (distanceFromPlayerSquared <= (float)pow(enemy->_sightRadius, 2) && objIsEnabled((Object*)_playerReference)) // Player is in sight, move towards them
	{
		// Check for a necessary flap  (player is above enemy)
		if (playerPosition.y <= enemyPosition.y)
		{
			enemy->entity.velocity.y -= ENT_BALANCE_FLAP_SINGLE * ENT_DEFAULT_VELOCITY_CHANGE;
			enemy->_amCurFrame = ENEMY_ANIM_WING_DOWN_FRAME;
			if (enemy->entity.isGrounded)
			{
				enemy->entity.isGrounded = false;
				enemy->_animation = animFly;
				enemy->_amCurFrame = ENEMY_ANIM_WING_UP_FRAME;
				enemy->entity.obj.position.y -= MAGIC_NUMBER_PLATFORMFLAP * 2.0f;
			}
			//_enemyTriggerEnemyActionCB("flap"); //CONSIDER ADDING THIS, IT SEEMS TO BE OVERWHELMING THOUGH
		}

		// Make sure the enemy's intended direction is towards the player
		if (playerPosition.x < enemyPosition.x)
		{
			enemy->_intendedDirection = false;

			if (enemy->entity.isGrounded == false) { enemy->_currentDirection = enemy->_intendedDirection; } // In the air so change facing direction instantly
			else // On the ground so enemy should switch to the slowRun animation
			{
				enemy->_animation = animRunSlow;
				enemy->_amCurFrame = 0;
			}

			enemy->entity.velocity.x -= ENT_DEFAULT_VELOCITY_CHANGE / ((enemy->entity.isGrounded) ? 1 : 2);
		}
		else if (playerPosition.x > enemyPosition.x)
		{
			enemy->_intendedDirection = true;

			if (enemy->entity.isGrounded == false) { enemy->_currentDirection = enemy->_intendedDirection; } // In the air so change facing direction instantly
			else // On the ground so enemy should switch to the slowRun animation
			{
				enemy->_animation = animRunSlow;
				enemy->_amCurFrame = 0;
			}

			enemy->entity.velocity.x += ENT_DEFAULT_VELOCITY_CHANGE / ((enemy->entity.isGrounded) ? 1 : 2);
		}
	}
	else // Player is not in sight, move around randomly
	{
		if (enemy->entity.isGrounded && enemy->_animation != animRun)
		{
			enemy->_animation = animRun;
			enemy->_amCurFrame = 0;
		}

		// Check for a flap
		enemy->_msFlapCounter += milliseconds;
		if (enemy->_msFlapCounter >= enemy->_msPerFlap)
		{
			enemy->_msFlapCounter = 0;
			enemy->_msPerFlap = randGetInt(_MS_PER_FLAP_MIN, _MS_PER_FLAP_MAX);

			// Now flap
			enemy->entity.velocity.y -= ENT_BALANCE_FLAP_SINGLE * ENT_DEFAULT_VELOCITY_CHANGE;
			enemy->_amCurFrame = ENEMY_ANIM_WING_DOWN_FRAME;
			if (enemy->entity.isGrounded)
			{
				enemy->entity.isGrounded = false;
				enemy->_animation = animFly;
				enemy->_amCurFrame = ENEMY_ANIM_WING_UP_FRAME;
				enemy->entity.obj.position.y -= MAGIC_NUMBER_PLATFORMFLAP * 2.0f;
			}
			//_enemyTriggerEnemyActionCB("flap"); //CONSIDER ADDING THIS, IT SEEMS TO BE OVERWHELMING THOUGH
		}

		// Check for direction change
		enemy->_msDirectionCounter += milliseconds;
		if (enemy->_msDirectionCounter >= enemy->_msPerDirection)
		{
			enemy->_intendedDirection = !enemy->_currentDirection;
			if (enemy->entity.isGrounded == false) { enemy->_currentDirection = enemy->_intendedDirection; } // In the air so change facing direction instantly
			else // On the ground so enemy should switch to the slowRun animation
			{
				enemy->_animation = animRunSlow;
				enemy->_amCurFrame = 0;
			}

			// New random counter for changing direction
			enemy->_msDirectionCounter = 0;
			enemy->_msPerDirection = randGetInt(_MS_PER_DIRECTION_MIN, _MS_PER_DIRECTION_MAX);
		}

		if (enemy->_currentDirection == false && enemy->entity.velocity.x != -(enemy->entity.terminalVelocity.x)) // I think the 2nd check here is irrelevant as velocity is already capped in entityDefaultUpdate
		{
			enemy->entity.velocity.x -= ENT_DEFAULT_VELOCITY_CHANGE / ((enemy->entity.isGrounded) ? 1 : 2);
		}
		else if (enemy->_currentDirection == true && enemy->entity.velocity.x != enemy->entity.terminalVelocity.x)
		{
			enemy->entity.velocity.x += ENT_DEFAULT_VELOCITY_CHANGE / ((enemy->entity.isGrounded) ? 1 : 2);
		}
	}

	// Call the parent class's default update
	entityDefaultUpdate(obj, milliseconds);

	// Update the animation speeds/timers accordingly
	if (enemy->_animation == animIdle)
	{
		enemy->_amTimerIdle += milliseconds;
	}
	else if (enemy->_animation == animRun)
	{
		enemy->_amTimerRun += milliseconds * (uint32_t)fabs(enemy->entity.velocity.x);
	}
	else if (enemy->_animation == animRunSlow)
	{
		enemy->_amTimerRunSlow += milliseconds;
	}
	else if (enemy->_animation == animFly && enemy->_amCurFrame == ENEMY_ANIM_WING_DOWN_FRAME)
	{
		enemy->_amTimerFly += milliseconds;
	}

	// Update the enemy's direction appropriately
	if (enemy->entity.isGrounded)
	{
		if (enemy->entity.velocity.x > 0 && enemy->_currentDirection == false) 
		{ 
			enemy->_currentDirection = true; 
			enemy->_animation = animRun;
			enemy->_amCurFrame = 0;
		}
		else if (enemy->entity.velocity.x < 0 && enemy->_currentDirection == true) 
		{ 
			enemy->_currentDirection = false; 
			enemy->_animation = animRun;
			enemy->_amCurFrame = 0;
		}
	}
	else	// Enemy is flying
	{
		// Set the enemy's flying position
		Coord2D enemyTopLeft = { .x = enemy->entity.obj.position.x - enemy->entity.obj.size.x / 2, .y = enemy->entity.obj.position.y - enemy->entity.obj.size.y / 2 };
		Coord2D enemyBotRightFlying = { .x = enemyTopLeft.x + enemy->entity.flyingSize.x, .y = enemyTopLeft.y + enemy->entity.flyingSize.y };
		Bounds2D enemyFlyingBounds = { .topLeft = enemyTopLeft, .botRight = enemyBotRightFlying };
		enemy->entity.flyingPosition = boundsGetCenter(&enemyFlyingBounds);

		// Set the animation if necessary
		if (enemy->_animation != animFly)
		{
			enemy->_animation = animFly;
			enemy->_amCurFrame = 0;
		}
	}

	// This will reset if the enemy is on top of a platform anyway
	enemy->entity.isGrounded = false;
}

/// <summary>
/// Handles the enemy's collision response as necessary. Specifically if an enemy collides with another enemy. Otherwise it calls its parent class's collide function.
/// </summary>
/// <param name="thisObj"></param>
/// <param name="otherObj"></param>
/// <param name="collision"></param>
static void _enemyCollide(Object* thisObj, Object* otherObj, Collision collision)
{
	Entity* thisEntity = (Entity*)thisObj;
	Entity* otherEntity = (Entity*)otherObj;

	switch (otherEntity->collresp)
	{
		case COLLRESP_ENEMY: // Enemies should bounce off each other
		{
			Coord2D thisEntityPos = thisEntity->obj.position;
			Coord2D thisEntitySize = thisEntity->obj.size;
			Coord2D otherEntityPos = otherEntity->obj.position;
			Coord2D otherEntitySize = otherEntity->obj.size;

			// Since we might be flying, check if we need to use a smaller collision box
			if (thisEntity->isGrounded == false)
			{
				thisEntityPos = thisEntity->flyingPosition;
				thisEntitySize = thisEntity->flyingSize;

				// Get the thisEntity's bounds
				Coord2D thisTopLeft = { .x = thisEntity->flyingPosition.x - thisEntity->flyingSize.x / 2, .y = thisEntity->flyingPosition.y - thisEntity->flyingSize.y / 2 };
				Coord2D thisBotRight = { .x = thisTopLeft.x + thisEntity->flyingSize.x, .y = thisTopLeft.y + thisEntity->flyingSize.y };
				Bounds2D thisBounds = { .topLeft = thisTopLeft, .botRight = thisBotRight };

				// Get the otherEntity's bounds
				Bounds2D otherBounds;
				if (otherEntity->isGrounded == false) // Flying
				{
					otherEntityPos = otherEntity->flyingPosition;
					otherEntitySize = otherEntity->flyingSize;

					Coord2D otherTopleft = { .x = otherEntity->flyingPosition.x - otherEntity->flyingSize.x / 2, .y = otherEntity->flyingPosition.y - otherEntity->flyingSize.y / 2 };
					Coord2D otherBotRight = { .x = otherTopleft.x + otherEntity->flyingSize.x, .y = otherTopleft.y + otherEntity->flyingSize.y };
					otherBounds.topLeft = otherTopleft;
					otherBounds.botRight = otherBotRight;
				}
				else // Grounded
				{
					Coord2D otherTopLeft = { .x = otherEntity->obj.position.x - otherEntity->obj.size.x / 2, .y = otherEntity->obj.position.y - otherEntity->obj.size.y / 2 };
					Coord2D otherBotRight = { .x = otherTopLeft.x + otherEntity->obj.size.x, .y = otherTopLeft.y + otherEntity->obj.size.y };
					otherBounds.topLeft = otherTopLeft;
					otherBounds.botRight = otherBotRight;
				}

				// Check for collision again
				Collision doubleCheck = detectCollision(&thisBounds, &otherBounds);
				if (doubleCheck.isColliding == false)
				{
					return;
				}
				collision = doubleCheck;
			}

			Enemy* thisEnemy = (Enemy*)thisEntity;
			Enemy* otherEnemy = (Enemy*)otherEntity;
			float pushValue = 0.5f;
			float movement = 0;

			// Vertical push
			if (collision.intersect.x < collision.intersect.y)
			{
				movement = collision.intersect.y * pushValue;
				thisEnemy->entity.velocity.y = -(thisEnemy->entity.velocity.y);
				otherEnemy->entity.velocity.y = -(thisEnemy->entity.velocity.y);

				// Top of other enemy
				if (collision.delta.y > 0.0f)
				{
					// Push the enemy out of the other enemy
					thisEnemy->entity.obj.position.y -= movement;
					thisEnemy->entity.flyingPosition.y -= movement;
					otherEnemy->entity.obj.position.y += movement;
					otherEnemy->entity.flyingPosition.y += movement;
				}
				else // Below other enemy
				{
					thisEnemy->entity.obj.position.y += movement;
					thisEnemy->entity.flyingPosition.y += movement;
					otherEnemy->entity.obj.position.y -= movement;
					otherEnemy->entity.flyingPosition.y -= movement;
				}
			}
			// Horizontal push
			else if (collision.intersect.x > collision.intersect.y)
			{
				movement = collision.intersect.x * pushValue;
				thisEnemy->entity.velocity.x = -(thisEnemy->entity.velocity.x);
				otherEnemy->entity.velocity.x = -(thisEnemy->entity.velocity.x);

				// Left of other enemy
				if (collision.delta.x > 0.0f)
				{
					thisEnemy->entity.obj.position.x -= movement;
					thisEnemy->entity.flyingPosition.x -= movement;
					otherEnemy->entity.obj.position.x += movement;
					otherEnemy->entity.flyingPosition.x += movement;
				}
				else // Right of other enemy
				{
					thisEnemy->entity.obj.position.x += movement;
					thisEnemy->entity.flyingPosition.x += movement;
					otherEnemy->entity.obj.position.x -= movement;
					otherEnemy->entity.flyingPosition.x -= movement;
				}

				// Reverse the directions of both enemies
				thisEnemy->_intendedDirection = !thisEnemy->_intendedDirection;
				thisEnemy->_currentDirection = thisEnemy->_intendedDirection;
				otherEnemy->_intendedDirection = !otherEnemy->_intendedDirection;
				otherEnemy->_currentDirection = otherEnemy->_intendedDirection;
			}
		
		
			break;
		}
		case COLLRESP_PLATFORM: // Should fall through into default
		{
			// Make sure we're not on the top of a platform
			if (collision.intersect.x >= collision.intersect.y || collision.delta.y <= 0.0f)
			{
				//_enemyTriggerEnemyActionCB("platform"); //CONSIDER ADDING THIS, IT SEEMS TO BE OVERWHELMING THOUGH
			}
		}
		default:
		{
			entityDefaultCollide(thisObj, otherObj, collision);
			break;
		}
	}
}


/// <summary>
/// Triggers the enemyAction callback if it has been set.
/// </summary>
/// <param name="action"></param>
static void _enemyTriggerEnemyActionCB(const char* action)
{
	if (_enemyActionCB != NULL)
	{
		_enemyActionCB(action);
	}
}


/// <summary>
/// Sets a class reference to the player.
/// </summary>
/// <param name="player"></param>
void enemySetPlayerReference(const Player* player)
{
	_playerReference = player;
}

/// <summary>
/// Clears the class reference to the player.
/// </summary>
void enemyClearPlayerReference()
{
	_playerReference = NULL;
}

/// <param name="enemy"></param>
/// <returns>The enemy's type.</returns>
EnemyType enemyGetType(Enemy* enemy)
{
	return enemy->enemyType;
}

/// <summary>
/// Sets the enemyAction callback.
/// </summary>
/// <param name="cb"></param>
void enemySetEnemyActionCB(EnemyActionCB cb)
{
	_enemyActionCB = cb;
}

/// <summary>
/// Clears the enemyAction callback.
/// </summary>
void enemyClearEnemyActionCB()
{
	_enemyActionCB = NULL;
}
