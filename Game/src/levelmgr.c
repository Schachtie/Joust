#include <Windows.h>
#include <stdlib.h>
#include <gl/GLU.h>
#include <assert.h>

#include "baseTypes.h"
#include "levelmgr.h"
#include "objmgr.h"
#include "SOIL.h"
#include "sound.h"
#include "entity.h"
#include "player.h"
#include "enemy.h"
#include "background.h"
#include "collisionBox.h"
#include "joustGlobalConstants.h"
#include "numberDisplay.h"
#include "livesDisplay.h"
#include "soundOneShot.h"
#include "tools.h"


static const char TITLE_SPRITE_SHEET[] = "asset/Joust_Title_Screen.png";
static const char REMAINING_SPRITE_SHEET[] = "asset/Joust_Full_Sprite_Sheet_Bridge.png";

enum sound_e {
    SOUND_DEATH,
    SOUND_EXTRALIFE,
    SOUND_FLAP,
    SOUND_PLATFORMBOUNCE,
    SOUND_RUN,
    SOUND_SPAWN,
    SOUND_START,
    SOUND_TIE,
    SOUND_WAVESTART,

    SOUND_COUNT
};
static const char SOUND_FILES[SOUND_COUNT][50] = {
    "asset/sounds/jDeath.wav",
    "asset/sounds/jExtraLife.wav",
    "asset/sounds/jFlap.wav",
    "asset/sounds/jPlatformBounce.wav",
    "asset/sounds/jRun.wav",
    "asset/sounds/jSpawn1.wav",
    "asset/sounds/jStart.wav",
    "asset/sounds/jTie.wav",
    "asset/sounds/jWaveStart.wav"
};
static const uint32_t SOUND_LENGTHS[SOUND_COUNT] = { //in milliseconds
    500,
    1000,
    200,
    200,
    100,
    300,
    2200,
    900,
    1100
};
static SoundOneShot* _sounds[SOUND_COUNT] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static const uint8_t NUMBER_OF_COLLISION_BOXES = 8;

static const float SCALER_WORDSIZE_WIDTH = 2;
static const float SCALER_WORDSIZE_HEIGHT = 2;

static const Coord2D _DISPLAY_WAVE_LOCATION = { .x = 400, .y = 250 };
static const Coord2D _DISPLAY_WAVE_NUMBER_LOCATION = { .x = 500, .y = 250 };

static const Coord2D _DISPLAY_FLAVOR_TEXT_MIDDLE = { .x = 465, .y = 385 };
static const Coord2D _DISPLAY_FLAVOR_TEXT_BOTTOM = { .x = 0, .y = 0 };

static const uint32_t _POINTS_KILL_BOUNDER = 500;
static const uint32_t _POINTS_KILL_HUNTER = 750;
static const uint32_t _POINTS_KILL_SHADOWLORD = 1500;


static SpriteSheet* _spriteSheetTitle = NULL;
static SpriteSheet* _spriteSheetRemaining = NULL;

static Background* _titleBackground = NULL;
static Background* _waveBackground = NULL;
static Background* _hiscoresBackground = NULL;
static Coord2D _hiscoresBackgroundStartPos = { .x = 0, .y = 0 };
static Coord2D _hiscoresBackgroundEndPos = { .x = 0, .y = 0 };

static NumberDisplay* _scorePlayer1 = NULL;
static Coord2D _scorePlayer1WavePosition = { .x = 0, .y = 0 };
static Coord2D _scorePlayer1HiscorePosition = { .x = 0, .y = 0 };
static uint32_t _extraLifePointCounter = 0;
static const uint32_t _extraLifePointsThreshold = 20000;
static NumberDisplay* _waveCounter = NULL;

static const float _endScreenLerpSpeed = 3000;
static float _endScreenLerpTimer = 0;

static Player* _player = NULL;
static LivesDisplay* _playerLivesDisplay = NULL;
static const uint8_t _playerMaxLives = 6;

static CollisionBox** _collisionBoxes = NULL;

static uint8_t _numAliveEnemies; // Should start at the number of enemies in each wave
static uint8_t _numSpawnedEnemies = 0; // Should increase up to the number of enemies in the wave

static Coord2D _spawnLocations[4]; // There are 4 spawn locations
static const uint8_t _numSpawnLocations = 4;
static const uint16_t _safeSpawnRadius = 200;
static const uint32_t _spawnCycle = 2000; // This is number of milliseconds in between each potential spawn
static uint32_t _spawnTimer = 0;
static uint8_t _spawnActiveLocation = 0;

static Sprite* _wordPopups[WORDS_COUNT];
static uint32_t _popupDisplayTime = 5000;
static uint32_t _popupDisplayTimer = 0;

static bool _wasPressedLastFrame_Return = false;
static bool _canSpawnCamp = false;


typedef struct level_t
{
    const LevelDef* def;

    Background* background;

    uint8_t numEnemies;
    Enemy** enemies;
} Level;


// Function Prototypes
static void _levelMgrInitSpriteSheets();
static void _levelMgrDeinitSpriteSheets();
static void _levelMgrInitBackgrounds();
static void _levelMgrDeinitBackgrounds();
static void _levelMgrInitScore();
static void _levelMgrDeinitScore();
static void _levelMgrInitWaveCounter();
static void _levelMgrDeinitWaveCounter();
static void _levelMgrInitPlayer(); 
static void _levelMgrDeinitPlayer();
static void _levelMgrInitLivesDisplay();
static void _levelMgrDeinitLivesDisplay();
static void _levelMgrInitEnemyAnimations();
static void _levelMgrDeinitEnemyAnimations();
static void _levelMgrInitCollisionBoxes();
static void _levelMgrDeinitCollisionBoxes();
static void _levelMgrInitSpawnLocations();
static void _levelMgrDeinitSpawnLocations(); // This shouldn't actually do anything, as spawn locations are not dynamic memory, but have this and call it in case spawn locations become dynamic
static void _levelMgrInitWordPopups();
static void _levelMgrDeinitWordPopups();
static void _levelMgrInitSounds();
static void _levelMgrDeinitSounds();

