#ifndef FULAPPEARANCEPAGE_H
#define FULAPPEARANCEPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlcrack.h>
#include "PropPage.h"

class FulAppearancePage : public CPropertyPage<IDD_FULAPPEARANCEPAGE>, public PropPage
{
public:
	FulAppearancePage(SettingsManager *s) : PropPage(s) {	
		SetTitle( CTSTRING(SETTINGS_FUL_APPEARANCE) );
		m_psp.dwFlags |= PSP_HASHELP;
	};
	~FulAppearancePage() {};

	BEGIN_MSG_MAP(FulAppearancePage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		MESSAGE_HANDLER(WM_HELP, onHelp)
		COMMAND_ID_HANDLER(IDC_BTN_TIME_STAMP_HELP, onTimeStampHelp)
		NOTIFY_CODE_HANDLER_EX(PSN_HELP, onHelpInfo)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onTimeStampHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onHelpInfo(LPNMHDR /*pnmh*/);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();

protected:
	static Item items[];
	static TextItem texts[];
};

#endif //FULAPPEARANCEPAGE_H