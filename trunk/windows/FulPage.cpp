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
	{ IDC_CHATBUFFERSIZE, SettingsManager::CHATBUFFERSIZE, PropPage::T_INT },
	{ IDC_DISPLAYTIME, SettingsManager::POPUP_TIMEOUT, PropPage::T_INT },
	{ 0, 0, PropPage::T_END }
};

LRESULT FulPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::read((HWND)*this, items);

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

	return TRUE;
}

void FulPage::write() {
	colorList.Detach();

	PropPage::write((HWND)*this, items);
}

LRESULT FulPage::onColorButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
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

	return 0;
}

LRESULT FulPage::onTextColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CColorDialog dlg(SETTING(POPUP_TEXTCOLOR), CC_FULLOPEN);
	if(dlg.DoModal() == IDOK)
		SettingsManager::getInstance()->set(SettingsManager::POPUP_TEXTCOLOR, (int)dlg.GetColor());
	return 0;
}

LRESULT FulPage::onFont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	LOGFONT font;  
	WinUtil::decodeFont(SETTING(POPUP_FONT), font);
	CFontDialog dlg(&font, CF_EFFECTS | CF_SCREENFONTS);
	if(dlg.DoModal() == IDOK){
		settings->set(SettingsManager::POPUP_FONT, WinUtil::encodeFont(font));
	}
	return 0;
}