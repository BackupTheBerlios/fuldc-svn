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
	{ IDC_BTN_TIME_STAMP_HELP,ResourceManager::SETTINGS_TIME_STAMP_HELP		 },
	{ IDC_SB_TIME_STAMPS,	  ResourceManager::SETTINGS_SB_TIME_STAMPS		 },
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

	return TRUE;
}

void FulAppearancePage::write() {
	PropPage::write((HWND)*this, items);
}

LRESULT FulAppearancePage::onTimeStampHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	MessageBox(CTSTRING(HELP_TIME_STAMPS), CTSTRING(TIME_STAMPS_HELP_CAPTION), MB_OK | MB_ICONINFORMATION);

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