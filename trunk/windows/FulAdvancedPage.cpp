/* 
* Copyright (C) 2002-2003 Opera, opera@home.se
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

#include "FulAdvancedPage.h"
#include "CommandDlg.h"
#include "WebShortcutsProperties.h"

#include "../client/SettingsManager.h"
#include "../client/HubManager.h"

PropPage::TextItem FulAdvancedPage::texts[] = {
	{ IDC_WEB_SHORTCUTS_ADD,		ResourceManager::ADD						},
	{ IDC_WEB_SHORTCUTS_REMOVE,		ResourceManager::REMOVE						},
	{ IDC_WEB_SHORTCUTS_PROPERTIES, ResourceManager::PROPERTIES					},
	{ IDC_SB_WEB_SHORTCUTS,			ResourceManager::SETTINGS_SB_WEB_SHORTCUTS	},
	{ 0,							ResourceManager::SETTINGS_AUTO_AWAY			}
};
PropPage::Item FulAdvancedPage::items[] = {
	{ 0, 0, PropPage::T_END }
};

FulAdvancedPage::ListItem FulAdvancedPage::listItems[] = {
	{ SettingsManager::EXPAND_QUEUE,					ResourceManager::EXPAND_QUEUE				},
	{ SettingsManager::STRIP_ISP,						ResourceManager::STRIP_ISP					},
	{ SettingsManager::STRIP_ISP_PM,					ResourceManager::STRIP_ISP_PM				},
	{ SettingsManager::HIGH_PRIO_SAMPLE,				ResourceManager::HIGH_PRIO_SAMPLE			},
	{ SettingsManager::ROTATE_LOG,						ResourceManager::ROTATE_LOG					},
	{ SettingsManager::HUBFRAME_CONFIRMATION,			ResourceManager::HUBFRAME_CONFIRMATION		},
	{ SettingsManager::QUEUE_REMOVE_CONFIRMATION,		ResourceManager::QUEUE_REMOVE_CONFIRMATION	},
	{ SettingsManager::CUSTOM_SOUND,					ResourceManager::CUSTOM_SOUND				},
	{ SettingsManager::REMOVE_TOPIC,					ResourceManager::REMOVE_TOPIC				}, 
	{ SettingsManager::TOGGLE_ACTIVE_WINDOW,			ResourceManager::TOGGLE_ACTIVE_WINDOW		},
	{ SettingsManager::DROP_STUPID_CONNECTION,			ResourceManager::DROP_STUPID_CONNECTION		},
	{ 0,												ResourceManager::SETTINGS_AUTO_AWAY			}
};

LRESULT FulAdvancedPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::read((HWND)*this, items, listItems, GetDlgItem(IDC_FUL_ADVANCED_BOOLEANS));
	PropPage::translate((HWND)(*this), texts);
	
	wsList = WebShortcuts::getInstance()->copyList();

	CRect rc;
	ctrlWebShortcuts.Attach(GetDlgItem(IDC_WEB_SHORTCUTS_LIST));
	ctrlWebShortcuts.GetClientRect(rc);
	rc.right -= GetSystemMetrics(SM_CXVSCROLL);
	ctrlWebShortcuts.InsertColumn(0, CTSTRING(SETTINGS_NAME), LVCFMT_LEFT, rc.Width() / 5, 0);
	ctrlWebShortcuts.InsertColumn(1, CTSTRING(KEY), LVCFMT_LEFT, rc.Width() / 5, 1);
	ctrlWebShortcuts.InsertColumn(2, CTSTRING(URL), LVCFMT_LEFT, rc.Width() * 3 / 5, 2);
	ctrlWebShortcuts.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	for (WebShortcut::Iter i = wsList.begin(); i != wsList.end(); ++i) {
		WebShortcut* ws = *i;
		addListItem(ws);
	}

	// Do specialized reading here
	return TRUE;
}

void FulAdvancedPage::write() {
	WebShortcuts::getInstance()->replaceList(wsList);
	
	PropPage::write((HWND)*this, items, listItems, GetDlgItem(IDC_FUL_ADVANCED_BOOLEANS));
}

LRESULT FulAdvancedPage::onClickedShortcuts(WORD /* wNotifyCode */, WORD wID, HWND /* hWndCtl */, BOOL& /* bHandled */) {
	if (wID == IDC_WEB_SHORTCUTS_ADD) {
		WebShortcut* ws;
		ws = new WebShortcut();
		WebShortcutsProperties wsp(wsList, ws);
		if (wsp.DoModal() == IDOK) {
			wsList.push_back(ws);
			addListItem(ws);
		} else {
			delete ws;
		}
	} else if (wID == IDC_WEB_SHORTCUTS_PROPERTIES) {
		if (ctrlWebShortcuts.GetSelectedCount() == 1) {
			int sel = ctrlWebShortcuts.GetSelectedIndex();
			WebShortcut* ws = wsList[sel];
			WebShortcutsProperties wsp(wsList, ws);
			if (wsp.DoModal() == IDOK) {
				updateListItem(sel);
			}
		}
	} else if (wID == IDC_WEB_SHORTCUTS_REMOVE) {
		if (ctrlWebShortcuts.GetSelectedCount() == 1) {
			int sel = ctrlWebShortcuts.GetSelectedIndex();
			dcassert(sel >= 0 && sel < (int)wsList.size());

			wsList.erase(find(wsList.begin(), wsList.end(), wsList[sel]));
			ctrlWebShortcuts.DeleteItem(sel);
		}
	}
	return S_OK;
}
LRESULT FulAdvancedPage::onSelChangeShortcuts(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */) {
	CButton cbnProp, cbnRemove;
	cbnProp.Attach(GetDlgItem(IDC_WEB_SHORTCUTS_PROPERTIES));
	cbnRemove.Attach(GetDlgItem(IDC_WEB_SHORTCUTS_REMOVE));

	dcassert(ctrlWebShortcuts.IsWindow());
	if (ctrlWebShortcuts.GetSelectedCount() == 1) {
		cbnProp.EnableWindow(TRUE);
		cbnRemove.EnableWindow(TRUE);
	} else {
		cbnProp.EnableWindow(FALSE);
		cbnRemove.EnableWindow(FALSE);
	}
	return S_OK;
}

LRESULT FulAdvancedPage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_FULADVANCEDPAGE);
	return 0;
}

LRESULT FulAdvancedPage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_FULADVANCEDPAGE);
	return 0;
}