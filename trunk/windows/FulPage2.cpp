#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "../client/SettingsManager.h"

#include "FulPage2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::Item FulPage2::items[] = {
	{ IDC_FREE_SLOTS_EXTENSIONS, SettingsManager::FREE_SLOTS_EXTENSIONS, PropPage::T_STR }, 
	{ IDC_FREE_SLOTS_SIZE, SettingsManager::FREE_SLOTS_SIZE, PropPage::T_INT }, 
	{ 0, 0, PropPage::T_END }
};

LRESULT FulPage2::onInitDialog(UINT, WPARAM, LPARAM, BOOL&){
	PropPage::read((HWND)*this, items);

	return 0;
}

void FulPage2::write() {
	PropPage::write((HWND)*this, items);
}