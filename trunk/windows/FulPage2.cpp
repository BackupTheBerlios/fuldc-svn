#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "../client/SettingsManager.h"
#include "WinUtil.h"

#include "FulPage2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::Item FulPage2::items[] = {
	{ IDC_FREE_SLOTS_EXTENSIONS, SettingsManager::FREE_SLOTS_EXTENSIONS, PropPage::T_STR }, 
	{ IDC_FREE_SLOTS_SIZE, SettingsManager::FREE_SLOTS_SIZE, PropPage::T_INT }, 
	{ IDC_PATHS, SettingsManager::DOWNLOAD_TO_PATHS, PropPage::T_STR },
	{ IDC_TAB_SIZE, SettingsManager::TAB_SIZE, PropPage::T_INT }, 
	{ IDC_MAX_SOURCES, SettingsManager::MAX_AUTO_MATCH_SOURCES, PropPage::T_INT }, 
	{ IDC_DISPLAYTIME, SettingsManager::POPUP_TIMEOUT, PropPage::T_INT },
	{ IDC_MESSAGE_LENGTH, SettingsManager::MAX_MSG_LENGTH, PropPage::T_INT },
	{ 0, 0, PropPage::T_END }
};

LRESULT FulPage2::onInitDialog(UINT, WPARAM, LPARAM, BOOL&){
	PropPage::read((HWND)*this, items);

	return 0;
}

LRESULT FulPage2::onBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	string tmp;
	if(WinUtil::browseDirectory(tmp)){
		char buf[1000];
		GetDlgItemText(IDC_PATHS, buf, 1000);
		strncat(buf, tmp.c_str(), tmp.length());
		strncat(buf, "|", 1);
		SetDlgItemText(IDC_PATHS, buf);
	}

	return 0;
}

void FulPage2::write() {
	PropPage::write((HWND)*this, items);
}

LRESULT FulPage2::onTextColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CColorDialog dlg(SETTING(POPUP_TEXTCOLOR), CC_FULLOPEN);
	if(dlg.DoModal() == IDOK)
		SettingsManager::getInstance()->set(SettingsManager::POPUP_TEXTCOLOR, (int)dlg.GetColor());
	return 0;
}

LRESULT FulPage2::onFont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	LOGFONT font;  
	WinUtil::decodeFont(SETTING(POPUP_FONT), font);
	CFontDialog dlg(&font, CF_EFFECTS | CF_SCREENFONTS);
	if(dlg.DoModal() == IDOK){
		settings->set(SettingsManager::POPUP_FONT, WinUtil::encodeFont(font));
	}
	return 0;
}