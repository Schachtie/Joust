#include <math.h>

#include "player.h"
#include "animation.h"
#include "joustGlobalConstants.h"
#include "collision.h"


#define VK_X	0x58
#define VK_Z	0x5A

#define PLAYER_LIVES_MAX		6
#define PLAYER_LIVES_START		PLAYER_LIVES_MAX - 1

#define PLAYER_ANIM_WING_UP_FRAME	0
#define PLAYER_ANIM_WING_DOWN_FRAME	1
#define PLAYER_ANIM_IDLE_FRAME		3


static bool	_wasPressedLastFrame_X = false;


typedef struct player_t {
	Entity entity;

	uint8_t lives;

	const Animation* animation;
	uint8_t amCurFrame;
	uint32_t amSpeedMSIdle;
	uint32_t amSpeedMSRun;
	uint32_t amSpeedMSRunSlow;
	uint32_t amSpeedMSFly;
	uint32_t amTimerIdle;
	uint32_t amTimerRun;
	uint32_t amTimerRunSlow;
	uint32_t amTimerFly;

	bool		_currentDirection; // [false - left | true - right]
} Player;


// =============== vTable ===============
static void _playerDraw(Object* obj);
static void _playerUpdate(Object* obj, uint32_t milliseconds);
static void _playerCollide(Object* thisObj, Object* otherObj, Collision collision);
static ObjVtable _playerVtable = {
	playerDelete,
	_playerDraw,
	_playerUpdate,
	_playerCollide
};


static Animation* _animIdle = NULL;
static Animation* _animRunning = NULL;
static Animation* _animRunSlowing = NULL;
static Animation* _animFlying = NULL;


// CALLBACKS
static PlayerEnemyKilledCB _enemyKilledCB = NULL;
static PlayerEventCB _playerKilledCB = NULL;
static PlayerActionCB _playerActionCB = NULL;

static void _playerTriggerEnemyKilledCB(Object* enemy);
static void _playerTriggerPlayerKilledCB();
static void _playerTriggerPlayerActionCB(const char* action);


/// <summary>
/// Sets the internal enemyKilled class callback.
/// </summary>
/// <param name="cb"></param>
void playerSetEnemyKilledCB(PlayerEnemyKilledCB cb)
{
	_enemyKilledCB = cb;
}

/// <summary>
/// Clears the internal enemyKilled class callback.
/// </summary>
void playerClearEnemyKilledCB()
{
	_enemyKilledCB = NULL;
}


/// <summary>
/// Sets the internal playerKilled class callback.
/// </summary>
/// <param name="cb"></param>
void playerSetPlayerKilledCB(PlayerEventCB cb)
{
	_playerKilledCB = cb;
}

/// <summary>
/// Clears the internal playerKilled class callback.
/// </summary>
void playerClearPlayerKilledCB()
{
	_playerKilledCB = NULL;
}


/// <summary>
/// Sets the internal playerAction class callback.
/// </summary>
/// <param name="cb"></param>
void playerSetPlayerActionCB(PlayerActionCB cb)
{
	_playerActionCB = cb;
}

/// <summary>
/// Clears the internal playerAction class callback.
/// </summary>
void playerClearPlayerActionCB()
{
	_playerActionCB = NULL;
}


/// <summary>
/// Initializes all necessary player animations.
/// </summary>
/// <param name="sheet"></param>
void playerInitAnimations(const SpriteSheet* const sheet)
{
	_animIdle = animationNew(PLAYER_ANIMATION_IDLE_NUM_FRAMES, sheet, PLAYER_ANIMATION_IDLE_FIRST_SPRITE_BOUNDS, PLAYER_ANIMATION_IDLE_PIXELS_BETWEEN_FRAMES, 0);
	_animRunning = animationNew(PLAYER_ANIMATION_RUNNING_NUM_FRAMES, sheet, PLAYER_ANIMATION_RUNNING_FIRST_SPRITE_BOUNDS, PLAYER_ANIMATION_RUNNING_PIXELS_BETWEEN_FRAMES, 0);
	_animRunSlowing = animationNew(PLAYER_ANIMATION_RUNSLOWING_NUM_FRAMES, sheet, PLAYER_ANIMATION_RUNSLOWING_FIRST_SPRITE_BOUNDS, PLAYER_ANIMATION_RUNSLOWING_PIXELS_BETWEEN_FRAMES, 0);
	_animFlying = animationNew(PLAYER_ANIMATION_FLYING_NUM_FRAMES, sheet, PLAYER_ANIMATION_FLYING_FIRST_SPRITE_BOUNDS, PLAYER_ANIMATION_FLYING_PIXELS_BETWEEN_FRAMES, 0);
}

