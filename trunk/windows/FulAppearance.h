#ifndef FULAPPEARANCEPAGE_H
#define FULAPPEARANCEPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropPage.h"

class FulAppearancePage : public CPropertyPage<IDD_FULAPPEARANCEPAGE>, public PropPage
{
public:
	FulAppearancePage(SettingsManager *s) : PropPage(s) {	
		SetTitle(CSTRING(SETTINGS_FUL_APPEARANCE));
	};
	~FulAppearancePage() {};

	BEGIN_MSG_MAP(FulAppearancePage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_COLOR_BUTTON, onColorButton)
		COMMAND_ID_HANDLER(IDC_FONT, onFont)
		COMMAND_ID_HANDLER(IDC_TEXTCOLOR, onTextColor)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT onColorButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onTextColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onFont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();
	virtual void setTitle(const string& t) { SetTitle(t.c_str()); };

protected:
	static Item items[];
	static TextItem texts[];
	CComboBox colorList;
};

#endif //FULAPPEARANCEPAGE_H