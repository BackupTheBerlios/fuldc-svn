#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "../client/SettingsManager.h"

#include "fulappearance.h"
#include "WinUtil.h"

PropPage::TextItem FulAppearancePage::texts[] = {
	{ IDC_SB_CHATBUFFERSIZE,  ResourceManager::SETTINGS_SB_CHATBUFFERSIZE	 },
	{ IDC_SB_POPUP,			  ResourceManager::SETTINGS_SB_POPUP			 },
	{ IDC_ST_CHATBUFFERSIZE,  ResourceManager::SETTINGS_ST_CHATBUFFERSIZE	 },
	{ IDC_ST_DISPLAYTIME,	  ResourceManager::SETTINGS_ST_DISPLAYTIME		 },
	{ IDC_ST_MESSAGE_LENGTH,  ResourceManager::SETTINGS_ST_MESSAGE_LENGTH	 },
	{ IDC_BTN_FONT,			  ResourceManager::SETTINGS_BTN_FONT			 },
	{ IDC_BTN_TEXTCOLOR,	  ResourceManager::SETTINGS_BTN_TEXTCOLOR		 },
	{ IDC_BTN_TIME_STAMP_HELP,ResourceManager::SETTINGS_TIME_STAMP_HELP		 },
	{ IDC_SB_TIME_STAMPS,	  ResourceManager::SETTINGS_SB_TIME_STAMPS		 },
	{ 0,					  ResourceManager::SETTINGS_AUTO_AWAY			 }
};

PropPage::Item FulAppearancePage::items[] = {
	{ IDC_CHATBUFFERSIZE,	SettingsManager::CHATBUFFERSIZE,	PropPage::T_INT },
	{ IDC_DISPLAYTIME,		SettingsManager::POPUP_TIMEOUT,		PropPage::T_INT },
	{ IDC_MESSAGE_LENGTH,	SettingsManager::MAX_MSG_LENGTH,	PropPage::T_INT },
	{ IDC_EDIT_TIME_STAMPS, SettingsManager::TIME_STAMPS_FORMAT,PropPage::T_STR },
	{ 0,					0,									PropPage::T_END }
};

LRESULT FulAppearancePage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::read((HWND)*this, items);
	PropPage::translate((HWND)(*this), texts);

	return TRUE;
}

void FulAppearancePage::write() {
	PropPage::write((HWND)*this, items);
}

LRESULT FulAppearancePage::onTextColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CColorDialog dlg(SETTING(POPUP_TEXTCOLOR), CC_FULLOPEN);
	if(dlg.DoModal() == IDOK)
		SettingsManager::getInstance()->set(SettingsManager::POPUP_TEXTCOLOR, (int)dlg.GetColor());
	return 0;
}

LRESULT FulAppearancePage::onFont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	LOGFONT font;  
	WinUtil::decodeFont(Text::toT(SETTING(POPUP_FONT)), font);
	CFontDialog dlg(&font, CF_EFFECTS | CF_SCREENFONTS);
	if(dlg.DoModal() == IDOK){
		settings->set(SettingsManager::POPUP_FONT, Text::fromT(WinUtil::encodeFont(font)));
	}
	return 0;
}

LRESULT FulAppearancePage::onTimeStampHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	MessageBox(CTSTRING(HELP_TIME_STAMPS), CTSTRING(TIME_STAMPS_HELP_CAPTION), MB_OK | MB_ICONINFORMATION);

	return 0;
}