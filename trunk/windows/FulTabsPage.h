#ifndef FULTABSPAGE_H
#define FULTABSPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropPage.h"

class FulTabsPage: public CPropertyPage<IDD_FULTABSPAGE>, public PropPage
{
public:
	FulTabsPage(SettingsManager *s) : PropPage(s) {	
		SetTitle( CTSTRING(SETTINGS_FUL_TABS) );
	};
	~FulTabsPage() { };

	BEGIN_MSG_MAP(FulTabsPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_COLOR_BUTTON, onColorButton)
		COMMAND_ID_HANDLER(IDC_CH_BLEND, onClickedBox)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onColorButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onClickedBox(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();

protected:
	static Item items[];
	static TextItem texts[];
	CComboBox colorList;
};

#endif // FULTABSPAGE_H