#ifndef FULPAGE_H
#define FULPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropPage.h"

class FulSharePage : public CPropertyPage<IDD_FULSHAREPAGE>, public PropPage
{
public:
	FulSharePage(SettingsManager *s) : PropPage(s) {
		SetTitle( CTSTRING(SETTINGS_FUL_SHARE) );
	};
	~FulSharePage() {};

	BEGIN_MSG_MAP(FulSharePage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_REFRESH_INCOMING, onSetCheck)
		COMMAND_ID_HANDLER(IDC_REFRESH_SHARE, onSetCheck)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onSetCheck(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		
	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();
	
protected:
	static Item items[];
	static TextItem texts[];
};

#endif //FULPAGE_H