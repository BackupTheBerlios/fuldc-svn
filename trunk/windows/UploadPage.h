/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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

#ifndef UPLOADPAGE_H
#define UPLOADPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropPage.h"
#include "ExListViewCtrl.h"

class UploadPage : public CPropertyPage<IDD_UPLOADPAGE>, public PropPage
{
public:
	UploadPage(SettingsManager *s) : PropPage(s) { 
		SetTitle(CTSTRING(SETTINGS_UPLOADS));
	};
	~UploadPage() {
		ctrlDirectories.Detach();
		ctrlTotal.Detach();
	};

	BEGIN_MSG_MAP(UploadPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		MESSAGE_HANDLER(WM_DROPFILES, onDropFiles)
		NOTIFY_HANDLER(IDC_DIRECTORIES, LVN_ITEMCHANGED, onItemchangedDirectories)
		COMMAND_ID_HANDLER(IDC_ADD, onClickedAdd)
		COMMAND_ID_HANDLER(IDC_REMOVE, onClickedRemove)
		COMMAND_ID_HANDLER(IDC_SHAREHIDDEN, onClickedShareHidden)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT onDropFiles(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT onItemchangedDirectories(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onClickedAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onClickedRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onClickedShareHidden(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	virtual void write();
	virtual void setTitle(const tstring& t) { SetTitle(t.c_str()); };
	
protected:
	static Item items[];
	static TextItem texts[];
	ExListViewCtrl ctrlDirectories;
	CStatic ctrlTotal;

	void addDirectory(tstring path);
};

#endif //UPLOADPAGE_H

/**
 * @file
 * $Id: UploadPage.h,v 1.2 2003/12/30 13:31:47 trem Exp $
 */