/// <summary>
/// Deinitializes all necessary player animations.
/// </summary>
void playerDeinitAnimations()
{
	animationDelete(_animIdle);
	_animIdle = NULL;

	animationDelete(_animRunning);
	_animRunning = NULL;

	animationDelete(_animRunSlowing);
	_animRunSlowing = NULL;

	animationDelete(_animFlying);
	_animFlying = NULL;
}


/// <summary>
/// Creates a player object.
/// </summary>
/// <param name="startPos"></param>
/// <param name="size"></param>
/// <returns></returns>
Player* playerNew(Coord2D startPos, Coord2D size)
{
	Player* player = (Player*)malloc(sizeof(Player));
	if (player != NULL)
	{
		entityInit(&player->entity, &_playerVtable, startPos, size);

		player->entity.awake = true;
		player->entity.collresp = COLLRESP_PLAYER;

		player->lives = (uint8_t)PLAYER_LIVES_START;

		player->entity.flyingSize = PLAYER_SIZE_FLYING;
		player->entity.flyingPosition = startPos;

		player->animation = _animRunning;
		player->amCurFrame = PLAYER_ANIM_IDLE_FRAME;
		player->amSpeedMSIdle = 0;
		player->amSpeedMSRun = ANIMATION_SPEED_RUN;
		player->amSpeedMSRunSlow = 0;
		player->amSpeedMSFly = ANIMATION_SPEED_FLAP;
		player->amTimerIdle = 0;
		player->amTimerRun = 0;
		player->amTimerRunSlow = 0;
		player->amTimerFly = 0;

		player->_currentDirection = true; // Player starts by facing to the right
	}
	return player;
}

/// <summary>
/// Deletes the player object.
/// </summary>
/// <param name="player"></param>
void playerDelete(Object* player)
{
	if (player != NULL)
	{
		entityDeinit(&((Player*)player)->entity);
	}
	free(player);
}


/// <summary>
/// Draws the player to the screen with their appropriate animation. Also handles screen wrapping.
/// </summary>
/// <param name="obj"></param>
static void _playerDraw(Object* obj)
{
	Player* player = (Player*)obj;

	uint32_t animationSpeed = 0;
	uint32_t* animationTimer = NULL;
	Coord2D playerSize = player->entity.obj.size;
	Coord2D playerPos = player->entity.obj.position;
	if (player->animation == _animIdle)
	{
		animationSpeed = player->amSpeedMSIdle;
		animationTimer = &player->amTimerIdle;
	}
	else if (player->animation == _animRunning)
	{
		animationSpeed = player->amSpeedMSRun;
		animationTimer = &player->amTimerRun;
	}
	else if (player->animation == _animRunSlowing)
	{
		animationSpeed = player->amSpeedMSRunSlow;
		animationTimer = &player->amTimerRunSlow;
	}
	else if (player->animation == _animFlying)
	{
		animationSpeed = player->amSpeedMSFly;
		animationTimer = &player->amTimerFly;
		playerSize = player->entity.flyingSize;
		playerPos = player->entity.flyingPosition;
	}
	assert(animationTimer != NULL);

	if (*animationTimer >= animationSpeed)
	{
		*animationTimer = 0;
		if (++player->amCurFrame >= player->animation->numFrames)
		{
			if (player->animation == _animFlying)
			{
				player->amCurFrame = PLAYER_ANIM_WING_UP_FRAME;
			}
			else
			{
				player->amCurFrame = 0;
			}
		}
	}
	animationDraw(player->animation, player->amCurFrame, playerPos, playerSize, !player->_currentDirection);

	// Check for other side of screen redraw here!!!
		// Approaching left wall
	if (playerPos.x - playerSize.x / 2 < player->entity.gameBounds.topLeft.x)
	{
		float overlap = player->entity.gameBounds.topLeft.x - (playerPos.x - playerSize.x / 2);
		Coord2D wrapDrawPos = { .x = player->entity.gameBounds.botRight.x + (playerSize.x / 2 - overlap), .y = playerPos.y };
		animationDraw(player->animation, player->amCurFrame, wrapDrawPos, playerSize, !player->_currentDirection);
	}
		// Approaching right wall
	else if (playerPos.x + playerSize.x / 2 > player->entity.gameBounds.botRight.x)
	{
		float overlap = (playerPos.x + playerSize.x / 2) - player->entity.gameBounds.botRight.x;
		Coord2D wrapDrawPos = { .x = player->entity.gameBounds.topLeft.x - (playerSize.x / 2 - overlap), .y = playerPos.y };
		animationDraw(player->animation, player->amCurFrame, wrapDrawPos, playerSize, !player->_currentDirection);
	}
}