static void _levelMgrSpawnEntity(Level* level, uint32_t milliseconds);

static void _levelMgrEnemyKilled(Object* enemy);
static void _levelMgrPlayerKilled(void);

static void _levelMgrPlaySound(const char* action);


/// @brief Initialize the level manager
void levelMgrInit()
{
    // Initialize all class variables
    _levelMgrInitSpriteSheets();
    _levelMgrInitBackgrounds();
    numberDisplayInit(_spriteSheetRemaining);
    _levelMgrInitScore();
    _levelMgrInitWaveCounter();
    _levelMgrInitPlayer();
    _levelMgrInitLivesDisplay();
    _levelMgrInitEnemyAnimations();
    _levelMgrInitCollisionBoxes();
    _levelMgrInitSpawnLocations();
    _levelMgrInitWordPopups();
    _levelMgrInitSounds();

    // Set the enemy class's reference to the player
    enemySetPlayerReference(_player);
   
    // Set any necessary callbacks
    playerSetEnemyKilledCB(_levelMgrEnemyKilled);
    playerSetPlayerKilledCB(_levelMgrPlayerKilled);
    playerSetPlayerActionCB(_levelMgrPlaySound);
    enemySetEnemyActionCB(_levelMgrPlaySound);
}

/// @brief Shutdown the level manager
void levelMgrShutdown()
{
    enemyClearEnemyActionCB();
    playerClearPlayerActionCB();
    playerClearPlayerKilledCB();
    playerClearEnemyKilledCB();

    enemyClearPlayerReference();

    _levelMgrDeinitSounds();
    _levelMgrDeinitWordPopups();
    _levelMgrDeinitSpawnLocations();
    _levelMgrDeinitCollisionBoxes();
    _levelMgrDeinitEnemyAnimations();
    _levelMgrDeinitLivesDisplay();
    _levelMgrDeinitPlayer();
    _levelMgrDeinitWaveCounter();
    _levelMgrDeinitScore();
    numberDisplayShutdown();
    _levelMgrDeinitBackgrounds();
    _levelMgrDeinitSpriteSheets();
}


/// @brief Loads the level and all required objects/assets
/// @param levelDef 
/// @return pointer to the loaded level
Level* levelMgrLoad(const LevelDef* levelDef)
{
    Level* level = malloc(sizeof(Level));
    if (level != NULL)
    {
        level->def = levelDef;
        
        switch (level->def->type)
        {
            case LEVELTYPE_TITLE:
            {
                // Set and enable the background
                level->background = _titleBackground;
                objEnable((Object*)level->background);

                break;
            }
            case LEVELTYPE_HISCORES: // THIS IS PROBABLY NOT GOING TO BE FINISHED IN TIME -> just display user's score
            {
                // Set and enable the background
                level->background = _hiscoresBackground;
                level->background->obj.position = _hiscoresBackgroundStartPos;
                objEnable((Object*)level->background);

                // Reset the lerp timer
                _endScreenLerpTimer = 0;

                break;
            }
            case LEVELTYPE_WAVE:
            case LEVELTYPE_WAVE_ENDLESS:
            {
                // Increment the wave counter
                objEnable((Object*)_waveCounter);
                _waveCounter->numberToDisplay++;

                // Play necessary start sound and reset spawn location
                if (_waveCounter->numberToDisplay == 1)
                {
                    soundOneShotPlayIsolated(_sounds[SOUND_START], true);
                    _spawnActiveLocation = 0;
                }
                else
                {
                    soundOneShotPlayIsolated(_sounds[SOUND_WAVESTART], true);
                }
            
                // Reset any necessary timers
                _popupDisplayTimer = 0;
                _spawnTimer = 0;

                // Set and enable the background, score, lives display
                level->background = _waveBackground;
                objEnable((Object*)level->background);
                numberDisplayChangePosition(_scorePlayer1, _scorePlayer1WavePosition);
                objEnable((Object*)_scorePlayer1);
                objEnable((Object*)_playerLivesDisplay);

                // Enable all platform collision
                for (uint8_t i = 0; i < NUMBER_OF_COLLISION_BOXES; ++i)
                {
                    objEnable((Object*)_collisionBoxes[i]);
                }

                // Create all the enemies based on the level definition (disable all of them, spawning function will enable them)
                level->numEnemies = levelDef->numBounders + level->def->numHunters + level->def->numShadowLords;
                _numAliveEnemies = level->numEnemies;
                _numSpawnedEnemies = 0;
                level->enemies = (Enemy**)malloc(sizeof(Enemy*) * level->numEnemies);
                assert(level->enemies != NULL);
                    // Bounders
                uint8_t enemyIndex = 0;
                for (uint8_t i = 0; i < levelDef->numBounders; ++i, ++enemyIndex)
                {
                    level->enemies[enemyIndex] = enemyNew(SPAWN_LOCATIONS[enemyIndex % NUMBER_SPAWN_LOCATIONS], ENEMY_SIZE_GROUNDED, ENEMYTYPE_BOUNDER);
                    objDisable((Object*)level->enemies[enemyIndex]);
                }
                    // Hunters
                for (uint8_t i = 0; i < levelDef->numHunters; ++i, ++enemyIndex)
                {
                    level->enemies[enemyIndex] = enemyNew(SPAWN_LOCATIONS[enemyIndex % NUMBER_SPAWN_LOCATIONS], ENEMY_SIZE_GROUNDED, ENEMYTYPE_HUNTER);
                    objDisable((Object*)level->enemies[enemyIndex]);
                }
                    // Shadow Lords
                for (uint8_t i = 0; i < levelDef->numShadowLords; ++i, ++enemyIndex)
                {
                    level->enemies[enemyIndex] = enemyNew(SPAWN_LOCATIONS[enemyIndex % NUMBER_SPAWN_LOCATIONS], ENEMY_SIZE_GROUNDED, ENEMYTYPE_SHADOWLORD);
                    objDisable((Object*)level->enemies[enemyIndex]);
                }

                break;
            }
            default:
            {
                break;
            }
        }
    }
    return level;
}

