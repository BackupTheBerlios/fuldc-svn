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

#ifndef FULADVANCEDPAGE_H
#define FULADVANCEDPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WebShortcuts.h"

#include "PropPage.h"
#include "ExListViewCtrl.h"

class FulAdvancedPage : public CPropertyPage<IDD_FULADVANCEDPAGE>, public PropPage
{
public:
	FulAdvancedPage(SettingsManager *s) : PropPage(s) { 
		SetTitle(CSTRING(SETTINGS_FUL_ADVANCED));
	};

	~FulAdvancedPage() { 
		for (WebShortcut::Iter i = wsList.begin(); i != wsList.end(); ++i)
			delete *i;
		
		ctrlWebShortcuts.Detach();
	};

	BEGIN_MSG_MAP(FulAdvancedPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_HANDLER(IDC_WEB_SHORTCUTS_ADD, BN_CLICKED, onClickedShortcuts)
		COMMAND_HANDLER(IDC_WEB_SHORTCUTS_PROPERTIES, BN_CLICKED, onClickedShortcuts)
		COMMAND_HANDLER(IDC_WEB_SHORTCUTS_REMOVE, BN_CLICKED, onClickedShortcuts)
		COMMAND_HANDLER(IDC_WEB_SHORTCUTS_LIST, LVN_ITEMCHANGED, onSelChangeShortcuts)
		COMMAND_HANDLER(IDC_WEB_SHORTCUTS_LIST, LVN_ITEMCHANGING, onSelChangeShortcuts)
		COMMAND_HANDLER(IDC_WEB_SHORTCUTS_LIST, LVN_ITEMACTIVATE, onSelChangeShortcuts)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT onClickedShortcuts(WORD /* wNotifyCode */, WORD wID, HWND /* hWndCtl */, BOOL& /* bHandled */);
	LRESULT onSelChangeShortcuts(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
	
	void updateListItem(int pos) {
		dcassert(pos >= 0 && (unsigned int)pos < wsList.size());
		ctrlWebShortcuts.SetItemText(pos, 0, wsList[pos]->name.c_str());
		ctrlWebShortcuts.SetItemText(pos, 1, wsList[pos]->key.c_str());
		ctrlWebShortcuts.SetItemText(pos, 2, wsList[pos]->url.c_str());
	}
	void addListItem(WebShortcut* ws) {
		StringList cols;
		cols.push_back(ws->name);
		cols.push_back(ws->key);
		cols.push_back(ws->url);
		ctrlWebShortcuts.insert(cols);
		cols.clear();
	}

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();

protected:
	static Item items[];
	static TextItem texts[];
	static ListItem listItems[];

	ExListViewCtrl ctrlWebShortcuts;
	WebShortcut::List wsList;
};

#endif //FULADVANCEDPAGE_H