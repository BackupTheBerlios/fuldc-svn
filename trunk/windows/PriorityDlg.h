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

#pragma once

class PriorityDlg: public CDialogImpl<PriorityDlg>
{
public:
	enum { IDD = IDD_PRIORITY };
	
	BEGIN_MSG_MAP(PriorityDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	PriorityDlg(){}
	~PriorityDlg(){}

	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlSearch.SetFocus();
		return FALSE;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		ctrlSearch.Attach(GetDlgItem(IDC_SEARCH));
		ctrlSearch.SetFocus();

		//lägg till strängarna som ska visas i comboboxen
		ctrlPriority.Attach(GetDlgItem(IDC_PRIORITY));
		ctrlPriority.AddString(CTSTRING(PAUSED));
		ctrlPriority.AddString(CTSTRING(LOWEST));
		ctrlPriority.AddString(CTSTRING(LOW));
		ctrlPriority.AddString(CTSTRING(NORMAL));
		ctrlPriority.AddString(CTSTRING(HIGH));
		ctrlPriority.AddString(CTSTRING(HIGHEST));

		//Sätt förvalt till normal
		ctrlPriority.SetCurSel(3);
	
		SetWindowText(CTSTRING(SET_PRIORITY));

		CenterWindow(GetParent());
		return FALSE;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if(wID == IDOK) {
			//spara söksträngen så den kan hämtas senare
			int len = ctrlSearch.GetWindowTextLength()+1;
			TCHAR* buf = new TCHAR[len];
			GetDlgItemText(IDC_SEARCH, buf, len);
			search = buf;
			delete[] buf;
			
			priority = ctrlPriority.GetCurSel();
		}else {
			//returnera -1 om användaren avbryter
			priority = -1;
			search = Util::emptyStringT;
		}

		EndDialog(wID);
		return 0;
	}

	//funktioner för att hämta resultatet
	int GetPriority(){
		return priority;
	}

	tstring GetSearch() {
		return search;
	}

private:
	CEdit ctrlSearch;
	CComboBox ctrlPriority;

	int priority;
	tstring search;
};
