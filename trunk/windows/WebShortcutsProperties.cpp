/* 
 * Copyright (C) 2003 Opera, opera@home.se
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
#include "../client/ResourceManager.h"
#include "Resource.h"

#include "WebShortcutsProperties.h"
#include "WebShortcuts.h"

const string WebShortcutsProperties::badkeys = " refresh slots search odc odc++ nmdc dc++ away back zeros shutdown lastlog me join clear ts password showjoins close userlist connection favorite help pm ";

// Initialize dialog
LRESULT WebShortcutsProperties::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {

	SetWindowText(CSTRING(SETTINGS_WS_TITLE));
	SetDlgItemText(IDOK, CSTRING(DIALOG_OK));
	SetDlgItemText(IDCANCEL, CSTRING(DIALOG_CANCEL));
	SetDlgItemText(IDC_WEB_SHORTCUTS_NAME_DESC, CSTRING(SETTINGS_NAME));
	SetDlgItemText(IDC_WEB_SHORTCUT_KEY_DESC, CSTRING(KEY));
	SetDlgItemText(IDC_WEB_SHORTCUT_URL_DESC, CSTRING(URL));
	SetDlgItemText(IDC_WEB_SHORTCUTS_HOWTO, CSTRING(SETTINGS_WS_HOWTO));
	SetDlgItemText(IDC_WEB_SHORTCUTS_DESC, CSTRING(SETTINGS_WS_DESCR));

	SetDlgItemText(IDC_WEB_SHORTCUT_NAME, this->ws->name.c_str());
	SetDlgItemText(IDC_WEB_SHORTCUT_KEY,  this->ws->key.c_str());
	SetDlgItemText(IDC_WEB_SHORTCUT_URL,  this->ws->url.c_str());

	::SetFocus(GetDlgItem(IDC_WEB_SHORTCUT_NAME));

	// Center dialog
	//CenterWindow(GetParent());

	return FALSE;
}

// Exit dialog
LRESULT WebShortcutsProperties::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(wID == IDOK) {
		// Update search
		char buf[2048];

		GetDlgItemText(IDC_WEB_SHORTCUT_NAME, buf, 2048);
		string sName = buf;
		if (sName == "") {
			MessageBox(CSTRING(NAME_REQUIRED), sName.c_str(), MB_OK | MB_ICONEXCLAMATION);
			return 0;
		}
		WebShortcut* _ws = WebShortcuts::getShortcutByName(wslist, sName);
		if ( _ws != NULL && _ws != ws ) {
			MessageBox(CSTRING(NAME_ALREADY_IN_USE), sName.c_str(), MB_OK | MB_ICONEXCLAMATION);
			return 0;
		}

		GetDlgItemText(IDC_WEB_SHORTCUT_KEY, buf, 2048);
		string sKey = buf;
		// Check if key is busy
		if (sKey != "") {
			_ws = WebShortcuts::getShortcutByKey(wslist, sKey);
			if ( _ws != NULL && _ws != ws ) {
				MessageBox(CSTRING(KEY_ALREADY_IN_USE), (sName + " (" + sKey + ")").c_str(), MB_OK | MB_ICONEXCLAMATION);
				return 0;
			}
			if (badkeys.find(" " + sKey + " ") != string::npos) {
				MessageBox(CSTRING(KEY_ALREADY_IN_USE), (sName + " (" + sKey + ")").c_str(), MB_OK | MB_ICONEXCLAMATION);
				return 0;
			}
		}

		GetDlgItemText(IDC_WEB_SHORTCUT_URL, buf, 2048);
		string sUrl = buf;

		this->ws->name = sName;
		this->ws->key = sKey;
		this->ws->url = sUrl;
	}

	EndDialog(wID);
	return 0;
}
