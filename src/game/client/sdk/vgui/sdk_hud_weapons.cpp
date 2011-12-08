#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"

#include "iclientmode.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

// shows weapons held by the current marine
class CSDKHudWeapons : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CSDKHudWeapons, CHudNumericDisplay );

public:
	CSDKHudWeapons( const char *pElementName );
	~CSDKHudWeapons();
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void Paint();	
	virtual void ApplySchemeSettings(IScheme *pScheme);
	//virtual bool ShouldDraw( void ) { return asw_draw_hud.GetBool() && CASW_HudElement::ShouldDraw(); }
	
	int m_iFrameWidth, m_iFrameHeight;
private:
};	

// Disabled - merged into ASW_Hud_Master
//DECLARE_HUDELEMENT( CSDKHudWeapons );

CSDKHudWeapons::CSDKHudWeapons( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay(NULL, "SDKHudWeapons")
{
	SetHiddenBits( HIDEHUD_PLAYERDEAD);
	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/SwarmSchemeNew.res", "SwarmSchemeNew");
	SetScheme(scheme);

}

CSDKHudWeapons::~CSDKHudWeapons()
{
	
}

void CSDKHudWeapons::Init()
{
	Reset();
}

void CSDKHudWeapons::Reset()
{

}

void CSDKHudWeapons::VidInit()
{
	Reset();
}

void CSDKHudWeapons::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetBgColor(Color(0,0,0,0));
	//m_hWeaponHUDFont = SDK_GetDefaultFont(false);
}

void CSDKHudWeapons::Paint()
{

}
