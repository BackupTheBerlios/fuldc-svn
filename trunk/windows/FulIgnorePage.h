#ifndef FULIGNOREPAGE_H
#define FULIGNOREPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlcrack.h>
#include "ExListViewCtrl.h"
#include "PropPage.h"

class FulIgnorePage: public CPropertyPage<IDD_FULIGNOREPAGE>, public PropPage
{
public:
	FulIgnorePage(SettingsManager *s) : PropPage(s) { 
		SetTitle( CTSTRING(SETTINGS_FUL_IGNORE) );
		m_psp.dwFlags |= PSP_HASHELP;
	};

	~FulIgnorePage(void) {
		patternList.Detach();
	}

	BEGIN_MSG_MAP(FulIgnorePage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		MESSAGE_HANDLER(WM_HELP, onHelp)
		COMMAND_ID_HANDLER(IDC_ADD, onAdd)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		NOTIFY_CODE_HANDLER_EX(PSN_HELP, onHelpInfo)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT onAdd(WORD /* wNotifyCode */, WORD /*wID*/, HWND /* hWndCtl */, BOOL& /* bHandled */);
	LRESULT onRemove(WORD /* wNotifyCode */, WORD /*wID*/, HWND /* hWndCtl */, BOOL& /* bHandled */);
	LRESULT onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onHelpInfo(LPNMHDR /*pnmh*/);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();
private:
	static TextItem texts[];
	ExListViewCtrl patternList;
	StringSet patterns;
};

#endif //FULIGNOREPAGE_H