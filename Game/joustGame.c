/*	Trenton Schacht
*	Programming 1
*	C Game: Joust
*	Joust Main Program
*/



// =============== Includes ===============
#include "baseTypes.h"


// =============== Defines ===============

// =============== Enums ===============

// =============== Structs ===============

// =============== Function Prototypes ===============
static void _gameInit();
static void _gameShutdown();
static void _gameDraw();
static void _gameUpdate(uint32_t milliseconds);





// =============== MAIN ===============
/// @brief Program Entry Point (WinMain)
/// @param hInstance 
/// @param hPrevInstance 
/// @param lpCmdLine 
/// @param nCmdShow 
/// @return 
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
//{
//	const char GAME_NAME[] = "Framework1";
//
//	Application* app = appNew(hInstance, GAME_NAME, _gameDraw, _gameUpdate);
//
//	if (app != NULL)
//	{
//		GLWindow* window = fwInitWindow(app);
//		if (window != NULL)
//		{
//			_gameInit();
//
//			bool running = true;
//			while (running)
//			{
//				running = fwUpdateWindow(window);
//			}
//
//			_gameShutdown();
//			fwShutdownWindow(window);
//		}
//
//		appDelete(app);
//	}
//}








// =============== Function Definitions ===============



