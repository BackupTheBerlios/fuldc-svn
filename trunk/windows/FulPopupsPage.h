#ifndef FULPOPUPSPAGE_H
#define FULPOPUPSPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropPage.h"

class FulPopupsPage: public CPropertyPage<IDD_FULPOPUPPAGE>, public PropPage
{
public:
	FulPopupsPage(SettingsManager *s) : PropPage(s) { 
		SetTitle( CTSTRING(SETTINGS_FUL_POPUP) );
	};

	~FulPopupsPage() { };

	BEGIN_MSG_MAP(FulPopupsPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();

protected:
	static Item items[];
	static TextItem texts[];
};

#endif //FULPOPUPSPAGE_H