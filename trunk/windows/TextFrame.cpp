/*
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
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

#include "TextFrame.h"
#include "WinUtil.h"
#include "../client/File.h"

#define MAX_TEXT_LEN 32768

void TextFrame::openWindow(const tstring& aFileName) {
	TextFrame* frame = new TextFrame(aFileName);
	frame->CreateEx(WinUtil::mdiClient);
}

void TextFrame::openWindow(deque<tstring>* aLog) {
	TextFrame* frame = new TextFrame(aLog);
	frame->CreateEx(WinUtil::mdiClient);
}

LRESULT TextFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	ctrlPad.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY, WS_EX_CLIENTEDGE);

	ctrlPad.LimitText(0);
	
	string tmp;

	m_hMenu = WinUtil::mainMenu;

	WinUtil::SetIcon(m_hWnd, _T("notepad.ico"));
	bHandled = FALSE;
	
	init = true;
	ctrlPad.unsetFlag(	CFulEditCtrl::HANDLE_SCROLL |
						CFulEditCtrl::POPUP |
						CFulEditCtrl::SOUND |
						CFulEditCtrl::TAB );
	

	if(file.empty()){
		SetWindowText(CTSTRING(HISTORY));
		ctrlPad.SetFont(WinUtil::font);
		ctrlPad.SetBackgroundColor(WinUtil::bgColor);
		ctrlPad.SetTextColor(WinUtil::textColor);
		deque<tstring>::iterator i = log->begin();
		for(; i != log->end(); ++i)
			ctrlPad.AddLine(*i);
		return 1;
	}

	try {
		tmp = File(Text::fromT(file), File::READ, File::OPEN).read(MAX_TEXT_LEN);
		string::size_type i = 0;
		while((i = tmp.find('\n', i)) != string::npos) {
			if(i == 0 || tmp[i-1] != '\r') {
				tmp.insert(i, 1, '\r');
				i++;
			}
			i++;
		}
		
		ctrlPad.SetFont(WinUtil::monoFont);
		ctrlPad.SetBackgroundColor(WinUtil::bgColor);
		ctrlPad.SetTextColor(WinUtil::textColor);
		ctrlPad.SetWindowText(Text::toT(tmp).c_str());
		ctrlPad.EmptyUndoBuffer();
		ctrlPad.ScrollToBeginning();
		SetWindowText(Text::toT(Util::getFileName(Text::fromT(file))).c_str());
	} catch(const FileException& e) {
		SetWindowText(Text::toT(Util::getFileName(Text::fromT(file)) + ": " + e.getError()).c_str());
	}

	bHandled = FALSE;
	return 1;
}

LRESULT TextFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

void TextFrame::UpdateLayout(BOOL /*bResizeBars*/ /* = TRUE */)
{
	CRect rc;

	GetClientRect(rc);

	rc.bottom -= 1;
	rc.top += 1;
	rc.left +=1;
	rc.right -=1;
	ctrlPad.MoveWindow(rc);
}

LRESULT TextFrame::onContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
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
