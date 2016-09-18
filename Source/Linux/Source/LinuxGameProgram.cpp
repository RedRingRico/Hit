#include <GameProgram.hpp>
#include <GameWindowXCB.hpp>
#include <iostream>

namespace Hit
{
	HIT_UINT32 GameProgram::PlatformInitialise( )
	{
		m_pGameWindow = new GameWindowXCB( );

		if( m_pGameWindow == nullptr )
		{
			std::cout << "Failed to create game window" << std::endl;

			return FATALERROR;
		}

		return OK;
	}
}

