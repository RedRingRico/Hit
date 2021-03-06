#include <GameWindowXCB.hpp>
#include <iostream>
#include <xcb/xproto.h>
#include <xcb/randr.h>
#include <xcb/xfixes.h>
#include <Renderer.hpp>

namespace Hit
{
	GameWindowDataXCB::GameWindowDataXCB( xcb_connection_t *p_pXCBConnection,
		xcb_window_t p_XCBWindow ) :
		m_pXCBConnection( p_pXCBConnection ),
		m_XCBWindow( p_XCBWindow )
	{
		m_Type = GAME_WINDOW_DATA_TYPE_XCB;
	}

	GameWindowDataXCB::~GameWindowDataXCB( )
	{
	}

	xcb_connection_t *GameWindowDataXCB::GetConnection( ) const
	{
		return m_pXCBConnection;
	}

	xcb_window_t GameWindowDataXCB::GetWindow( ) const
	{
		return m_XCBWindow;
	}

	GameWindowXCB::GameWindowXCB( ) :
		m_pXCBConnection( nullptr ),
		m_pXCBDeleteReply( nullptr )
	{
		m_X = 0;
		m_Y = 0;
		m_Width = 640;
		m_Height = 480;
		m_Open = HIT_FALSE;
		m_pGameWindowData = nullptr;
		m_Resize = HIT_FALSE;
		m_pRenderer = nullptr;
	}

	GameWindowXCB::~GameWindowXCB( )
	{
		this->Destroy( );
	}

