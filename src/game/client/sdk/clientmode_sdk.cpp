#include "cbase.h"
#include "clientmode_sdk.h"
#include "vgui_int.h"
#include "ienginevgui.h"

#include "sdk_loading_panel.h"
#include "sdk_logo_panel.h"
#include "ivmodemanager.h"
#include "panelmetaclassmgr.h"
#include "nb_header_footer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar default_fov( "default_fov", "75", FCVAR_CHEAT );
ConVar fov_desired( "fov_desired", "75", FCVAR_USERINFO, "Sets the base field-of-view.", true, 1.0, true, 75.0 );

vgui::HScheme g_hVGuiCombineScheme = 0;

vgui::DHANDLE<CSDK_Logo_Panel> g_hLogoPanel;

static IClientMode *g_pClientMode[ MAX_SPLITSCREEN_PLAYERS ];
IClientMode *GetClientMode()
{
	Assert( engine->IsLocalPlayerResolvable() );
	return g_pClientMode[ engine->GetActiveSplitScreenPlayerSlot() ];
}

// --------------------------------------------------------------------------------- //
// CASWModeManager.
// --------------------------------------------------------------------------------- //

class CSDKModeManager : public IVModeManager
{
public:
	virtual void	Init();
	virtual void	SwitchMode( bool commander, bool force ) {}
	virtual void	LevelInit( const char *newmap );
	virtual void	LevelShutdown( void );
	virtual void	ActivateMouse( bool isactive ) {}
};

static CSDKModeManager g_ModeManager;
IVModeManager *modemanager = ( IVModeManager * )&g_ModeManager;



// --------------------------------------------------------------------------------- //
// CASWModeManager implementation.
// --------------------------------------------------------------------------------- //

#define SCREEN_FILE		"scripts/vgui_screens.txt"

void CSDKModeManager::Init()
{
	for( int i = 0; i < MAX_SPLITSCREEN_PLAYERS; ++i )
	{
		ACTIVE_SPLITSCREEN_PLAYER_GUARD( i );
		g_pClientMode[ i ] = GetClientModeNormal();
	}

	PanelMetaClassMgr()->LoadMetaClassDefinitionFile( SCREEN_FILE );

	//GetClientVoiceMgr()->SetHeadLabelOffset( 40 );
}

void CSDKModeManager::LevelInit( const char *newmap )
{
	for( int i = 0; i < MAX_SPLITSCREEN_PLAYERS; ++i )
	{
		ACTIVE_SPLITSCREEN_PLAYER_GUARD( i );
		GetClientMode()->LevelInit( newmap );
	}
}

void CSDKModeManager::LevelShutdown( void )
{
	for( int i = 0; i < MAX_SPLITSCREEN_PLAYERS; ++i )
	{
		ACTIVE_SPLITSCREEN_PLAYER_GUARD( i );
		GetClientMode()->LevelShutdown();
	}
}

ClientModeSDK g_ClientModeNormal[ MAX_SPLITSCREEN_PLAYERS ];
IClientMode *GetClientModeNormal()
{
	Assert( engine->IsLocalPlayerResolvable() );
	return &g_ClientModeNormal[ engine->GetActiveSplitScreenPlayerSlot() ];
}

ClientModeSDK* GetClientModeSDK()
{
	Assert( engine->IsLocalPlayerResolvable() );
	return &g_ClientModeNormal[ engine->GetActiveSplitScreenPlayerSlot() ];
}

//-----------------------------------------------------------------------------
// Purpose: this is the viewport that contains all the hud elements
//-----------------------------------------------------------------------------
class CHudViewport : public CBaseViewport
{
private:
	DECLARE_CLASS_SIMPLE( CHudViewport, CBaseViewport );

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme )
	{
		BaseClass::ApplySchemeSettings( pScheme );

		GetHud().InitColors( pScheme );

		SetPaintBackgroundEnabled( false );
	}

	virtual void CreateDefaultPanels( void ) { /* don't create any panels yet*/ };
};
//--------------------------------------------------------------------------------------------------------

