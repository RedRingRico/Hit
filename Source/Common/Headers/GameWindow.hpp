#ifndef __HIT_GAMEWINDOW_HPP__
#define __HIT_GAMEWINDOW_HPP__

#include <DataTypes.hpp>

namespace Hit
{
	struct WINDOW_PARAMETERS
	{
		HIT_SINT32	m_X;
		HIT_SINT32	m_Y;
		HIT_UINT32	m_Width;
		HIT_UINT32	m_Height;
	};

	class GameWindow
	{
	public:
		virtual ~GameWindow( );

		virtual HIT_UINT32 Create(
			const WINDOW_PARAMETERS &p_WindowParameters ) = 0;
		virtual void Destroy( ) = 0;

		virtual void ProcessEvents( ) = 0;

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
	};
}

#endif // __HIT_GAMEWINDOW_HPP__

