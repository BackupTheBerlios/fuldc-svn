#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "../client/SettingsManager.h"

#include "FulPage.h"
#include "WinUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::Item FulPage::items[] = {
	{ IDC_SKIPLIST_DOWNLOAD, SettingsManager::SKIPLIST_DOWNLOAD, PropPage::T_STR },
	{ IDC_SKIPLIST_SHARE, SettingsManager::SKIPLIST_SHARE, PropPage::T_STR },
	{ IDC_INCOMING_REFRESH_TIME, SettingsManager::INCOMING_REFRESH_TIME, PropPage::T_INT },
	{ IDC_SHARE_REFRESH_TIME, SettingsManager::SHARE_REFRESH_TIME, PropPage::T_INT },
	{ 0, 0, PropPage::T_END }
};

LRESULT FulPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::read((HWND)*this, items);

	return TRUE;
}

void FulPage::write() {
	PropPage::write((HWND)*this, items);
}
