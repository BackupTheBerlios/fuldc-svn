#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "../client/SettingsManager.h"

#include "FulTabsPage.h"


PropPage::TextItem FulTabsPage::texts[] = {
	{ IDC_CH_STATUS_HUB_BOLD,	ResourceManager::HUB_BOLD_TABS				},
	{ IDC_CH_STATUS_PM_BOLD,	ResourceManager::PM_BOLD_TABS				},
	{ IDC_CH_ICONS,				ResourceManager::TAB_SHOW_ICONS				},
	{ IDC_CH_BLEND,				ResourceManager::BLEND_TABS					},
	{ IDC_SB_TAB_COLORS,		ResourceManager::SETTINGS_SB_TAB_COLORS		},
	{ IDC_SB_TAB_SIZE,			ResourceManager::SETTINGS_SB_TAB_SIZE		},
	{ IDC_BTN_COLOR,			ResourceManager::SETTINGS_BTN_COLOR			},
	{ IDC_SB_TAB_DIRTY_BLEND,	ResourceManager::SETTINGS_SB_DIRTY_BLEND	},
	{ 0,						ResourceManager::SETTINGS_AUTO_AWAY			}
};

PropPage::Item FulTabsPage::items[] = {
	{ IDC_CH_STATUS_HUB_BOLD,	SettingsManager::HUB_BOLD_TABS,		PropPage::T_BOOL },
	{ IDC_CH_STATUS_PM_BOLD,	SettingsManager::PM_BOLD_TABS,		PropPage::T_BOOL },
	{ IDC_CH_ICONS,				SettingsManager::TAB_SHOW_ICONS,	PropPage::T_BOOL },
	{ IDC_CH_BLEND,				SettingsManager::BLEND_TABS,		PropPage::T_BOOL },
	{ IDC_TAB_SIZE,				SettingsManager::TAB_SIZE,			PropPage::T_INT }, 
	{ IDC_TAB_DIRTY_BLEND,		SettingsManager::TAB_DIRTY_BLEND,	PropPage::T_INT },
	{ 0,						0,									PropPage::T_END }
};

LRESULT FulTabsPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::read((HWND)*this, items);
	PropPage::translate((HWND)(*this), texts);

	colorList.Attach(GetDlgItem(IDC_COLOR_COMBO));

	colorList.AddString(CTSTRING(TAB_ACTIVE_BG));
	colorList.AddString(CTSTRING(TAB_ACTIVE_TEXT));
	colorList.AddString(CTSTRING(TAB_ACTIVE_BORDER));
	colorList.AddString(CTSTRING(TAB_INACTIVE_BG));
	colorList.AddString(CTSTRING(TAB_INACTIVE_TEXT));
	colorList.AddString(CTSTRING(TAB_INACTIVE_BORDER));
	colorList.AddString(CTSTRING(TAB_INACTIVE_BG_NOTIFY));
	colorList.AddString(CTSTRING(TAB_INACTIVE_BG_DISCONNECTED));
	colorList.SetCurSel(0);

	colorList.Detach();

	BOOL b;

	onClickedBox(0, IDC_CH_BLEND, 0, b );

	return TRUE;
}

void FulTabsPage::write() {
	PropPage::write((HWND)*this, items);
}

LRESULT FulTabsPage::onColorButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
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

LRESULT FulTabsPage::onClickedBox(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	int button = 0;
	
	//this seems utterly stupid now, but it's easier if i want/need to
	//add more items in the future =)
	switch(wID) {
	case IDC_CH_BLEND:		button = IDC_TAB_DIRTY_BLEND; break;
	}

	::EnableWindow( GetDlgItem(button), IsDlgButtonChecked(wID) == BST_CHECKED );

	return TRUE;
}