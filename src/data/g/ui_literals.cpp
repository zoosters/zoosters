// Unnamed UI literals (floats, resource names) in the game's order. Floats are m_k<n>, strings are named by content.

struct UiLiteralPool
{
	float m_k0;
	float m_k1;
	float m_k2;
	float m_k3;
	float m_k4;
	char m_tsa16[8];
	float m_k5;
	float m_k6;
	char m_promptCraneButton[20];
	char m_pressStart[12];
	char m_penguinGunner[16];
	char m_crosshairs3D[16];
	char m_rave2P2ControllerIn[24];
	char m_ravePromptOInstReplay[28];
	char m_mEMPromptSelect[20];
	char m_mEMLoadGameTitle[20];
	char m_mEMNewGameTitle[20];
	char m_mEMTitleMenuTitle[24];
	char m_menuSliderBkgnd[20];
	char m_menuChecklistBkTop[24];
	char m_aUTOMsgBoxTop[20];
	char m_msgBoxBot[12];
	char m_matchConfirmationMsg[24];
	char m_quitConfirmationMsg[24];
	char m_shopMenuMaster[20];
	char m_gameMenuMaster[20];
	char m_menuMaster[12];
	char m_menuFrameTop[16];
	char m_promptBkgndNew[20];
	char m_pickAction[16];
};

// ALONE: g_uiLiteralPool unnamed literal pool, followed by a vtable and the last literal (prompt_main_menu, NUL shared with the next block)
// GLOBAL: 0x5cbd58 g_uiLiteralPool
extern const UiLiteralPool g_uiLiteralPool = {
	480.0f,
	640.0f,
	0.000416666677f,
	2400.0f,
	360.0f,
	"tsa16",
	0.00200000009f,
	0.100000001f,
	"Prompt_Crane_Button",
	"Press Start",
	"Penguin_Gunner",
	"Crosshairs 3D",
	"Rave_2P_2ControllerIn",
	"Rave_Prompt_O_InstReplay",
	"MEM_prompt_select",
	"MEM_Load_Game_title",
	"MEM_New_Game_title",
	"MEM_Title_menu_title",
	"Menu_Slider_Bkgnd",
	"menu_checklist_bkTop",
	"AUTO_Msg_Box_Top",
	"Msg_Box_Bot",
	"Match_Confirmation_Msg",
	"Quit_Confirmation_Msg",
	"Shop_menu_master",
	"game_menu_master",
	"menu_master",
	"menu_frame_top",
	"prompt_bkgnd_new",
	"& Pick Action",
};
