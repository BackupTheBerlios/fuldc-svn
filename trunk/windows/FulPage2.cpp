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