/// @brief Unloads the level and frees up any assets associated
/// @param level 
void levelMgrUnload(Level* level)
{
    if (level != NULL) 
    {
        // Turn off the associated background object
        objDisable((Object*)level->background);

        // Delete all enemies if necessary
        if (level->def->type == LEVELTYPE_WAVE || level->def->type == LEVELTYPE_WAVE_ENDLESS)
        {
            for (uint8_t i = 0; i < level->numEnemies; ++i)
            {
                enemyDelete((Object*)level->enemies[i]);
            }
            free(level->enemies);
        }
    }
    free(level);
}


/// <summary>
/// Draws all necessary images/UI that are not objects or that do not have their own draw function.
/// </summary>
/// <param name="level"></param>
void levelMgrDraw(Level* level)
{
    switch (level->def->type)
    {
        case LEVELTYPE_WAVE:
        case LEVELTYPE_WAVE_ENDLESS:
        {
            if (_popupDisplayTimer <= _popupDisplayTime)
            {
                // Draw the wave popup
                spriteDraw(_wordPopups[WORDS_WAVE], _DISPLAY_WAVE_LOCATION, WORDS_WAVE_SIZE, false);

                // Draw any flavor text here as necessary
                if (_waveCounter->numberToDisplay == 1)
                {
                    spriteDraw(_wordPopups[WORDS_PREPARE], _DISPLAY_FLAVOR_TEXT_MIDDLE, WORDS_PREPARE_SIZE, false);
                }
            }
            else
            {
                // Disable the number for the wave
                objDisable((Object*)_waveCounter);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

/// <summary>
/// Handles moving between different levels, taking in user input during each level, logic to move between levels based on the current level.
/// </summary>
/// <param name="level"></param>
/// <param name="milliseconds"></param>
/// <returns>The outcome of the level update</returns>
LUO levelMgrUpdate(Level* level, uint32_t milliseconds)
{
    soundOneShotUpdateInternalFields(milliseconds);

    // Reset input latching if necessary
    if (!inputKeyPressed(VK_RETURN) && _wasPressedLastFrame_Return)
    {
        _wasPressedLastFrame_Return = false;
    }

    switch (level->def->type)
    {
        case LEVELTYPE_TITLE:
        {
            // PLAYER INPUT TO MOVE FROM TITLE INTO GAME
            if (inputKeyPressed(VK_RETURN) && !_wasPressedLastFrame_Return)
            {
                _wasPressedLastFrame_Return = true;

                // Reset the player's lives and score, and the wave counter
                playerResetLives(_player);
                _playerLivesDisplay->numSpritesToDisplay = playerGetLives(_player);
                _scorePlayer1->numberToDisplay = 0;
                _extraLifePointCounter = 0;
                _waveCounter->numberToDisplay = 0;
                return LUO_STARTWAVES;
            }

            break;
        }
        case LEVELTYPE_HISCORES:
        {
            // Lerp in the score and end game background
            _endScreenLerpTimer += milliseconds;
            Coord2D scoreNewPosition = toolLerp(_scorePlayer1WavePosition, _scorePlayer1HiscorePosition, toolClampFloat(_endScreenLerpTimer / _endScreenLerpSpeed, 0, 1));
            numberDisplayChangePosition(_scorePlayer1, scoreNewPosition);
            level->background->obj.position = toolLerp(_hiscoresBackgroundStartPos, _hiscoresBackgroundEndPos, toolClampFloat(_endScreenLerpTimer / _endScreenLerpSpeed, 0, 1));

            // Move to title screen
            if (inputKeyPressed(VK_RETURN) && !_wasPressedLastFrame_Return)
            {
                _wasPressedLastFrame_Return = true;

                // Turn off the score
                objDisable((Object*)_scorePlayer1);
                return LUO_TITLE;
            }

            break;
        }
        case LEVELTYPE_WAVE:
        case LEVELTYPE_WAVE_ENDLESS:
        {
            // Update the popup timer
            _popupDisplayTimer += milliseconds;

            // Check for the end of the wave (all enemies are dead)
            if (_numAliveEnemies == 0)
            {
                return LUO_NEXTWAVE;
            }
            
            // Check if the player is out of lives
            if (playerGetLives(_player) == 0)
            {
                return LUO_HISCORES;
            }

            // Check for any necessary entity spawning
            _levelMgrSpawnEntity(level, milliseconds);

            break;
        }
        default:
        {
            break;
        }
    }

    return LUO_CONTINUE;
}


/// <param name="level"></param>
/// <returns>The level's type</returns>
LevelType levelGetType(const Level* const level)
{
    return level->def->type;
}


static void _levelMgrInitSpriteSheets()
{
    GLuint titleHandle = SOIL_load_OGL_texture(TITLE_SPRITE_SHEET, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
    _spriteSheetTitle = (SpriteSheet*)malloc(sizeof(SpriteSheet));
    assert(_spriteSheetTitle != NULL);
    _spriteSheetTitle->textureHandle = titleHandle;
    _spriteSheetTitle->WIDTH_PIXELS = 302;
    _spriteSheetTitle->HEIGHT_PIXELS = 255;
        // Nearest neighbor scaling: THANK YOU JOSH!
    glBindTexture(GL_TEXTURE_2D, titleHandle);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLuint remainingHandle = SOIL_load_OGL_texture(REMAINING_SPRITE_SHEET, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
    _spriteSheetRemaining = (SpriteSheet*)malloc(sizeof(SpriteSheet));
    assert(_spriteSheetRemaining != NULL);
    _spriteSheetRemaining->textureHandle = remainingHandle;
    _spriteSheetRemaining->WIDTH_PIXELS = 1353;
    _spriteSheetRemaining->HEIGHT_PIXELS = 1269;
        // Nearest neighbor scaling
    glBindTexture(GL_TEXTURE_2D, remainingHandle);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

static void _levelMgrDeinitSpriteSheets()
{
    free(_spriteSheetRemaining);
    free(_spriteSheetTitle);
}

static void _levelMgrInitBackgrounds()
{
    // Title
    _titleBackground = backgroundNew(_spriteSheetTitle, BACKGROUND_TITLE_SPRITE_BOUNDS, BACKGROUND_TITLE_SIZE);
    _titleBackground->obj.size = SCREEN_RESOLUTION;
    objDisable((Object*)_titleBackground);

    // Wave
    _waveBackground = backgroundNew(_spriteSheetRemaining, BACKGROUND_WAVES_SPRITE_BOUNDS, BACKGROUND_WAVES_SIZE);
    _waveBackground->obj.size = SCREEN_RESOLUTION;
    objDisable((Object*)_waveBackground);

    // Hiscores
    _hiscoresBackground = backgroundNew(_spriteSheetRemaining, BACKGROUND_HISCORES_SPRITE_BOUNDS, BACKGROUND_HISCORES_SIZE);
    _hiscoresBackgroundStartPos = _hiscoresBackground->obj.position;
    _hiscoresBackgroundEndPos = _hiscoresBackground->obj.position;
    _hiscoresBackgroundStartPos.y = 0 - _hiscoresBackground->obj.size.y;
    _hiscoresBackgroundEndPos.y = 0 + _hiscoresBackground->obj.size.y;
    objDisable((Object*)_hiscoresBackground);
}

static void _levelMgrDeinitBackgrounds()
{
    if (_titleBackground != NULL) { backgroundDelete((Object*)_titleBackground); }
    if (_waveBackground != NULL) { backgroundDelete((Object*)_waveBackground); }
    if (_hiscoresBackground != NULL) { backgroundDelete((Object*)_hiscoresBackground); }
}

static void _levelMgrInitScore()
{
    Coord2D scoreBoardTL = { .x = (SCOREBOARD_TL.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x, .y = (SCOREBOARD_TL.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y };
    Coord2D scoreBoardBR = { .x = (SCOREBOARD_BR.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x, .y = (SCOREBOARD_BR.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y };
    Coord2D scoreBoardSize = { .x = scoreBoardBR.x - scoreBoardTL.x, .y = scoreBoardBR.y - scoreBoardTL.y };

    _scorePlayer1 = numberDisplayNew(NUMBERCOLOR_YELLOW, scoreBoardTL, scoreBoardSize, 7, 6); // 7 - digits (millions), 4 - pixelsBetweenNumbers
    _scorePlayer1WavePosition = _scorePlayer1->obj.position;
    _scorePlayer1HiscorePosition = boundsGetCenter(&SCREEN_BOUNDS);
    _scorePlayer1HiscorePosition.x -= scoreBoardSize.x / 2;

    objDisable((Object*)_scorePlayer1);
}

static void _levelMgrDeinitScore()
{
    numberDisplayDelete((Object*)_scorePlayer1);
    
}

static void _levelMgrInitWaveCounter()
{
    // THESE WILL NEED TO BE ADJUSTED WHEN MOVING TO A LARGER SCREEN
    Coord2D topLeft = { .x = 475, .y = 238 };
    Coord2D size = { .x = 80, .y = 24 };
    _waveCounter = numberDisplayNew(NUMBERCOLOR_WHITE, topLeft, size, 3, 6);
    objDisable((Object*)_waveCounter);
    //_waveCounter->numberToDisplay = 999;
}

static void _levelMgrDeinitWaveCounter()
{
    numberDisplayDelete((Object*)_waveCounter);
}

static void _levelMgrInitPlayer()
{
    playerInitAnimations(_spriteSheetRemaining);

    // MOVE THESE TO THE GLOBAL CONST FILE!
    Coord2D playerStart = { .x = 0.0f, .y = 0.0f };
    _player = playerNew(playerStart, PLAYER_SIZE_GROUNDED);
    objDisable((Object*)_player);
}

static void _levelMgrDeinitPlayer()
{
    playerDelete((Object*)_player);
    
    playerDeinitAnimations();
}

static void _levelMgrInitLivesDisplay()
{
    livesDisplayInit(_spriteSheetRemaining);

    Coord2D livesDisplayTL = { .x = (LIVES_DISPLAY_TL.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x, .y = (LIVES_DISPLAY_TL.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y };
    Coord2D livesDisplayBR = { .x = (LIVES_DISPLAY_BR.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x, .y = (LIVES_DISPLAY_BR.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y };
    Coord2D livesDisplaySize = { .x = livesDisplayBR.x - livesDisplayTL.x, .y = livesDisplayBR.y - livesDisplayTL.y };

    _playerLivesDisplay = livesDisplayNew(livesDisplayTL, livesDisplaySize, _playerMaxLives - 1, 6); //6 - pixels between lives
    objDisable((Object*)_playerLivesDisplay);
}

static void _levelMgrDeinitLivesDisplay()
{
    livesDisplayDelete((Object*)_playerLivesDisplay);

    livesDisplayShutdown();
}

static void _levelMgrInitEnemyAnimations()
{
    enemyInitAnimations(_spriteSheetRemaining);
}

static void _levelMgrDeinitEnemyAnimations()
{
    enemyDeinitAnimations();
}

static void _levelMgrInitCollisionBoxes()
{
    _collisionBoxes = (CollisionBox**)malloc(sizeof(CollisionBox*) * NUMBER_OF_COLLISION_BOXES);
    assert(_collisionBoxes != NULL);

    // Create all platform collision boxes
            // Bottom platform
    Coord2D collisionBoxTL = { .x = (COLLBOX_BOT_TL.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x, .y = (COLLBOX_BOT_TL.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y };
    Coord2D collisionBoxBR = { .x = (COLLBOX_BOT_BR.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x, .y = (COLLBOX_BOT_BR.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y };
    Coord2D collisionBoxSize = { .x = collisionBoxBR.x - collisionBoxTL.x, .y = collisionBoxBR.y - collisionBoxTL.y };
    _collisionBoxes[0] = collisionBoxNew(collisionBoxTL, collisionBoxSize);
        // Middle Left (wrap)
    collisionBoxTL.x = (COLLBOX_MIDLEFT_TL.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    collisionBoxTL.y = (COLLBOX_MIDLEFT_TL.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    collisionBoxBR.x = (COLLBOX_MIDLEFT_BR.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    collisionBoxBR.y = (COLLBOX_MIDLEFT_BR.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    collisionBoxSize.x = collisionBoxBR.x - collisionBoxTL.x;
    collisionBoxSize.y = collisionBoxBR.y - collisionBoxTL.y;
    _collisionBoxes[1] = collisionBoxNew(collisionBoxTL, collisionBoxSize);
        // Middle
    collisionBoxTL.x = (COLLBOX_MID_TL.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    collisionBoxTL.y = (COLLBOX_MID_TL.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    collisionBoxBR.x = (COLLBOX_MID_BR.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    collisionBoxBR.y = (COLLBOX_MID_BR.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    collisionBoxSize.x = collisionBoxBR.x - collisionBoxTL.x;
    collisionBoxSize.y = collisionBoxBR.y - collisionBoxTL.y;
    _collisionBoxes[2] = collisionBoxNew(collisionBoxTL, collisionBoxSize);
        // Middle Right
    collisionBoxTL.x = (COLLBOX_MIDRIGHT_TL.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    collisionBoxTL.y = (COLLBOX_MIDRIGHT_TL.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    collisionBoxBR.x = (COLLBOX_MIDRIGHT_BR.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    collisionBoxBR.y = (COLLBOX_MIDRIGHT_BR.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    collisionBoxSize.x = collisionBoxBR.x - collisionBoxTL.x;
    collisionBoxSize.y = collisionBoxBR.y - collisionBoxTL.y;
    _collisionBoxes[3] = collisionBoxNew(collisionBoxTL, collisionBoxSize);
        // Middle Right (wrap)
    collisionBoxTL.x = (COLLBOX_MIDRIGHT_WRAP_TL.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    collisionBoxTL.y = (COLLBOX_MIDRIGHT_WRAP_TL.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    collisionBoxBR.x = (COLLBOX_MIDRIGHT_WRAP_BR.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    collisionBoxBR.y = (COLLBOX_MIDRIGHT_WRAP_BR.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    collisionBoxSize.x = collisionBoxBR.x - collisionBoxTL.x;
    collisionBoxSize.y = collisionBoxBR.y - collisionBoxTL.y;
    _collisionBoxes[4] = collisionBoxNew(collisionBoxTL, collisionBoxSize);
        // Top Left (wrap)
    collisionBoxTL.x = (COLLBOX_TOPLEFT_TL.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    collisionBoxTL.y = (COLLBOX_TOPLEFT_TL.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    collisionBoxBR.x = (COLLBOX_TOPLEFT_BR.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    collisionBoxBR.y = (COLLBOX_TOPLEFT_BR.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    collisionBoxSize.x = collisionBoxBR.x - collisionBoxTL.x;
    collisionBoxSize.y = collisionBoxBR.y - collisionBoxTL.y;
    _collisionBoxes[5] = collisionBoxNew(collisionBoxTL, collisionBoxSize);
        // Top Middle
    collisionBoxTL.x = (COLLBOX_TOPMID_TL.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    collisionBoxTL.y = (COLLBOX_TOPMID_TL.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    collisionBoxBR.x = (COLLBOX_TOPMID_BR.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    collisionBoxBR.y = (COLLBOX_TOPMID_BR.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    collisionBoxSize.x = collisionBoxBR.x - collisionBoxTL.x;
    collisionBoxSize.y = collisionBoxBR.y - collisionBoxTL.y;
    _collisionBoxes[6] = collisionBoxNew(collisionBoxTL, collisionBoxSize);
        // Top Right (wrap)
    collisionBoxTL.x = (COLLBOX_TOPRIGHT_TL.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    collisionBoxTL.y = (COLLBOX_TOPRIGHT_TL.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    collisionBoxBR.x = (COLLBOX_TOPRIGHT_BR.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    collisionBoxBR.y = (COLLBOX_TOPRIGHT_BR.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    collisionBoxSize.x = collisionBoxBR.x - collisionBoxTL.x;
    collisionBoxSize.y = collisionBoxBR.y - collisionBoxTL.y;
    _collisionBoxes[7] = collisionBoxNew(collisionBoxTL, collisionBoxSize);


    // Disable all collision boxes
    for (uint8_t i = 0; i < NUMBER_OF_COLLISION_BOXES; ++i)
    {
        objDisable((Object*)_collisionBoxes[i]);
    }
}

static void _levelMgrDeinitCollisionBoxes()
{
    for (uint8_t i = 0; i < NUMBER_OF_COLLISION_BOXES; ++i)
    {
        collisionBoxDelete((Object*)_collisionBoxes[i]);
    }
    free(_collisionBoxes);
}

static void _levelMgrInitSpawnLocations()
{
    // Spawn locations must be adjusted based on the screen size
    Bounds2D tempBounds;

    tempBounds.topLeft.x = (SPAWN_BOTTOM_LOCATION.topLeft.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    tempBounds.topLeft.y = (SPAWN_BOTTOM_LOCATION.topLeft.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    tempBounds.botRight.x = (SPAWN_BOTTOM_LOCATION.botRight.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    tempBounds.botRight.y = (SPAWN_BOTTOM_LOCATION.botRight.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    _spawnLocations[0] = boundsGetCenter(&tempBounds);

    tempBounds.topLeft.x = (SPAWN_LEFT_LOCATION.topLeft.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    tempBounds.topLeft.y = (SPAWN_LEFT_LOCATION.topLeft.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    tempBounds.botRight.x = (SPAWN_LEFT_LOCATION.botRight.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    tempBounds.botRight.y = (SPAWN_LEFT_LOCATION.botRight.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    _spawnLocations[1] = boundsGetCenter(&tempBounds);

    tempBounds.topLeft.x = (SPAWN_RIGHT_LOCATION.topLeft.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    tempBounds.topLeft.y = (SPAWN_RIGHT_LOCATION.topLeft.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    tempBounds.botRight.x = (SPAWN_RIGHT_LOCATION.botRight.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    tempBounds.botRight.y = (SPAWN_RIGHT_LOCATION.botRight.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    _spawnLocations[2] = boundsGetCenter(&tempBounds);

    tempBounds.topLeft.x = (SPAWN_TOP_LOCATION.topLeft.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    tempBounds.topLeft.y = (SPAWN_TOP_LOCATION.topLeft.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    tempBounds.botRight.x = (SPAWN_TOP_LOCATION.botRight.x / BACKGROUND_WAVES_SIZE.x) * SCREEN_RESOLUTION.x;
    tempBounds.botRight.y = (SPAWN_TOP_LOCATION.botRight.y / BACKGROUND_WAVES_SIZE.y) * SCREEN_RESOLUTION.y;
    _spawnLocations[3] = boundsGetCenter(&tempBounds);
}

static void _levelMgrDeinitSpawnLocations()
{

}

static void _levelMgrInitWordPopups()
{
    // Calculate each sprite's UVs, and their respective size based on the starting screen size
    Bounds2D spriteUV;

    spriteUV.topLeft.x = WORDS_SPRITE_WAVE.topLeft.x / _spriteSheetRemaining->WIDTH_PIXELS;
    spriteUV.topLeft.y = WORDS_SPRITE_WAVE.topLeft.y / _spriteSheetRemaining->HEIGHT_PIXELS;
    spriteUV.botRight.x = WORDS_SPRITE_WAVE.botRight.x / _spriteSheetRemaining->WIDTH_PIXELS;
    spriteUV.botRight.y = WORDS_SPRITE_WAVE.botRight.y / _spriteSheetRemaining->HEIGHT_PIXELS;
    _wordPopups[WORDS_WAVE] = spriteNew(_spriteSheetRemaining, spriteUV, 0.99f);
    WORDS_WAVE_SIZE.x = (spriteUV.botRight.x - spriteUV.topLeft.x) * SCREEN_RESOLUTION.x * SCALER_WORDSIZE_WIDTH;
    WORDS_WAVE_SIZE.y = (spriteUV.topLeft.y - spriteUV.botRight.y) * SCREEN_RESOLUTION.y * SCALER_WORDSIZE_HEIGHT;

    spriteUV.topLeft.x = WORDS_SPRITE_PREPARE.topLeft.x / _spriteSheetRemaining->WIDTH_PIXELS;
    spriteUV.topLeft.y = WORDS_SPRITE_PREPARE.topLeft.y / _spriteSheetRemaining->HEIGHT_PIXELS;
    spriteUV.botRight.x = WORDS_SPRITE_PREPARE.botRight.x / _spriteSheetRemaining->WIDTH_PIXELS;
    spriteUV.botRight.y = WORDS_SPRITE_PREPARE.botRight.y / _spriteSheetRemaining->HEIGHT_PIXELS;
    _wordPopups[WORDS_PREPARE] = spriteNew(_spriteSheetRemaining, spriteUV, 0.99f);
    WORDS_PREPARE_SIZE.x = (spriteUV.botRight.x - spriteUV.topLeft.x) * SCREEN_RESOLUTION.x * SCALER_WORDSIZE_WIDTH;
    WORDS_PREPARE_SIZE.y = (spriteUV.topLeft.y - spriteUV.botRight.y) * SCREEN_RESOLUTION.y * SCALER_WORDSIZE_HEIGHT;

    spriteUV.topLeft.x = WORDS_SPRITE_GAMEOVER.topLeft.x / _spriteSheetRemaining->WIDTH_PIXELS;
    spriteUV.topLeft.y = WORDS_SPRITE_GAMEOVER.topLeft.y / _spriteSheetRemaining->HEIGHT_PIXELS;
    spriteUV.botRight.x = WORDS_SPRITE_GAMEOVER.botRight.x / _spriteSheetRemaining->WIDTH_PIXELS;
    spriteUV.botRight.y = WORDS_SPRITE_GAMEOVER.botRight.y / _spriteSheetRemaining->HEIGHT_PIXELS;
    _wordPopups[WORDS_GAMEOVER] = spriteNew(_spriteSheetRemaining, spriteUV, 0.99f);
    WORDS_GAMEOVER_SIZE.x = (spriteUV.botRight.x - spriteUV.topLeft.x) * SCREEN_RESOLUTION.x * SCALER_WORDSIZE_WIDTH;
    WORDS_GAMEOVER_SIZE.y = (spriteUV.topLeft.y - spriteUV.botRight.y) * SCREEN_RESOLUTION.y * SCALER_WORDSIZE_HEIGHT;

    spriteUV.topLeft.x = WORDS_SPRITE_BUZZARD.topLeft.x / _spriteSheetRemaining->WIDTH_PIXELS;
    spriteUV.topLeft.y = WORDS_SPRITE_BUZZARD.topLeft.y / _spriteSheetRemaining->HEIGHT_PIXELS;
    spriteUV.botRight.x = WORDS_SPRITE_BUZZARD.botRight.x / _spriteSheetRemaining->WIDTH_PIXELS;
    spriteUV.botRight.y = WORDS_SPRITE_BUZZARD.botRight.y / _spriteSheetRemaining->HEIGHT_PIXELS;
    _wordPopups[WORDS_BUZZARD] = spriteNew(_spriteSheetRemaining, spriteUV, 0.99f);
    WORDS_BUZZARD_SIZE.x = (spriteUV.botRight.x - spriteUV.topLeft.x) * SCREEN_RESOLUTION.x * SCALER_WORDSIZE_WIDTH;
    WORDS_BUZZARD_SIZE.y = (spriteUV.topLeft.y - spriteUV.botRight.y) * SCREEN_RESOLUTION.y * SCALER_WORDSIZE_HEIGHT;

    spriteUV.topLeft.x = WORDS_SPRITE_SURVIVAL.topLeft.x / _spriteSheetRemaining->WIDTH_PIXELS;
    spriteUV.topLeft.y = WORDS_SPRITE_SURVIVAL.topLeft.y / _spriteSheetRemaining->HEIGHT_PIXELS;
    spriteUV.botRight.x = WORDS_SPRITE_SURVIVAL.botRight.x / _spriteSheetRemaining->WIDTH_PIXELS;
    spriteUV.botRight.y = WORDS_SPRITE_SURVIVAL.botRight.y / _spriteSheetRemaining->HEIGHT_PIXELS;
    _wordPopups[WORDS_SURVIVAL] = spriteNew(_spriteSheetRemaining, spriteUV, 0.99f);
    WORDS_SURVIVAL_SIZE.x = (spriteUV.botRight.x - spriteUV.topLeft.x) * SCREEN_RESOLUTION.x * SCALER_WORDSIZE_WIDTH;
    WORDS_SURVIVAL_SIZE.y = (spriteUV.topLeft.y - spriteUV.botRight.y) * SCREEN_RESOLUTION.y * SCALER_WORDSIZE_HEIGHT;

    spriteUV.topLeft.x = WORDS_SPRITE_EGGWAVE.topLeft.x / _spriteSheetRemaining->WIDTH_PIXELS;
    spriteUV.topLeft.y = WORDS_SPRITE_EGGWAVE.topLeft.y / _spriteSheetRemaining->HEIGHT_PIXELS;
    spriteUV.botRight.x = WORDS_SPRITE_EGGWAVE.botRight.x / _spriteSheetRemaining->WIDTH_PIXELS;
    spriteUV.botRight.y = WORDS_SPRITE_EGGWAVE.botRight.y / _spriteSheetRemaining->HEIGHT_PIXELS;
    _wordPopups[WORDS_EGGWAVE] = spriteNew(_spriteSheetRemaining, spriteUV, 0.99f);
    WORDS_EGGWAVE_SIZE.x = (spriteUV.botRight.x - spriteUV.topLeft.x) * SCREEN_RESOLUTION.x * SCALER_WORDSIZE_WIDTH;
    WORDS_EGGWAVE_SIZE.y = (spriteUV.topLeft.y - spriteUV.botRight.y) * SCREEN_RESOLUTION.y * SCALER_WORDSIZE_HEIGHT;

    spriteUV.topLeft.x = WORDS_SPRITE_BEWARE.topLeft.x / _spriteSheetRemaining->WIDTH_PIXELS;
    spriteUV.topLeft.y = WORDS_SPRITE_BEWARE.topLeft.y / _spriteSheetRemaining->HEIGHT_PIXELS;
    spriteUV.botRight.x = WORDS_SPRITE_BEWARE.botRight.x / _spriteSheetRemaining->WIDTH_PIXELS;
    spriteUV.botRight.y = WORDS_SPRITE_BEWARE.botRight.y / _spriteSheetRemaining->HEIGHT_PIXELS;
    _wordPopups[WORDS_BEWARE] = spriteNew(_spriteSheetRemaining, spriteUV, 0.99f);
    WORDS_BEWARE_SIZE.x = (spriteUV.botRight.x - spriteUV.topLeft.x) * SCREEN_RESOLUTION.x * SCALER_WORDSIZE_WIDTH;
    WORDS_BEWARE_SIZE.y = (spriteUV.topLeft.y - spriteUV.botRight.y) * SCREEN_RESOLUTION.y * SCALER_WORDSIZE_HEIGHT;
}

static void _levelMgrDeinitWordPopups()
{
    for (uint8_t i = 0; i < WORDS_COUNT; ++i)
    {
        spriteDelete(_wordPopups[i]);
    }
}

static void _levelMgrInitSounds()
{
    for (uint8_t i = 0; i < SOUND_COUNT; ++i)
    {
        _sounds[i] = soundOneShotNew(SOUND_FILES[i], SOUND_LENGTHS[i]);
    }
}

static void _levelMgrDeinitSounds()
{
    for (uint8_t i = 0; i < SOUND_COUNT; ++i)
    {
        soundOneShotDelete(_sounds[i]);
    }
}


/// <summary>
/// Spawns the player or an enemy at one of the spawn locations at a specific time interval.
/// </summary>
/// <param name="level"></param>
/// <param name="milliseconds"></param>
static void _levelMgrSpawnEntity(Level* level, uint32_t milliseconds)
{
    assert(_player != NULL);

    // Check if it is time to spawn an entity
    _spawnTimer += milliseconds;
    if (_spawnTimer >= _spawnCycle)
    {
        // Check if player needs to spawn (check if player is disabled & has at least 1 life)
        if (!objIsEnabled((Object*)_player) && playerGetLives(_player) > 0)
        {
            // Avoid any spawn camping if the feature is disabled
            if (_canSpawnCamp == false)
            {
                for (uint8_t i = 0; i <= _numSpawnLocations; ++i)
                {
                    if (i == _numSpawnLocations) { return; }

                    // Check if there are any enemies near the active spawn location
                    bool isOpenSpawn = true;
                    for (uint8_t i = 0; i < level->numEnemies; ++i)
                    {
                        Object* enemy = (Object*)level->enemies[i];
                        if (objIsEnabled(enemy) && toolDistance(_spawnLocations[_spawnActiveLocation], enemy->position) <= _safeSpawnRadius)
                        {
                            isOpenSpawn = false;
                            break;
                        }
                    }
                    if (isOpenSpawn) { break; }
                    if (++_spawnActiveLocation >= _numSpawnLocations) { _spawnActiveLocation = 0; }
                }
            }

            // Spawn player
            ((Object*)_player)->position.x = _spawnLocations[_spawnActiveLocation].x;
            ((Object*)_player)->position.y = _spawnLocations[_spawnActiveLocation].y - ((Object*)_player)->size.y / 2;
            objEnable((Object*)_player);

            // Reduce the lives counter by one
            --_playerLivesDisplay->numSpritesToDisplay;

            // Sfx
            soundOneShotPlayIsolated(_sounds[SOUND_SPAWN], true);

            // Move to the next spawn location
            _spawnTimer = 0;
            if (++_spawnActiveLocation >= _numSpawnLocations) { _spawnActiveLocation = 0; }
            return;
        }

        // Check if an enemy needs to spawn (the number of spawned enemies < number of enemies in wave)
        if (_numSpawnedEnemies < level->numEnemies)
        {
            // Avoid any spawn camping if the feature is disabled
            if (_canSpawnCamp == false)
            {
                for (uint8_t i = 0; i <= _numSpawnLocations; ++i)
                {
                    if (i == _numSpawnLocations) { return; }

                    // Check if there are any players near the active spawn location
                    bool isOpenSpawn = true;

                    if (objIsEnabled((Object*)_player) && toolDistance(_spawnLocations[_spawnActiveLocation], ((Object*)_player)->position) <= _safeSpawnRadius)
                    {
                        isOpenSpawn = false;
                    }
                    if (isOpenSpawn) { break; }
                    if (++_spawnActiveLocation >= _numSpawnLocations) { _spawnActiveLocation = 0; }
                }
            }


            Enemy* enemy = level->enemies[_numSpawnedEnemies];
            ((Object*)enemy)->position.x = _spawnLocations[_spawnActiveLocation].x;
            ((Object*)enemy)->position.y = _spawnLocations[_spawnActiveLocation].y - ((Object*)enemy)->size.y / 2;
            objEnable((Object*)enemy);

            soundOneShotPlayIsolated(_sounds[SOUND_SPAWN], false);

            _spawnTimer = 0;
            if (++_spawnActiveLocation >= _numSpawnLocations) { _spawnActiveLocation = 0; }
            ++_numSpawnedEnemies;
            return;
        }
    }
}


/// <summary>
/// Decrements the number of alive enemies, increases the player's points based on the enemy type, and gives the player an extra life if they cross a specific point threshold.
/// </summary>
/// <param name="enemy"></param>
static void _levelMgrEnemyKilled(Object* enemy)
{
    --_numAliveEnemies;

    soundOneShotPlayIsolated(_sounds[SOUND_DEATH], false);
    
    // Points should be allocated based on the enemy type
    uint32_t pointsGained = 0;
    switch (enemyGetType((Enemy*)enemy))
    {
        case ENEMYTYPE_BOUNDER:
        {
            pointsGained = _POINTS_KILL_BOUNDER;
            break;
        }
        case ENEMYTYPE_HUNTER:
        {
            pointsGained = _POINTS_KILL_HUNTER;
            break;
        }
        case ENEMYTYPE_SHADOWLORD:
        {
            pointsGained = _POINTS_KILL_SHADOWLORD;
            break;
        }
        default:
        {

            break;
        }
    }

    // Give the player points, and if necessary an extra life
    _scorePlayer1->numberToDisplay += pointsGained;
    _extraLifePointCounter += pointsGained;
    if (_extraLifePointCounter >= _extraLifePointsThreshold)
    {
        _extraLifePointCounter -= _extraLifePointsThreshold;
        if (playerAddLife(_player))
        {
            ++_playerLivesDisplay->numSpritesToDisplay;
            soundOneShotPlayIsolated(_sounds[SOUND_EXTRALIFE], true);
        }
    }
}

/// <summary>
/// Decrements the player's life counter.
/// </summary>
/// <param name=""></param>
static void _levelMgrPlayerKilled(void)
{
    _spawnTimer = 0;

    soundOneShotPlayIsolated(_sounds[SOUND_DEATH], true);
}


/// <summary>
/// Plays an appropriate sound if the passed in action was "flap" or "platform".
/// </summary>
/// <param name="action"></param>
static void _levelMgrPlaySound(const char* action)
{
    if (_stricmp(action, "flap") == 0)
    {
        soundOneShotPlayIsolated(_sounds[SOUND_FLAP], false);
    }
    else if (_stricmp(action, "platform") == 0)
    {
        soundOneShotPlayIsolated(_sounds[SOUND_PLATFORMBOUNCE], false);
    }
}