	HIT_UINT32 GameWindowXCB::Create(
		const WINDOW_PARAMETERS &p_WindowParameters )
	{
		int ScreenIndex;

		m_pXCBConnection = xcb_connect( nullptr, &ScreenIndex );
		int Error = xcb_connection_has_error( m_pXCBConnection );

		if( Error != 0 )
		{
			std::cout << "Unable to open XCB display" << std::endl;

			std::cout << "ERROR: ";

			switch( Error )
			{
				case XCB_CONN_ERROR:
				{
					std::cout << "Stream";
					break;
				}
				case XCB_CONN_CLOSED_EXT_NOTSUPPORTED:
				{
					std::cout << "Unsupported extension";
					break;
				}
				case XCB_CONN_CLOSED_MEM_INSUFFICIENT:
				{
					std::cout << "Not enough memory";
					break;
				}
				case XCB_CONN_CLOSED_REQ_LEN_EXCEED:
				{
					std::cout << "Requested length exceeded";
					break;
				}
				case XCB_CONN_CLOSED_PARSE_ERR:
				{
					std::cout << "Failed to parse display string";
					break;
				}
				case XCB_CONN_CLOSED_INVALID_SCREEN:
				{
					std::cout << "No screen matching the display";
					break;
				}
				default:
				{
					std::cout << "UNKNOWN [" << Error << "]";
					break;
				}
			}

			std::cout << std::endl;

			return FATALERROR;
		}

		const xcb_setup_t *pXCBSetup = xcb_get_setup( m_pXCBConnection );
		xcb_screen_iterator_t ScreenIterator =
			xcb_setup_roots_iterator( pXCBSetup );

		while( ScreenIndex-- > 0 )
		{
			xcb_screen_next( &ScreenIterator );
		}

		xcb_screen_t *pXCBScreen = ScreenIterator.data;

		uint32_t XCBValues[ ] =
		{
			pXCBScreen->black_pixel,
			XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS |
				XCB_EVENT_MASK_STRUCTURE_NOTIFY
		};

		m_X = p_WindowParameters.X;
		m_Y = p_WindowParameters.Y;
		m_Width = p_WindowParameters.Width;
		m_Height = p_WindowParameters.Height;

		m_XCBWindow = xcb_generate_id( m_pXCBConnection );

		// Create a window which will be destroyed to get a fullscreen one
		xcb_create_window( m_pXCBConnection, XCB_COPY_FROM_PARENT,
			m_XCBWindow, pXCBScreen->root,
			m_X, m_Y, m_Width, m_Height, 0,
			XCB_WINDOW_CLASS_INPUT_OUTPUT, pXCBScreen->root_visual,
			XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK, XCBValues );

		xcb_flush( m_pXCBConnection );

		if( p_WindowParameters.Fullscreen == HIT_TRUE )
		{
			// Use the primary display to set up the window
			xcb_generic_error_t *pError = nullptr;

			xcb_randr_get_output_primary_cookie_t RandrOutputPrimary =
				xcb_randr_get_output_primary( m_pXCBConnection, m_XCBWindow );
			xcb_randr_get_output_primary_reply_t *pRandrOutputPrimary =
				xcb_randr_get_output_primary_reply( m_pXCBConnection,
					RandrOutputPrimary, &pError );

			if( pError != nullptr )
			{
				std::cout << "Error" << std::endl;

				std::cout << "Response:    " << +pError->response_type <<
					std::endl;
				std::cout << "Error code:  " << +pError->error_code <<
					std::endl;
				std::cout << "Sequence:    " << pError->sequence << std::endl;
				std::cout << "Resource ID: " << pError->resource_id <<
					std::endl;
				std::cout << "Major: " << +pError->major_code << std::endl;
				std::cout << "Minor: " << pError->minor_code << std::endl;
				std::cout << "Full sequence: " << pError->full_sequence <<
					std::endl;

				free( pRandrOutputPrimary );

				return FATALERROR;
			}

			pError = nullptr;

			xcb_randr_get_output_info_cookie_t RandrGetOutputInfo =
				xcb_randr_get_output_info( m_pXCBConnection,
					pRandrOutputPrimary->output, 0 );
			xcb_randr_get_output_info_reply_t *pRandrGetOutputInfo =
				xcb_randr_get_output_info_reply( m_pXCBConnection,
					RandrGetOutputInfo, &pError );

			free( pRandrOutputPrimary );

			if( pError != nullptr )
			{
				std::cout << "Error" << std::endl;

				std::cout << "Response:    " << +pError->response_type <<
					std::endl;
				std::cout << "Error code:  " << +pError->error_code <<
					std::endl;
				std::cout << "Sequence:    " << pError->sequence << std::endl;
				std::cout << "Resource ID: " << pError->resource_id <<
					std::endl;
				std::cout << "Major: " << +pError->major_code << std::endl;
				std::cout << "Minor: " << pError->minor_code << std::endl;
				std::cout << "Full sequence: " << pError->full_sequence <<
					std::endl;

				free( pRandrGetOutputInfo );

				return FATALERROR;
			}

			xcb_randr_get_crtc_info_cookie_t CRTCCookie =
				xcb_randr_get_crtc_info( m_pXCBConnection,
					pRandrGetOutputInfo->crtc, 0 );
			xcb_randr_get_crtc_info_reply_t *pCRTCResourceReply =
				xcb_randr_get_crtc_info_reply( m_pXCBConnection, CRTCCookie,
					0 );

			free( pRandrGetOutputInfo );

			std::cout << "[CRTC Info]" << std::endl;
			std::cout << "\tX offset: " << pCRTCResourceReply->x <<	std::endl;
			std::cout << "\tY offset: " << pCRTCResourceReply->y << std::endl;
			std::cout << "\tWidth:    " << pCRTCResourceReply->width <<
				std::endl;
			std::cout << "\tHeight:   " << pCRTCResourceReply->height <<
				std::endl;

			m_X = pCRTCResourceReply->x;
			m_Y = pCRTCResourceReply->y;
			m_Width = pCRTCResourceReply->width;
			m_Height = pCRTCResourceReply->height;

			free( pCRTCResourceReply );

			xcb_destroy_window( m_pXCBConnection, m_XCBWindow );

			// Recreate the window with the primary screen information
			xcb_create_window( m_pXCBConnection, XCB_COPY_FROM_PARENT,
				m_XCBWindow, pXCBScreen->root, m_X, m_Y, m_Width, m_Height, 0,
				XCB_WINDOW_CLASS_INPUT_OUTPUT, pXCBScreen->root_visual,
				XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK, XCBValues );

			xcb_intern_atom_cookie_t StateCookie = xcb_intern_atom(
				m_pXCBConnection, 1, 13, "_NET_WM_STATE" );
			xcb_intern_atom_reply_t *pStateReply = xcb_intern_atom_reply(
				m_pXCBConnection, StateCookie, 0 );
			xcb_intern_atom_cookie_t FullscreenCookie = xcb_intern_atom(
				m_pXCBConnection, 1, 24, "_NET_WM_STATE_FULLSCREEN" );
			xcb_intern_atom_reply_t *pFullscreenReply = xcb_intern_atom_reply(
				m_pXCBConnection, FullscreenCookie, 0 );
			xcb_change_property( m_pXCBConnection, XCB_PROP_MODE_REPLACE,
				m_XCBWindow, ( *pStateReply ).atom, 4, 32, 1,
				&( *pFullscreenReply ).atom );

			free( pFullscreenReply );
			free( pStateReply );
		}

		xcb_map_window( m_pXCBConnection, m_XCBWindow );
		xcb_flush( m_pXCBConnection );

		xcb_xfixes_hide_cursor( m_pXCBConnection, m_XCBWindow );
		m_Open = HIT_TRUE;

		if( m_pGameWindowData )
		{
			delete m_pGameWindowData;
		}

		m_pGameWindowData = new GameWindowDataXCB( m_pXCBConnection,
			m_XCBWindow );

		xcb_intern_atom_cookie_t ProtocolsCookie = xcb_intern_atom(
			m_pXCBConnection, 1, 12, "WM_PROTOCOLS" );
		xcb_intern_atom_reply_t *pProtocolsReply = xcb_intern_atom_reply(
			m_pXCBConnection, ProtocolsCookie, 0 );
		xcb_intern_atom_cookie_t DeleteCookie = xcb_intern_atom(
			m_pXCBConnection, 0, 16, "WM_DELETE_WINDOW" );
		m_pXCBDeleteReply = xcb_intern_atom_reply( m_pXCBConnection,
			DeleteCookie, 0 );

		xcb_change_property( m_pXCBConnection, XCB_PROP_MODE_REPLACE,
			m_XCBWindow, ( *pProtocolsReply ).atom, 4, 32, 1,
			&( *m_pXCBDeleteReply ).atom );
		free( pProtocolsReply );

		return OK;
	}

