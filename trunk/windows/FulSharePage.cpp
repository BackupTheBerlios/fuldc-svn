/* 
* Copyright (C) 2003-2005 Pär Björklund, per.bjorklund@gmail.com
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "../client/SettingsManager.h"

#include "FulSharePage.h"
#include "WinUtil.h"

PropPage::TextItem FulSharePage::texts[] = {
	{ IDC_REFRESH_INCOMING,			ResourceManager::SETTINGS_AUTO_UPDATE_INCOMING			},
	{ IDC_REFRESH_SHARE,			ResourceManager::SETTINGS_AUTO_UPDATE_LIST				},
	{ IDC_SB_SKIPLIST_SHARE,		ResourceManager::SETTINGS_SB_SKIPLIST_SHARE				},
	{ IDC_SB_REFRESH,				ResourceManager::SETTINGS_SB_REFRESH					},
	{ IDC_ST_SKIPLIST_SHARE,		ResourceManager::SETTINGS_ST_SKIPLIST_SHARE				},
	{ IDC_SB_MINISLOTS,				ResourceManager::SETTINGS_SB_MINISLOTS					},
	{ IDC_ST_MINISLOTS_EXT,			ResourceManager::SETTINGS_ST_MINISLOTS_EXT				},
	{ IDC_ST_MINISLOTS_SIZE,		ResourceManager::SETTINGS_ST_MINISLOTS_SIZE				},
	{ IDC_ONLY_REFRESH_INCOMING,	ResourceManager::SETTINGS_REFRESH_INCOMING_BETWEEN		},
	{ IDC_ONLY_REFRESH_SHARE,		ResourceManager::SETTINGS_REFRESH_SHARE_BETWEEN			},
	{ 0,							ResourceManager::SETTINGS_AUTO_AWAY						},
};
PropPage::Item FulSharePage::items[] = {
	{ IDC_SKIPLIST_SHARE,		 SettingsManager::SKIPLIST_SHARE,					PropPage::T_STR  },
	{ IDC_INCOMING_REFRESH_TIME, SettingsManager::INCOMING_REFRESH_TIME,			PropPage::T_INT  },
	{ IDC_SHARE_REFRESH_TIME,	 SettingsManager::SHARE_REFRESH_TIME,				PropPage::T_INT  },
	{ IDC_REFRESH_SHARE,		 SettingsManager::AUTO_UPDATE_LIST,					PropPage::T_BOOL }, 
	{ IDC_REFRESH_INCOMING,		 SettingsManager::AUTO_UPDATE_INCOMING,				PropPage::T_BOOL },
	{ IDC_FREE_SLOTS_EXTENSIONS, SettingsManager::FREE_SLOTS_EXTENSIONS,			PropPage::T_STR  }, 
	{ IDC_FREE_SLOTS_SIZE,		 SettingsManager::FREE_SLOTS_SIZE,					PropPage::T_INT  }, 
	{ IDC_ONLY_REFRESH_INCOMING, SettingsManager::REFRESH_INCOMING_BETWEEN,			PropPage::T_BOOL },
	{ IDC_ONLY_REFRESH_SHARE,	 SettingsManager::REFRESH_SHARE_BETWEEN,			PropPage::T_BOOL },
	{ 0,						 0,													PropPage::T_END  }
};

LRESULT FulSharePage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::read((HWND)*this, items);
	PropPage::translate((HWND)(*this), texts);

	incomingBegin.Attach(GetDlgItem(IDC_ONLY_REFRESH_INCOMING_TIME1));
	incomingEnd.Attach(GetDlgItem(IDC_ONLY_REFRESH_INCOMING_TIME2));

	shareBegin.Attach(GetDlgItem(IDC_ONLY_REFRESH_SHARE_TIME1));
	shareEnd.Attach(GetDlgItem(IDC_ONLY_REFRESH_SHARE_TIME2));

	TCHAR buf[32];

	for(int i = 0; i < 24; ++i) {
		_stprintf(buf, 32, _T("%02d"), i);
		incomingBegin.AddString(buf);
		incomingEnd.AddString(buf);
		shareBegin.AddString(buf);
		shareEnd.AddString(buf);
	}

	incomingBegin.SetCurSel(SETTING(REFRESH_INCOMING_BEGIN));
	incomingEnd.SetCurSel(SETTING(REFRESH_INCOMING_END));
	shareBegin.SetCurSel(SETTING(REFRESH_SHARE_BEGIN));
	shareEnd.SetCurSel(SETTING(REFRESH_SHARE_END));

	
	BOOL b;
	onSetCheck(0, IDC_REFRESH_INCOMING, 0, b);
	onSetCheck(0, IDC_REFRESH_SHARE, 0, b);
	onSetCheck(0, IDC_ONLY_REFRESH_INCOMING, 0, b);
	onSetCheck(0, IDC_ONLY_REFRESH_SHARE, 0, b);

	return TRUE;
}

LRESULT FulSharePage::onSetCheck(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	bool show = (IsDlgButtonChecked(wID) == BST_CHECKED);
	
	if(wID == IDC_ONLY_REFRESH_INCOMING) {
		incomingBegin.EnableWindow(show);
		incomingEnd.EnableWindow(show);
	} else if(wID == IDC_ONLY_REFRESH_SHARE) {
		shareBegin.EnableWindow(show);
		shareEnd.EnableWindow(show);
	} else {
		int id = (wID == IDC_REFRESH_SHARE ? IDC_SHARE_REFRESH_TIME : IDC_INCOMING_REFRESH_TIME);

		CButton btn;
		btn.Attach(GetDlgItem(id));
		btn.EnableWindow(show);
		btn.Detach();
	} 

	return TRUE;
}
void FulSharePage::write() {
	PropPage::write((HWND)*this, items);

	settings->set(SettingsManager::REFRESH_INCOMING_BEGIN, incomingBegin.GetCurSel());
	settings->set(SettingsManager::REFRESH_INCOMING_END, incomingEnd.GetCurSel());
	settings->set(SettingsManager::REFRESH_SHARE_BEGIN, shareBegin.GetCurSel());
	settings->set(SettingsManager::REFRESH_SHARE_END, shareEnd.GetCurSel());
}

LRESULT FulSharePage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_FULSHAREPAGE);
	return 0;
}

LRESULT FulSharePage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_FULSHAREPAGE);
	return 0;
}