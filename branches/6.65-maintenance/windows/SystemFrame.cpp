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

#include "SystemFrame.h"
#include "WinUtil.h"
#include "../client/File.h"

LRESULT SystemFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	ctrlPad.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY, WS_EX_CLIENTEDGE);

	ctrlPad.LimitText(0);
	ctrlPad.SetFont(WinUtil::font);
	ctrlPad.SetTextColor(WinUtil::textColor);
	ctrlPad.SetBackgroundColor(WinUtil::bgColor);
	ctrlPad.unsetFlag(CFulEditCtrl::POPUP | CFulEditCtrl::SOUND | CFulEditCtrl::TAB | CFulEditCtrl::STRIP_ISP );

	deque<pair<time_t, string> > oldMessages = LogManager::getInstance()->getLastLogs();
	// Technically, we might miss a message or two here, but who cares...
	LogManager::getInstance()->addListener(this);

	for(deque<pair<time_t, string> >::iterator i = oldMessages.begin(); i != oldMessages.end(); ++i) {
		addLine(i->first, Text::toT(i->second));
	}

	PostMessage(WM_COMMAND, IDC_SCROLL, 0); //post this to make sure it's received after the window's been created

	WinUtil::SetIcon(m_hWnd, _T("systemlog.ico"));

	bHandled = FALSE;
	return 1;
}

LRESULT SystemFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {

	LogManager::getInstance()->removeListener(this);

	checkButton(false);
	frame = NULL;

	bHandled = FALSE;
	return 0;
	
}


LRESULT SystemFrame::onContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
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

void SystemFrame::UpdateLayout(BOOL /*bResizeBars*/ /* = TRUE */)
{
	CRect rc;

	GetClientRect(rc);
	
	rc.bottom -= 1;
	rc.top += 1;
	rc.left +=1;
	rc.right -=1;
	ctrlPad.MoveWindow(rc);
	
}

LRESULT SystemFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	auto_ptr<pair<time_t, tstring> > msg((pair<time_t, tstring>*)wParam);
	
	addLine(msg->first, msg->second);
	if(BOOLSETTING(BOLD_SYSTEM_LOG))
		setDirty();
	return 0;
}

void SystemFrame::addLine(time_t t, const tstring& msg) {
	ctrlPad.AddLine((_T("[") + Util::getShortTimeString(t) + _T("] ") + msg).c_str(), false);

}
/**
 * @file
 * $Id: SystemFrame.cpp,v 1.3 2005/12/19 00:15:52 arnetheduck Exp $
 */
