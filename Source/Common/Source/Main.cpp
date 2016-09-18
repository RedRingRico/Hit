#include <iostream>
#include <linux/joystick.h>
#include <GameProgram.hpp>

int main( int p_Argc, char **p_ppArgv )
{
	Hit::GameProgram TheGame;

	if( TheGame.Initialise( ) != Hit::GameProgram::OK )
	{
		std::cout << "Failed to start the game" << std::endl;

		return 1;
	}

	return TheGame.Execute( );
}

