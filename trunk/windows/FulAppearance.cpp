#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "../client/SettingsManager.h"

#include "fulappearance.h"
#include "WinUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::TextItem FulAppearancePage::texts[] = {
	{ IDC_SB_CHATBUFFERSIZE,  ResourceManager::SETTINGS_SB_CHATBUFFERSIZE	 },
	{ IDC_SB_POPUP,			  ResourceManager::SETTINGS_SB_POPUP			 },
	{ IDC_SB_TAB_COLORS,	  ResourceManager::SETTINGS_SB_TAB_COLORS		 },
	{ IDC_SB_TAB_SIZE,		  ResourceManager::SETTINGS_SB_TAB_SIZE			 },
	{ IDC_ST_CHATBUFFERSIZE,  ResourceManager::SETTINGS_ST_CHATBUFFERSIZE	 },
	{ IDC_ST_DISPLAYTIME,	  ResourceManager::SETTINGS_ST_DISPLAYTIME		 },
	{ IDC_ST_MESSAGE_LENGTH,  ResourceManager::SETTINGS_ST_MESSAGE_LENGTH	 },
	{ IDC_BTN_COLOR,		  ResourceManager::SETTINGS_BTN_COLOR			 },
	{ IDC_BTN_FONT,			  ResourceManager::SETTINGS_BTN_FONT			 },
	{ IDC_BTN_TEXTCOLOR,	  ResourceManager::SETTINGS_BTN_TEXTCOLOR		 },
	{ IDC_BTN_TIME_STAMP_HELP,ResourceManager::SETTINGS_TIME_STAMP_HELP		 },
	{ IDC_SB_TIME_STAMPS,	  ResourceManager::SETTINGS_SB_TIME_STAMPS		 },
	{ 0,					  ResourceManager::SETTINGS_AUTO_AWAY			 }
};

PropPage::Item FulAppearancePage::items[] = {
	{ IDC_CHATBUFFERSIZE,	SettingsManager::CHATBUFFERSIZE,	PropPage::T_INT },
	{ IDC_TAB_SIZE,			SettingsManager::TAB_SIZE,			PropPage::T_INT }, 
	{ IDC_DISPLAYTIME,		SettingsManager::POPUP_TIMEOUT,		PropPage::T_INT },
	{ IDC_MESSAGE_LENGTH,	SettingsManager::MAX_MSG_LENGTH,	PropPage::T_INT },
	{ IDC_EDIT_TIME_STAMPS, SettingsManager::TIME_STAMPS_FORMAT,PropPage::T_STR },
	{ 0,					0,									PropPage::T_END }
};

LRESULT FulAppearancePage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::read((HWND)*this, items);
	PropPage::translate((HWND)(*this), texts);

	colorList.Attach(GetDlgItem(IDC_COLOR_COMBO));

	colorList.AddString(CSTRING(TAB_ACTIVE_BG));
	colorList.AddString(CSTRING(TAB_ACTIVE_TEXT));
	colorList.AddString(CSTRING(TAB_ACTIVE_BORDER));
	colorList.AddString(CSTRING(TAB_INACTIVE_BG));
	colorList.AddString(CSTRING(TAB_INACTIVE_TEXT));
	colorList.AddString(CSTRING(TAB_INACTIVE_BORDER));
	colorList.AddString(CSTRING(TAB_INACTIVE_BG_NOTIFY));
	colorList.AddString(CSTRING(TAB_INACTIVE_BG_DISCONNECTED));
	colorList.SetCurSel(0);

	colorList.Detach();

	return TRUE;
}

void FulAppearancePage::write() {
	PropPage::write((HWND)*this, items);
}

LRESULT FulAppearancePage::onColorButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	colorList.Attach(GetDlgItem(IDC_COLOR_COMBO));
	int sel = colorList.GetCurSel();
	COLORREF col;
	switch(sel){
		case 0: col = SETTING(TAB_ACTIVE_BG); break;
		case 1: col = SETTING(TAB_ACTIVE_TEXT); break;
		case 2: col = SETTING(TAB_ACTIVE_BORDER); break;
		case 3: col = SETTING(TAB_INACTIVE_BG); break;
		case 4: col = SETTING(TAB_INACTIVE_TEXT); break;
		case 5: col = SETTING(TAB_INACTIVE_BORDER); break;
		case 6: col = SETTING(TAB_INACTIVE_BG_NOTIFY); break;
		case 7: col = SETTING(TAB_INACTIVE_BG_DISCONNECTED); break;
		default: col = RGB(0, 0, 0); break;
	}

	CColorDialog dlg(col, CC_FULLOPEN);
	if(dlg.DoModal() == IDOK){
		switch(sel){
		case 0: settings->set(SettingsManager::TAB_ACTIVE_BG, (int)dlg.GetColor()); break;
		case 1: settings->set(SettingsManager::TAB_ACTIVE_TEXT, (int)dlg.GetColor()); break;
		case 2: settings->set(SettingsManager::TAB_ACTIVE_BORDER, (int)dlg.GetColor()); break;
		case 3: settings->set(SettingsManager::TAB_INACTIVE_BG, (int)dlg.GetColor()); break;
		case 4: settings->set(SettingsManager::TAB_INACTIVE_TEXT, (int)dlg.GetColor()); break;
		case 5: settings->set(SettingsManager::TAB_INACTIVE_BORDER, (int)dlg.GetColor()); break;
		case 6: settings->set(SettingsManager::TAB_INACTIVE_BG_NOTIFY, (int)dlg.GetColor()); break;
		case 7: settings->set(SettingsManager::TAB_INACTIVE_BG_DISCONNECTED, (int)dlg.GetColor()); break;
		}
	}

	colorList.Detach();
	return 0;
}

LRESULT FulAppearancePage::onTextColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CColorDialog dlg(SETTING(POPUP_TEXTCOLOR), CC_FULLOPEN);
	if(dlg.DoModal() == IDOK)
		SettingsManager::getInstance()->set(SettingsManager::POPUP_TEXTCOLOR, (int)dlg.GetColor());
	return 0;
}

LRESULT FulAppearancePage::onFont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	LOGFONT font;  
	WinUtil::decodeFont(SETTING(POPUP_FONT), font);
	CFontDialog dlg(&font, CF_EFFECTS | CF_SCREENFONTS);
	if(dlg.DoModal() == IDOK){
		settings->set(SettingsManager::POPUP_FONT, WinUtil::encodeFont(font));
	}
	return 0;
}

LRESULT FulAppearancePage::onTimeStampHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	MessageBox(CSTRING(TIME_STAMPS_HELP), CSTRING(TIME_STAMPS_HELP_CAPTION), MB_OK | MB_ICONINFORMATION);

	return 0;
}