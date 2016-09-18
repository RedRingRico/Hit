#include <GameWindow.hpp>

namespace Hit
{
	GameWindow::~GameWindow( )
	{
	}

	HIT_BOOL GameWindow::IsOpen( ) const
	{
		return m_Open;
	}
}

