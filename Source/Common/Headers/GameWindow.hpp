#ifndef __HIT_GAMEWINDOW_HPP__
#define __HIT_GAMEWINDOW_HPP__

#include <DataTypes.hpp>

namespace Hit
{
	struct WINDOW_PARAMETERS
	{
		HIT_SINT32	X;
		HIT_SINT32	Y;
		HIT_UINT32	Width;
		HIT_UINT32	Height;
		HIT_BOOL	Fullscreen;
	};

	enum GAME_WINDOW_DATA_TYPE
	{
		GAME_WINDOW_DATA_TYPE_XCB,
		GAME_WINDOW_DATA_TYPE_UNKNOWN
	};

	class GameWindowData
	{
	public:
		virtual ~GameWindowData( );

		GAME_WINDOW_DATA_TYPE GetType( ) const;

	protected:
		GAME_WINDOW_DATA_TYPE	m_Type;
	};

	class Renderer;

	class GameWindow
	{
	public:
		virtual ~GameWindow( );

		virtual HIT_UINT32 Create(
			const WINDOW_PARAMETERS &p_WindowParameters ) = 0;
		virtual void Destroy( ) = 0;

		virtual void ProcessEvents( ) = 0;

		GameWindowData *GetGameWindowData( );

		void SetRenderer( Renderer *p_pRenderer );

		HIT_BOOL IsOpen( ) const;

		static const HIT_UINT32 OK = 0;
		static const HIT_UINT32 ERROR = 1;
		static const HIT_UINT32 FATALERROR = -1;

	protected:
		HIT_SINT32	m_X;
		HIT_SINT32	m_Y;
		HIT_UINT32	m_Width;
		HIT_UINT32	m_Height;

		HIT_BOOL	m_Open;
		HIT_BOOL	m_Resize;

		GameWindowData				*m_pGameWindowData;
		Renderer					*m_pRenderer;
	};
}

#endif // __HIT_GAMEWINDOW_HPP__

