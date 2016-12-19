#ifndef __HIT_GAMEPROGRAM_HPP__
#define __HIT_GAMEPROGRAM_HPP__

#include <DataTypes.hpp>
#include <GameWindow.hpp>
#include <InputDeviceManager.hpp>

namespace Hit
{
	class GameProgram
	{
	public:
		GameProgram( );
		~GameProgram( );

		HIT_UINT32 Initialise( );

		HIT_UINT32 Execute( );

		static const HIT_UINT32 OK = 0;
		static const HIT_UINT32 ERROR = 1;
		static const HIT_UINT32 FATALERROR = -1;

	private:
		HIT_UINT32 PlatformInitialise( );

		GameWindow			*m_pGameWindow;
		InputDeviceManager	m_InputDeviceManager;
	};
}

#endif // __HIT_GAMEPROGRAM_HPP__

