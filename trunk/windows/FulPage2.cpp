#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "../client/SettingsManager.h"
#include "../client/StringTokenizer.h"
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
	{ IDC_MAX_SOURCES, SettingsManager::MAX_AUTO_MATCH_SOURCES, PropPage::T_INT }, 
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
	char buf[2000];
	string tmp;
	GetDlgItemText(IDC_PATHS, buf, 2000);
	StringTokenizer t(buf, '|');
	StringList l = t.getTokens();

	for(StringIter i = l.begin(); i != l.end(); ++i){
		if((*i)[i->length()-1] == '\\')
			tmp += (*i) + "|";
		else
			tmp += (*i) + "\\|";
	}

	SetDlgItemText(IDC_PATHS, tmp.c_str());
	
	PropPage::write((HWND)*this, items);
}