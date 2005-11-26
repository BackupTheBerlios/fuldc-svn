/*
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(TEXT_FRAME_H)
#define TEXT_FRAME_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlatTabCtrl.h"
#include "WinUtil.h"
#include "FulEditCtrl.h"

class TextFrame : public MDITabChildWindowImpl<TextFrame>
{
public:
	static void openWindow(const tstring& aFileName);
	static void openWindow(deque<tstring>* aLog);
	
	DECLARE_FRAME_WND_CLASS_EX(_T("TextFrame"), IDR_NOTEPAD, 0, COLOR_3DFACE);

	TextFrame(const tstring& fileName) : file(fileName){ }
	TextFrame(deque<tstring> *aLog) : log(aLog) {}
	
	virtual ~TextFrame() { }
	
	typedef MDITabChildWindowImpl<TextFrame> baseClass;
	BEGIN_MSG_MAP(TextFrame)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		MESSAGE_HANDLER(WM_SIZE, onSize)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_MENUCOMMAND, ctrlPad.onMenuCommand)
		COMMAND_ID_HANDLER(IDC_FIND, ctrlPad.onFind)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	void UpdateLayout(BOOL bResizeBars = TRUE);
		
	LRESULT onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		HWND hWnd = (HWND)lParam;
		HDC hDC = (HDC)wParam;
		if(hWnd == ctrlPad.m_hWnd) {
			::SetBkColor(hDC, WinUtil::bgColor);
			::SetTextColor(hDC, WinUtil::textColor);
			return (LRESULT)WinUtil::bgBrush;
		}
		bHandled = FALSE;
		return FALSE;
	};
	
	
	LRESULT OnFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlPad.SetFocus();
		return 0;
	}

	LRESULT onSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		if(init){
			ctrlPad.ScrollToBeginning();
			init = false;
			
		} else {
			ctrlPad.setFlag(CFulEditCtrl::HANDLE_SCROLL);
		}
		
		bHandled = FALSE;
		return 0;
	}
	
private:
	deque<tstring> *log;
	tstring file;
	CFulEditCtrl ctrlPad;
	bool init;
};

#endif // !defined(TEXT_FRAME_H)

/**
 * @file
 * $Id: TextFrame.h,v 1.3 2004/01/08 11:27:05 trem Exp $
 */
