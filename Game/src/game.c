#include <time.h>

#include "baseTypes.h"
#include "input.h"
#include "application.h"
#include "framework.h"
#include "sound.h"
#include "levelmgr.h"
#include "objmgr.h"
#include "collisionMgr.h"


#define LEVEL_INDEX_HISCORES		0
#define LEVEL_INDEX_TITLE_SCREEN	1
#define LEVEL_INDEX_FIRST_WAVE		2


static uint8_t _numLevels = 1;
static uint8_t _curWaveIndex = LEVEL_INDEX_FIRST_WAVE;


static void _gameInit();
static void _gameShutdown();
static void _gameDraw();
static void _gameUpdate(uint32_t milliseconds);


// Level Order: hiscores (not implemented, will most likely just display your score), title, waves
// Contains the first 30 specified waves, then last one is an "endless" wave consisting of Shadow Lords
	// Has mostly the same as what I could find online. Wave 28 is different, and there are still some waves after 30 that do not seem formulaic. 
	// Note that starting on wave 37, only Shadow Lords should appear (except for the wave after an egg wave). (Not coded)
	// After wave 60 all waves should only contain Shadow Lords. (Not coded)
static LevelDef _levelDefs[] = {	// { Leveltype, #Bounders, #Hunters, #ShadowLords }
	{ LEVELTYPE_HISCORES,		0, 0, 0 },
	{ LEVELTYPE_TITLE,			0, 0, 0 },		// Waves start below!
	{ LEVELTYPE_WAVE,			3, 0, 0 },		// 1: "Prepare to Joust"
	{ LEVELTYPE_WAVE,			4, 0, 0 },		
	{ LEVELTYPE_WAVE,			6, 0, 0 },		
	{ LEVELTYPE_WAVE,			3, 3, 0 },		
	{ LEVELTYPE_WAVE,			0, 0, 1 },		// 5: EGG WAVE
	{ LEVELTYPE_WAVE,			3, 3, 0 },		
	{ LEVELTYPE_WAVE,			2, 4, 0 },		// 7: Survival Wave
	{ LEVELTYPE_WAVE,			0, 6, 0 },		
	{ LEVELTYPE_WAVE,			0, 6, 0 },		
	{ LEVELTYPE_WAVE,			0, 0, 2 },		// 10: EGG WAVE
	{ LEVELTYPE_WAVE,			3, 5, 0 },
	{ LEVELTYPE_WAVE,			2, 6, 0 },		// 12: Survival Wave
	{ LEVELTYPE_WAVE,			0, 7, 0 },
	{ LEVELTYPE_WAVE,			0, 8, 0 },
	{ LEVELTYPE_WAVE,			0, 0, 3 },		// 15: EGG WAVE
	{ LEVELTYPE_WAVE,			0, 5, 1 },
	{ LEVELTYPE_WAVE,			0, 5, 1 },		// 17: Survival
	{ LEVELTYPE_WAVE,			0, 5, 1 },
	{ LEVELTYPE_WAVE,			0, 4, 2 },
	{ LEVELTYPE_WAVE,			0, 0, 4 },		// 20: EGG WAVE
	{ LEVELTYPE_WAVE,			0, 3, 3 },
	{ LEVELTYPE_WAVE,			0, 2, 4 },		// 22: Survival
	{ LEVELTYPE_WAVE,			0, 2, 4 },
	{ LEVELTYPE_WAVE,			0, 2, 4 },
	{ LEVELTYPE_WAVE,			0, 0, 5 },		// 25: EGG WAVE
	{ LEVELTYPE_WAVE,			0, 3, 5 },
	{ LEVELTYPE_WAVE,			0, 3, 5 },		// 27: Survival
	{ LEVELTYPE_WAVE,			0, 3, 5 },
	{ LEVELTYPE_WAVE,			0, 3, 5 },
	{ LEVELTYPE_WAVE,			0, 0, 6 },		// 30: EGG WAVE
	{ LEVELTYPE_WAVE_ENDLESS,	0, 0, 6 }		// ENDLESS WAVE: Only Shadow Lords
};
static Level* _curLevel = NULL;

/// @brief Program Entry Point (WinMain)
/// @param hInstance  
/// @param hPrevInstance 
/// @param lpCmdLine 
/// @param nCmdShow 
/// @return 
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	const char GAME_NAME[] = "Joust";

	Application* app = appNew(hInstance, GAME_NAME, _gameDraw, _gameUpdate);

	if (app != NULL)
	{
		GLWindow* window = fwInitWindow(app);
		if (window != NULL)
		{
			_gameInit();

			bool running = true;
			while (running)
			{
				running = fwUpdateWindow(window);
			}

			_gameShutdown();
			fwShutdownWindow(window);
		}

		appDelete(app);
	}
}

/// @brief Initialize code to run at application startup
static void _gameInit()
{
	srand((unsigned int)time(NULL));

	const uint32_t MAX_OBJECTS = 500;
	const uint32_t MAX_SOUNDS = 100;
	objMgrInit(MAX_OBJECTS);
	collisionMgrInit(MAX_OBJECTS);
	levelMgrInit();

	_curLevel = levelMgrLoad(&_levelDefs[LEVEL_INDEX_TITLE_SCREEN]);

	ShowCursor(true);
}

/// @brief Cleanup the game and free up any allocated resources
static void _gameShutdown()
{
	levelMgrUnload(_curLevel);

	levelMgrShutdown();
	collisionMgrShutdown();
	objMgrShutdown();
}

/// @brief Draw everything to the screen for current frame
static void _gameDraw() 
{
	objMgrDraw();
	levelMgrDraw(_curLevel);
}

/// @brief Perform updates for all game objects, for the elapsed duration
/// @param milliseconds 
static void _gameUpdate(uint32_t milliseconds)
{
	// Load and unload levels here for waves and game screens -> sequencing based on how the current level's update went
	switch (levelMgrUpdate(_curLevel, milliseconds))
	{
		case LUO_TITLE:			// Show title screen
		{
			levelMgrUnload(_curLevel);
			_curLevel = levelMgrLoad(&_levelDefs[LEVEL_INDEX_TITLE_SCREEN]);
			break;
		}
		case LUO_HISCORES:		// Show hiscores
		{
			levelMgrUnload(_curLevel);
			_curLevel = levelMgrLoad(&_levelDefs[LEVEL_INDEX_HISCORES]);
			break;
		}
		case LUO_STARTWAVES:	// Load the first wave
		{
			levelMgrUnload(_curLevel);
			_curLevel = levelMgrLoad(&_levelDefs[LEVEL_INDEX_FIRST_WAVE]);
			_curWaveIndex = LEVEL_INDEX_FIRST_WAVE;
			break;
		}
		case LUO_NEXTWAVE:		// Continue to the next wave
		{
			LevelType curLevelType = levelGetType(_curLevel);
			levelMgrUnload(_curLevel);
			if (curLevelType == LEVELTYPE_WAVE) { _curLevel = levelMgrLoad(&_levelDefs[++_curWaveIndex]); }
			else if (curLevelType == LEVELTYPE_WAVE_ENDLESS) { _curLevel = levelMgrLoad(&_levelDefs[_curWaveIndex]); }
			else { assert(false); }

			break;
		}
		default:
		{
			break;
		}
	}

	objMgrUpdate(milliseconds);
}