/// <summary>
/// Update the player based on their internal state. Handles user input for moving/flapping as well as setting the appropriate animation.
/// </summary>
/// <param name="obj"></param>
/// <param name="milliseconds"></param>
static void _playerUpdate(Object* obj, uint32_t milliseconds)
{
	Player* player = (Player*)obj;

	// Left-Right input detection
	if (inputKeyPressed(VK_RIGHT) && inputKeyPressed(VK_LEFT))
	{
		// Should be empty, neither direction should be affected when both inputs are down
	}
	else if (inputKeyPressed(VK_RIGHT))
	{
		player->entity.velocity.x += ENT_DEFAULT_VELOCITY_CHANGE / ((player->entity.isGrounded) ? 1 : 2);
		if (player->entity.isGrounded == false) { player->_currentDirection = true; }
		else
		{
			if (player->_currentDirection == false && player->animation != _animRunSlowing)
			{
				player->animation = _animRunSlowing;
				player->amCurFrame = 0;
			}
			else if (player->_currentDirection == true && player->animation != _animRunning)
			{
				player->animation = _animRunning;
				player->amCurFrame = PLAYER_ANIM_IDLE_FRAME;
			}
		}
	}
	else if (inputKeyPressed(VK_LEFT))
	{
		player->entity.velocity.x -= ENT_DEFAULT_VELOCITY_CHANGE / ((player->entity.isGrounded) ? 1 : 2);
		if (player->entity.isGrounded == false) { player->_currentDirection = false; }
		else
		{
			if (player->_currentDirection == true && player->animation != _animRunSlowing)
			{
				player->animation = _animRunSlowing;
				player->amCurFrame = 0;
			}
			else if (player->_currentDirection == false && player->animation != _animRunning)
			{
				player->animation = _animRunning;
				player->amCurFrame = PLAYER_ANIM_IDLE_FRAME;
			}
		}
	}
	else
	{
		if (player->entity.isGrounded && player->animation != _animRunning)
		{
			player->animation = _animRunning;
			player->amCurFrame = PLAYER_ANIM_IDLE_FRAME;
		}
	}
	

	// Check for flapping
	if (!inputKeyPressed(VK_SPACE) && _wasPressedLastFrame_X)	// Reset the bool in order to allow another single flap
	{
		_wasPressedLastFrame_X = false;
	}
	if (inputKeyPressed(VK_SPACE) && !_wasPressedLastFrame_X)	// Singular flap
	{
		_wasPressedLastFrame_X = true;
		player->amCurFrame = PLAYER_ANIM_WING_DOWN_FRAME;
		player->entity.velocity.y -= ENT_BALANCE_FLAP_SINGLE * ENT_DEFAULT_VELOCITY_CHANGE;
		if (player->entity.isGrounded) 
		{
			player->entity.isGrounded = false;
			player->animation = _animFlying;
			player->amCurFrame = PLAYER_ANIM_WING_UP_FRAME;
			player->entity.obj.position.y -= MAGIC_NUMBER_PLATFORMFLAP * 2.0f;
		}
		_playerTriggerPlayerActionCB("flap");
	}


	// Call the parent class's update
	entityDefaultUpdate(obj, milliseconds);

	// Update the animation speed accordingly
	if (player->animation == _animRunning)
	{
		player->amTimerRun += milliseconds * (uint32_t)fabs(player->entity.velocity.x);
	}
	else if (player->animation == _animFlying && player->amCurFrame == PLAYER_ANIM_WING_DOWN_FRAME)
	{
		player->amTimerFly += milliseconds;
	}
	else if (player->animation == _animRunSlowing)
	{
		player->amTimerRunSlow += milliseconds;
	}

	// Update the player's direction appropriately (this will need to be more complex if the player is flying)
	if (player->entity.isGrounded)
	{
		if (player->entity.velocity.x > 0) { player->_currentDirection = true; }
		else if (player->entity.velocity.x < 0) { player->_currentDirection = false; }
	}
	else
	{
		// Set the player's flying position
		Coord2D playerTopLeft = { .x = player->entity.obj.position.x - player->entity.obj.size.x / 2, .y = player->entity.obj.position.y - player->entity.obj.size.y / 2 };
		Coord2D playerBotRightFlying = { .x = playerTopLeft.x + player->entity.flyingSize.x, .y = playerTopLeft.y + player->entity.flyingSize.y };
		Bounds2D playerFlyingBounds = { .topLeft = playerTopLeft, .botRight = playerBotRightFlying };
		player->entity.flyingPosition = boundsGetCenter(&playerFlyingBounds);

		// Set the animation if necessary
		if (player->animation != _animFlying)
		{
			player->animation = _animFlying;
			player->amCurFrame = 0;
		}
	}

	player->entity.isGrounded = false;
}

