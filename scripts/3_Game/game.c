// #include "Scripts/DayZGame.c"
#include "Scripts/DataBase/DataBase.h"
#include "Scripts/DataBase/DataBase.c"

// ---------------------
CGame CreateGame()
{
	// DataBase проверка и создание папок
	DataBase_CreateDir();
	// дальнейшая инициализация
	Print("CreateGame()");
	g_Game = new DayZGame;
	return g_Game;
}
