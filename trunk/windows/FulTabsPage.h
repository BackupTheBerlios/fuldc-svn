#ifndef FULTABSPAGE_H
#define FULTABSPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlcrack.h>
#include "PropPage.h"

class FulTabsPage: public CPropertyPage<IDD_FULTABSPAGE>, public PropPage
{
public:
	FulTabsPage(SettingsManager *s) : PropPage(s) {	
		SetTitle( CTSTRING(SETTINGS_FUL_TABS) );
		m_psp.dwFlags |= PSP_HASHELP;
	};
	~FulTabsPage() { };

	BEGIN_MSG_MAP(FulTabsPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		MESSAGE_HANDLER(WM_HELP, onHelp)
		COMMAND_ID_HANDLER(IDC_COLOR_BUTTON, onColorButton)
		COMMAND_ID_HANDLER(IDC_CH_BLEND, onClickedBox)
		NOTIFY_CODE_HANDLER_EX(PSN_HELP, onHelpInfo)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onColorButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onClickedBox(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onHelpInfo(LPNMHDR /*pnmh*/);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();

protected:
	static Item items[];
	static TextItem texts[];
	CComboBox colorList;
};

#endif // FULTABSPAGE_H