#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "../client/SettingsManager.h"

#include "fulappearance.h"
#include "WinUtil.h"

PropPage::TextItem FulAppearancePage::texts[] = {
	{ IDC_SB_CHATBUFFERSIZE,  ResourceManager::SETTINGS_SB_CHATBUFFERSIZE	 },
	{ IDC_ST_CHATBUFFERSIZE,  ResourceManager::SETTINGS_ST_CHATBUFFERSIZE	 },
	{ IDC_BTN_FONT,			  ResourceManager::SETTINGS_BTN_FONT			 },
	{ IDC_BTN_TEXTCOLOR,	  ResourceManager::SETTINGS_BTN_TEXTCOLOR		 },
	{ IDC_DUPE_COLOR,		  ResourceManager::SETTINGS_BTN_COLOR			 },
	{ IDC_DUPES,			  ResourceManager::SETTINGS_DUPES				 },
	{ IDC_DUPE_DESCRIPTION,	  ResourceManager::SETTINGS_DUPE_DESCRIPTION	 },
	{ IDC_NOTTH_BOX,		  ResourceManager::SETTINGS_NOTTH_BOX			 },
	{ IDC_NOTTH,			  ResourceManager::SETTINGS_BTN_COLOR			 },
	{ IDC_NOTTH_DESCRIPTION,  ResourceManager::SETTINGS_NOTTH_DESCRIPTION	 },

	{ 0,					  ResourceManager::SETTINGS_AUTO_AWAY			 }
};

PropPage::Item FulAppearancePage::items[] = {
	{ IDC_CHATBUFFERSIZE,	SettingsManager::CHATBUFFERSIZE,	PropPage::T_INT },
	{ IDC_EDIT_TIME_STAMPS, SettingsManager::TIME_STAMPS_FORMAT,PropPage::T_STR },
	{ 0,					0,									PropPage::T_END }
};

LRESULT FulAppearancePage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::read((HWND)*this, items);
	PropPage::translate((HWND)(*this), texts);

	dupeColor = SETTING(DUPE_COLOR);
	noTTHColor = SETTING(NO_TTH_COLOR);

	return TRUE;
}

void FulAppearancePage::write() {
	PropPage::write((HWND)*this, items);

	settings->set(SettingsManager::DUPE_COLOR, static_cast<int>(dupeColor));
	settings->set(SettingsManager::NO_TTH_COLOR, static_cast<int>(noTTHColor));
}

LRESULT FulAppearancePage::onDupeColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CColorDialog c(dupeColor, CC_FULLOPEN);
	if( c.DoModal() == IDOK )
		dupeColor = c.GetColor();

	return 0;
}

LRESULT FulAppearancePage::onNoTTHColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CColorDialog c(noTTHColor, CC_FULLOPEN);
	if( c.DoModal() == IDOK )
		noTTHColor = c.GetColor();

	return 0;
}

LRESULT FulAppearancePage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_FULAPPEARANCEPAGE);
	return 0;
}

LRESULT FulAppearancePage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_FULAPPEARANCEPAGE);
	return 0;
}