#include <GameWindow.hpp>
#include <Renderer.hpp>

namespace Hit
{
	GameWindowData::~GameWindowData( )
	{
	}

	GAME_WINDOW_DATA_TYPE GameWindowData::GetType( ) const
	{
		return m_Type;
	}

	GameWindow::~GameWindow( )
	{
	}

	HIT_BOOL GameWindow::IsOpen( ) const
	{
		return m_Open;
	}

	GameWindowData *GameWindow::GetGameWindowData( )
	{
		return m_pGameWindowData;
	}

	void GameWindow::SetRenderer( Renderer *p_pRenderer )
	{
		m_pRenderer = p_pRenderer;
	}
}

