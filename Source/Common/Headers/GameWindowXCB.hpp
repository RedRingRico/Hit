#ifndef __HIT_GAMEWINDOWXCB_HPP__
#define __HIT_GAMEWINDOWXCB_HPP__

#include <GameWindow.hpp>
#include <xcb/xcb.h>

namespace Hit
{
	class GameWindowDataXCB : public GameWindowData
	{
	public:
		GameWindowDataXCB( xcb_connection_t *p_pXCBConnection,
			xcb_window_t m_XCBWindow );
		virtual ~GameWindowDataXCB( );

		xcb_connection_t *GetConnection( ) const;
		xcb_window_t GetWindow( ) const;

	private:
		xcb_connection_t	*m_pXCBConnection;
		xcb_window_t		m_XCBWindow;
	};

	class GameWindowXCB : public GameWindow
	{
	public:
		GameWindowXCB( );
		~GameWindowXCB( );

		virtual HIT_UINT32 Create(
			const WINDOW_PARAMETERS &p_WindowParameters );
		virtual void Destroy( );

		virtual void ProcessEvents( );

	private:
		GameWindowXCB( const GameWindowXCB &p_Other );
		GameWindowXCB &operator=( const GameWindowXCB &p_Other );

		xcb_connection_t	*m_pXCBConnection;
		xcb_window_t		m_XCBWindow;
	};
};

#endif // __HIT_GAMEWINDOW_HPP__

