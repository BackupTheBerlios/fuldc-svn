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

		//l�gg till str�ngarna som ska visas i comboboxen
		ctrlPriority.Attach(GetDlgItem(IDC_PRIORITY));
		ctrlPriority.AddString(CTSTRING(PAUSED));
		ctrlPriority.AddString(CTSTRING(LOWEST));
		ctrlPriority.AddString(CTSTRING(LOW));
		ctrlPriority.AddString(CTSTRING(NORMAL));
		ctrlPriority.AddString(CTSTRING(HIGH));
		ctrlPriority.AddString(CTSTRING(HIGHEST));

		//S�tt f�rvalt till normal
		ctrlPriority.SetCurSel(3);
	
		SetWindowText(CTSTRING(SET_PRIORITY));

		CenterWindow(GetParent());
		return FALSE;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if(wID == IDOK) {
			//spara s�kstr�ngen s� den kan h�mtas senare
			int len = ctrlSearch.GetWindowTextLength()+1;
			TCHAR* buf = new TCHAR[len];
			GetDlgItemText(IDC_SEARCH, buf, len);
			search = buf;
			delete[] buf;
			
			priority = ctrlPriority.GetCurSel();
		}else {
			//returnera -1 om anv�ndaren avbryter
			priority = -1;
			search = Util::emptyStringT;
		}

		EndDialog(wID);
		return 0;
	}

	//funktioner f�r att h�mta resultatet
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
