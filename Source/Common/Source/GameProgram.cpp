#include <GameProgram.hpp>
#include <iostream>

namespace Hit
{
	GameProgram::GameProgram( )
	{
	}

	GameProgram::~GameProgram( )
	{
		if( m_pGameWindow )
		{
			delete m_pGameWindow;
		}
	}

	HIT_UINT32 GameProgram::Initialise( )
	{
		std::cout << "Game initialising" << std::endl;

		if( this->PlatformInitialise( ) != OK )
		{
			std::cout << "Failed to perform platform initialisation" <<
				std::endl;

			return FATALERROR;
		}

		WINDOW_PARAMETERS WindowParameters;

		if( m_pGameWindow->Create( WindowParameters ) != GameWindow::OK )
		{
			std::cout << "Failed to create window" << std::endl;

			return FATALERROR;
		}

		return OK;
	}

	HIT_UINT32 GameProgram::Execute( )
	{
		std::cout << "Game executing" << std::endl;

		// There should be a game state manager here or something
		while( m_pGameWindow->IsOpen( ) )
		{
			m_pGameWindow->ProcessEvents( );
		}

		m_pGameWindow->Destroy( );

		return OK;
	}
}