/// <summary>
/// Handles the player's collision responses, specifically against enemies.
/// </summary>
/// <param name="thisObj"></param>
/// <param name="otherObj"></param>
/// <param name="collision"></param>
static void _playerCollide(Object* thisObj, Object* otherObj, Collision collision)
{
	Entity* thisEntity = (Entity*)thisObj;
	Entity* otherEntity = (Entity*)otherObj;

	switch (otherEntity->collresp)
	{
		case COLLRESP_ENEMY:
		{
			// Since we might be flying, check if we need to use a smaller collision box
			if (thisEntity->isGrounded == false)
			{
				// Get the player's bounds
				Coord2D playerTopLeft = { .x = thisEntity->flyingPosition.x - thisEntity->flyingSize.x / 2, .y = thisEntity->flyingPosition.y - thisEntity->flyingSize.y / 2 };
				Coord2D playerBotRight = { .x = playerTopLeft.x + thisEntity->flyingSize.x, .y = playerTopLeft.y + thisEntity->flyingSize.y };
				Bounds2D playerBounds = { .topLeft = playerTopLeft, .botRight = playerBotRight };
			
				// Get the enemy bounds
				Bounds2D enemyBounds;
				if (otherEntity->isGrounded == false) // Flying
				{
					Coord2D enemyTopleft = { .x = otherEntity->flyingPosition.x - otherEntity->flyingSize.x / 2, .y = otherEntity->flyingPosition.y - otherEntity->flyingSize.y / 2 };
					Coord2D enemyBotRight = { .x = enemyTopleft.x + otherEntity->flyingSize.x, .y = enemyTopleft.y + otherEntity->flyingSize.y };
					enemyBounds.topLeft = enemyTopleft;
					enemyBounds.botRight = enemyBotRight;
				}
				else // Grounded
				{
					Coord2D enemyTopLeft = { .x = otherEntity->obj.position.x - otherEntity->obj.size.x / 2, .y = otherEntity->obj.position.y - otherEntity->obj.size.y / 2 };
					Coord2D enemyBotRight = { .x = enemyTopLeft.x + otherEntity->obj.size.x, .y = enemyTopLeft.y + otherEntity->obj.size.y };
					enemyBounds.topLeft = enemyTopLeft;
					enemyBounds.botRight = enemyBotRight;
				}

				// Check for collision again
				Collision doubleCheck = detectCollision(&playerBounds, &enemyBounds);
				if (doubleCheck.isColliding == false)
				{
					return;
				}
				collision = doubleCheck;
			}

			// Note that (0, 0) is in the top left

			// This check should be more accurate tbh,  but we are achieving collision detection now
			if (thisEntity->obj.position.y < otherEntity->obj.position.y)			// Player kills enemy
			{
				//consider how to adjust this to add the egg mechanic!! (just add more data to enemy class along with sprite info and an enum for what type of enemy it is)
			
				// Check whether to reverse x or y velocity (to bounce off killed enemy)
				if (collision.intersect.x < collision.intersect.y)
				{
					thisEntity->velocity.y = -(thisEntity->velocity.y);
				}
				else if (collision.intersect.x > collision.intersect.y)
				{
					thisEntity->velocity.x = -(thisEntity->velocity.x);
				}
				else
				{
					thisEntity->velocity.y = -(thisEntity->velocity.y);
					thisEntity->velocity.x = -(thisEntity->velocity.x);
				}

				otherEntity->obj.enabled = false;
				_playerTriggerEnemyKilledCB(otherObj);

			}
			else if (thisEntity->obj.position.y > otherEntity->obj.position.y)	// Enemy kills player
			{
				--((Player*)thisEntity)->lives;
				thisEntity->velocity.x = 0;
				thisEntity->velocity.y = 0;

				// Check whether to reverse x or y velocity of enemy (to bounce off killed player)
					// THIS DOESNT SEEM TO FEEL AS NICE, CHECK YOUTUBE AND TWEAK AS NECESSARY
				if (collision.intersect.x < collision.intersect.y)
				{
					otherEntity->velocity.y = -(otherEntity->velocity.y);
				}
				else if (collision.intersect.x > collision.intersect.y)
				{
					otherEntity->velocity.x = -(otherEntity->velocity.x);
				}
				else
				{
					otherEntity->velocity.y = -(otherEntity->velocity.y);
					otherEntity->velocity.x = -(otherEntity->velocity.x);
				}

				thisEntity->obj.enabled = false;
				_playerTriggerPlayerKilledCB();

			}
			else	// Player and Enemy tie!
			{

			}
			break;
		}
		case COLLRESP_PLATFORM: //should fall through into default
		{
			// Make sure we're not on the top of a platform
			if (collision.intersect.x >= collision.intersect.y || collision.delta.y <= 0.0f)
			{
				_playerTriggerPlayerActionCB("platform");
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
/// Triggers the enemyKilled callback.
/// </summary>
/// <param name="enemy"></param>
static void _playerTriggerEnemyKilledCB(Object* enemy)
{
	if (_enemyKilledCB != NULL)
	{
		_enemyKilledCB(enemy);
	}
}

/// <summary>
/// Triggers the playerKilled callback.
/// </summary>
static void _playerTriggerPlayerKilledCB()
{
	if (_playerKilledCB != NULL)
	{
		_playerKilledCB();
	}
}

/// <summary>
/// Triggers the playerAction callback.
/// </summary>
/// <param name="action"></param>
static void _playerTriggerPlayerActionCB(const char* action)
{
	if (_playerActionCB != NULL)
	{
		_playerActionCB(action);
	}
}


/// <param name="player"></param>
/// <returns>The player's underlying object.</returns>
Object* playerGetObject(Player* player)
{
	return &player->entity.obj;
}

/// <param name="player"></param>
/// <returns>Whether the player is currently grounded.</returns>
bool playerGetGroundedState(const Player* const player)
{
	return player->entity.isGrounded;
}

/// <param name="player"></param>
/// <returns>The player's current flying position.</returns>
Coord2D playerGetPositionFlying(const Player* const player)
{
	return player->entity.flyingPosition;
}

/// <param name="player"></param>
/// <returns>The player's current grounded position.</returns>
Coord2D playerGetPositionGrounded(const Player* const player)
{
	return player->entity.obj.position;
}

/// <param name="player"></param>
/// <returns>The number of lives the player currently has.</returns>
uint8_t playerGetLives(const Player* const player)
{
	return player->lives;
}

/// <summary>
/// Attempts to add a life to the player if they have not reached max lives.
/// </summary>
/// <param name="player"></param>
/// <returns>Whether a life has been added.</returns>
bool playerAddLife(Player* player)
{
	if (player->lives < PLAYER_LIVES_MAX)
	{
		++player->lives;
		return true;
	}
	return false;
}

/// <summary>
/// Resets the player's lives back to starting amount.
/// </summary>
/// <param name="player"></param>
void playerResetLives(Player* player)
{
	player->lives = PLAYER_LIVES_START;
}