#ifndef FULPAGE2_H
#define FULPAGE2_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropPage.h"

class FulPage2 : public CPropertyPage<IDD_FULPAGE2>, public PropPage
{
public:
	FulPage2(SettingsManager *s) : PropPage(s) {
		SetTitle(CSTRING(SETTINGS_FUL_MORE));
	};
	~FulPage2() {};

	BEGIN_MSG_MAP(FulPage2)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_BROWSE, onBrowse)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT onBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();
	virtual void setTitle(const string& t) { SetTitle(t.c_str()); };

protected:
	static Item items[];
};

#endif
