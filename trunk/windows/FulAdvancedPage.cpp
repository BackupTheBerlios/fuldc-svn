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

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::Item FulAdvancedPage::items[] = {
	{ 0, 0, PropPage::T_END }
};

FulAdvancedPage::ListItem FulAdvancedPage::listItems[] = {
	{ SettingsManager::AUTO_UPDATE_INCOMING, ResourceManager::AUTO_UPDATE_INCOMING },
	{ SettingsManager::EXPAND_QUEUE, ResourceManager::EXPAND_QUEUE },
	{ SettingsManager::STRIP_ISP, ResourceManager::STRIP_ISP },
	{ SettingsManager::STRIP_ISP_PM, ResourceManager::STRIP_ISP_PM },
	{ SettingsManager::HUB_BOLD_TABS, ResourceManager::HUB_BOLD_TABS },
	{ SettingsManager::PM_BOLD_TABS, ResourceManager::PM_BOLD_TABS },
	{ SettingsManager::HIGH_PRIO_SAMPLE, ResourceManager::HIGH_PRIO_SAMPLE },
	{ SettingsManager::ROTATE_LOG, ResourceManager::ROTATE_LOG },
	{ SettingsManager::POPUP_AWAY, ResourceManager::POPUP_AWAY },
	{ SettingsManager::POPUP_ON_PM, ResourceManager::POPUP_ON_PM },
	{ SettingsManager::POPUP_ON_NEW_PM, ResourceManager::POPUP_ON_NEW_PM },
	{ SettingsManager::POPUP_ON_HUBSTATUS, ResourceManager::POPUP_ON_HUBSTATUS },
	{ SettingsManager::HUBFRAME_CONFIRMATION, ResourceManager::HUBFRAME_CONFIRMATION },
	{ SettingsManager::QUEUE_REMOVE_CONFIRMATION, ResourceManager::QUEUE_REMOVE_CONFIRMATION },
	{ SettingsManager::QUEUE_BOLD_TABS, ResourceManager::QUEUE_BOLD_TABS },
	{ SettingsManager::POPUNDER_PM, ResourceManager::POPUNDER_PM },
	{ SettingsManager::POPUNDER_DIRLIST, ResourceManager::POPUNDER_DIRLIST }, 
	{ SettingsManager::TAB_SHOW_ICONS, ResourceManager::TAB_SHOW_ICONS },
	{ SettingsManager::CUSTOM_SOUND, ResourceManager::CUSTOM_SOUND },
	{ SettingsManager::REMOVE_POPUPS, ResourceManager::REMOVE_POPUPS },
	{ SettingsManager::SHOW_TOPIC, ResourceManager::SHOW_TOPIC }, 
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

LRESULT FulAdvancedPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::read((HWND)*this, items, listItems, GetDlgItem(IDC_FUL_ADVANCED_BOOLEANS));
	
	wsList = WebShortcuts::getInstance()->copyList();

	CRect rc;
	ctrlWebShortcuts.Attach(GetDlgItem(IDC_WEB_SHORTCUTS_LIST));
	ctrlWebShortcuts.GetClientRect(rc);
	rc.right -= GetSystemMetrics(SM_CXVSCROLL);
	ctrlWebShortcuts.InsertColumn(0, CSTRING(SETTINGS_NAME), LVCFMT_LEFT, rc.Width() / 5, 0);
	ctrlWebShortcuts.InsertColumn(1, CSTRING(KEY), LVCFMT_LEFT, rc.Width() / 5, 1);
	ctrlWebShortcuts.InsertColumn(2, CSTRING(URL), LVCFMT_LEFT, rc.Width() * 3 / 5, 2);
	if(BOOLSETTING(FULL_ROW_SELECT))
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
	WebShortcuts::getInstance()->propagateChanges();

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
			//dcassert(sel >= 0 && sel < (int)wsList.size());
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