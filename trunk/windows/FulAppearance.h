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
		SetTitle( CTSTRING(SETTINGS_FUL_APPEARANCE) );
	};
	~FulAppearancePage() {};

	BEGIN_MSG_MAP(FulAppearancePage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		COMMAND_ID_HANDLER(IDC_FONT, onFont)
		COMMAND_ID_HANDLER(IDC_TEXTCOLOR, onTextColor)
		COMMAND_ID_HANDLER(IDC_BTN_TIME_STAMP_HELP, onTimeStampHelp)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT onTextColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onFont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onTimeStampHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();

protected:
	static Item items[];
	static TextItem texts[];
};

#endif //FULAPPEARANCEPAGE_H