// See interface.h/.cpp for specifics:  basically this ensures that we actually Sys_UnloadModule the dll and that we don't call Sys_LoadModule 
//  over and over again.
static CDllDemandLoader g_GameUI( "gameui" );

class FullscreenSDKViewport : public CHudViewport
{
private:
	DECLARE_CLASS_SIMPLE( FullscreenSDKViewport, CHudViewport );

private:
	virtual void InitViewportSingletons( void )
	{
		SetAsFullscreenViewportInterface();
	}
};

class ClientModeSDKFullscreen : public	ClientModeSDK
{
	DECLARE_CLASS_SIMPLE( ClientModeSDKFullscreen, ClientModeSDK );
public:
	virtual void InitViewport()
	{
		// Skip over BaseClass!!!
		BaseClass::BaseClass::InitViewport();
		m_pViewport = new FullscreenSDKViewport();
		m_pViewport->Start( gameuifuncs, gameeventmanager );
	}
	virtual void Init()
	{
		CreateInterfaceFn gameUIFactory = g_GameUI.GetFactory();
		if ( gameUIFactory )
		{
			IGameUI *pGameUI = (IGameUI *) gameUIFactory(GAMEUI_INTERFACE_VERSION, NULL );
			if ( NULL != pGameUI )
			{
				// insert stats summary panel as the loading background dialog
				CSDK_Loading_Panel *pPanel = GSDKLoadingPanel();
				pPanel->InvalidateLayout( false, true );
				pPanel->SetVisible( false );
				pPanel->MakePopup( false );
				pGameUI->SetLoadingBackgroundDialog( pPanel->GetVPanel() );

				// add ASI logo to main menu
				CSDK_Logo_Panel *pLogo = new CSDK_Logo_Panel( NULL, "ASILogo" );
				vgui::VPANEL GameUIRoot = enginevgui->GetPanel( PANEL_GAMEUIDLL );
				pLogo->SetParent( GameUIRoot );
				g_hLogoPanel = pLogo;
			}		
		}

		// 
		//CASW_VGUI_Debug_Panel *pDebugPanel = new CASW_VGUI_Debug_Panel( GetViewport(), "ASW Debug Panel" );
		//g_hDebugPanel = pDebugPanel;

		// Skip over BaseClass!!!
		BaseClass::BaseClass::Init();

		// Load up the combine control panel scheme
		if ( !g_hVGuiCombineScheme )
		{
			g_hVGuiCombineScheme = vgui::scheme()->LoadSchemeFromFileEx( enginevgui->GetPanel( PANEL_CLIENTDLL ), IsXbox() ? "resource/ClientScheme.res" : "resource/CombinePanelScheme.res", "CombineScheme" );
			if (!g_hVGuiCombineScheme)
			{
				Warning( "Couldn't load combine panel scheme!\n" );
			}
		}
	}
	void Shutdown()
	{
		DestroySDKLoadingPanel();
		if (g_hLogoPanel.Get())
		{
			delete g_hLogoPanel.Get();
		}
	}
};

//--------------------------------------------------------------------------------------------------------
static ClientModeSDKFullscreen g_FullscreenClientMode;
IClientMode *GetFullscreenClientMode( void )
{
	return &g_FullscreenClientMode;
}

void ClientModeSDK::Shutdown()
{
	if ( SDKBackgroundMovie() )
	{
		SDKBackgroundMovie()->ClearCurrentMovie();
	}
	DestroySDKLoadingPanel();
	if (g_hLogoPanel.Get())
	{
		delete g_hLogoPanel.Get();
	}
}

void ClientModeSDK::InitViewport()
{
	m_pViewport = new CHudViewport();
	m_pViewport->Start( gameuifuncs, gameeventmanager );
}

void ClientModeSDK::DoPostScreenSpaceEffects( const CViewSetup *pSetup )
{

}