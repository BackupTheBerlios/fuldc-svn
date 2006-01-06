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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "NotepadFrame.h"
#include "WinUtil.h"
#include "../client/File.h"

LRESULT NotepadFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	//needs to be set before calling create otherwise the menu won't be created correctly
	ctrlPad.unsetFlag( CFulEditCtrl::URL_SINGLE_CLICK );
	ctrlPad.setFlag( CFulEditCtrl::MENU_PASTE | CFulEditCtrl::URL_DOUBLE_CLICK );

	ctrlPad.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL, WS_EX_CLIENTEDGE);
	
	ctrlPad.LimitText(0);
	ctrlPad.SetFont(WinUtil::font);
	ctrlPad.SetTextColor(WinUtil::textColor);
	ctrlPad.SetBackgroundColor(WinUtil::bgColor);
	ctrlPad.unsetFlag(CFulEditCtrl::POPUP | CFulEditCtrl::SOUND | CFulEditCtrl::TAB | CFulEditCtrl::STRIP_ISP |
		CFulEditCtrl::HANDLE_SCROLL );

	ctrlPad.SendMessage(EM_SETEDITSTYLE, SES_EMULATESYSEDIT, SES_EMULATESYSEDIT);
	
	string tmp;
	try {
		tmp = File(Util::getAppPath() + "Notepad.txt", File::READ, File::OPEN).read();
	} catch(const FileException&) {
		// ...
	}
	
	if(tmp.empty()) {
		tmp = SETTING(NOTEPAD_TEXT);
		if(!tmp.empty()) {
			dirty = true;
			SettingsManager::getInstance()->set(SettingsManager::NOTEPAD_TEXT, Util::emptyString);
		}
	}

	ctrlPad.SetWindowText( Text::acpToWide( tmp ).c_str() );
	ctrlPad.EmptyUndoBuffer();
	
	WinUtil::SetIcon(m_hWnd, _T("notepad.ico"));

	bHandled = FALSE;
	return 1;
}

LRESULT NotepadFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	checkButton(false);

	if(dirty || ctrlPad.GetModify()) {
		AutoArray<TCHAR> buf(ctrlPad.GetWindowTextLength() + 1);
		ctrlPad.GetWindowText(buf, ctrlPad.GetWindowTextLength() + 1);
		try {
			string tmp( Text::wideToAcp(  tstring( buf, ctrlPad.GetWindowTextLength() ) ) );
			File(Util::getAppPath() + "Notepad.txt", File::WRITE, File::CREATE | File::TRUNCATE).write(tmp);
		} catch(const FileException&) {
			// Oops...
		}
	}

	bHandled = FALSE;
	return 0;
	
}

LRESULT NotepadFrame::onContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if(reinterpret_cast<HWND>(wParam) == ctrlPad) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

		if( pt.x == -1 && pt.y == -1 )
			WinUtil::getContextMenuPos(ctrlPad, pt);

		ctrlPad.ShowMenu(m_hWnd, pt);
		bHandled = TRUE;
	}else {
		bHandled = FALSE;
	}
	return 0;
}

void NotepadFrame::UpdateLayout(BOOL /*bResizeBars*/ /* = TRUE */)
{
	CRect rc;

	GetClientRect(rc);
	
	rc.bottom -= 1;
	rc.top += 1;
	rc.left +=1;
	rc.right -=1;
	ctrlPad.MoveWindow(rc);
	
}

/**
 * @file
 * $Id: NotepadFrame.cpp,v 1.2 2004/01/06 01:52:12 trem Exp $
 */


