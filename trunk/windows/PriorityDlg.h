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
