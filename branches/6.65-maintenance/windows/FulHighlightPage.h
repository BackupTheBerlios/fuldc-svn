/* 
* Copyright (C) 2003-2006 P�r Bj�rklund, per.bjorklund@gmail.com
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

#if !defined(HIGHLIGHTPAGE_H)
#define HIGHLIGHTPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlcrack.h>
#include "PropPage.h"
#include "ExListViewCtrl.h"
#include "../client/SettingsManager.h"
#include "../client/HighlightManager.h"

class FulHighlightPage: public CPropertyPage<IDD_HIGHLIGHTPAGE>, public PropPage
{
	public:
	FulHighlightPage(SettingsManager *s) : PropPage(s) {
		SetTitle( CTSTRING(SETTINGS_FUL_HIGHLIGHT) );
		m_psp.dwFlags |= PSP_HASHELP | PSP_RTLREADING;
	};
	virtual ~FulHighlightPage();

	BEGIN_MSG_MAP(FulHighlightPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		MESSAGE_HANDLER(WM_HELP, onHelp)
		MESSAGE_HANDLER(WM_MENUCOMMAND, onMenuCommand)
		COMMAND_ID_HANDLER(IDC_ADD, onAdd)
		COMMAND_ID_HANDLER(IDC_DELETE, onDelete)
		COMMAND_ID_HANDLER(IDC_UPDATE, onUpdate)
		COMMAND_ID_HANDLER(IDC_MOVEUP, onMove)
		COMMAND_ID_HANDLER(IDC_MOVEDOWN, onMove)
		COMMAND_ID_HANDLER(IDC_PRESET, onPreset)
		NOTIFY_CODE_HANDLER_EX(PSN_HELP, onHelpInfo)
		NOTIFY_HANDLER(IDC_ITEMS, LVN_KEYDOWN, onKeyDown)
		NOTIFY_HANDLER(IDC_ITEMS, NM_DBLCLK, onDoubleClick)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onMenuCommand(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	
	LRESULT onAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onMove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onPreset(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onDoubleClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	
	LRESULT onHelpInfo(LPNMHDR /*pnmh*/);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();

	void getValues(ColorSettings* );
protected:
	static TextItem texts[];
	
	ExListViewCtrl ctrlStrings;

	//reset all controls except listview
	void addPreset(int preset);

	ColorList highlights;

	CMenu presets;
	
};

#endif //HIGHLIGHTPAGE_H
