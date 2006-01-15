/* 
* Copyright (C) 2003-2006 Pär Björklund, per.bjorklund@gmail.com
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#if !defined(FULAPPEARANCEPAGE_H)
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
		COMMAND_ID_HANDLER(IDC_DUPE_COLOR, onDupeColor)
		COMMAND_ID_HANDLER(IDC_NOTTH, onNoTTHColor)
		COMMAND_ID_HANDLER(IDC_SEARCH_HISTORY, onSearchHistory);
		COMMAND_ID_HANDLER(IDC_FILTER_HISTORY, onSearchHistory);
		NOTIFY_CODE_HANDLER_EX(PSN_HELP, onHelpInfo)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onDupeColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onNoTTHColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onSearchHistory(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onHelpInfo(LPNMHDR /*pnmh*/);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();

protected:
	DWORD dupeColor;
	DWORD noTTHColor;
	static Item items[];
	static TextItem texts[];
};

#endif //FULAPPEARANCEPAGE_H