	void GameWindowXCB::Destroy( )
	{
		if( m_pXCBDeleteReply )
		{
			free( m_pXCBDeleteReply );
			m_pXCBDeleteReply = nullptr;
		}

		if( m_pGameWindowData )
		{
			delete m_pGameWindowData;
			m_pGameWindowData = nullptr;
		}

		if( m_pXCBConnection )
		{
			xcb_xfixes_show_cursor( m_pXCBConnection, m_XCBWindow );
			xcb_unmap_window( m_pXCBConnection, m_XCBWindow );
			xcb_destroy_window( m_pXCBConnection, m_XCBWindow );
			xcb_disconnect( m_pXCBConnection );
			m_pXCBConnection = nullptr;
		}

		m_Open = HIT_FALSE;
	}

	void GameWindowXCB::ProcessEvents( )
	{
		xcb_generic_event_t *pEvent;

		while( ( pEvent = xcb_poll_for_event( m_pXCBConnection ) ) != nullptr )
		{
			switch( pEvent->response_type & 0x7F )
			{
				case XCB_CONFIGURE_NOTIFY:
				{
					xcb_configure_notify_event_t *pConfigureEvent =
						reinterpret_cast< xcb_configure_notify_event_t * >(
							pEvent );
					if( ( ( pConfigureEvent->width > 0 ) &&
							( m_Width != pConfigureEvent->width ) ) ||
						( ( pConfigureEvent->height > 0 ) &&
							( m_Height != pConfigureEvent->height ) ) )
					{
						m_Width = pConfigureEvent->width;
						m_Height = pConfigureEvent->height;
						m_Resize = HIT_TRUE;
					}

					break;
				}
				case XCB_CLIENT_MESSAGE:
				{
					if( ( *( xcb_client_message_event_t * )
						pEvent ).data.data32[ 0 ] ==
						( *m_pXCBDeleteReply ).atom )
					{
						m_Open = HIT_FALSE;
						free( m_pXCBDeleteReply );
						m_pXCBDeleteReply = nullptr;

						break;
					}
				}
				case XCB_KEY_PRESS:
				{
					break;
				}
			}
		}

		if( m_Resize == HIT_TRUE )
		{
			m_Resize = HIT_FALSE;

			if( m_pRenderer != nullptr )
			{
				m_pRenderer->OnGameWindowResized( );
			}
		}
	}
}

