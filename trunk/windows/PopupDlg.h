/* 
* Copyright (C) 2003-2005 Pär Björklund, per.bjorklund@gmail.com
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

#ifndef POPUPWND_H
#define POPUPWND_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "../client/version.h"
#include "Resource.h"
#include "WinUtil.h"

class PopupWnd : public CWindowImpl<PopupWnd, CWindow>
{
public:
	DECLARE_WND_CLASS(_T("Popup"));

	BEGIN_MSG_MAP(PopupWnd)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_PAINT, onPaint)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, onLButtonDown)
	END_MSG_MAP()

	PopupWnd(const tstring& aMsg, CRect rc, HBITMAP hBmp, u_int32_t aId): visible(GET_TICK()), id(aId) {
		if(int(aMsg.length()) > SETTING(MAX_MSG_LENGTH)){
			msg = aMsg.substr(0, SETTING(MAX_MSG_LENGTH)-3);
			msg += _T("...");
		}
		else
			msg = aMsg;

		bmp = hBmp;

		if(bmp == NULL)
			Create(NULL, rc, NULL, WS_CAPTION | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_TOOLWINDOW );
		else
			Create(NULL, rc, NULL, WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_TOOLWINDOW );

		WinUtil::decodeFont(Text::toT(SETTING(POPUP_FONT)), logFont);
		font = ::CreateFontIndirect(&logFont);

	}

	~PopupWnd(){
		DeleteObject(font);
	}

	LRESULT onLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled){
		::PostMessage(WinUtil::mainWnd, WM_SPEAKER, WM_CLOSE, (LPARAM)id);
		bHandled = TRUE;
		return 0;
	}

	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled){
		//if we have a background don't create CStatic control
		if(bmp != NULL) {
			bHandled = FALSE;
			return 1;
		}

		::SetClassLongPtr(m_hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)::GetSysColorBrush(COLOR_3DFACE));
		CRect rc;
		GetClientRect(rc);

		rc.top += 5;
		rc.left += 5;
		rc.right -= 5;
		rc.bottom -=5;

		label.Create(m_hWnd, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
			SS_CENTER | SS_NOPREFIX);

		SetWindowText(_T(FULDC));

		label.SetFont(WinUtil::font);
		label.SetWindowText(msg.c_str());


		bHandled = TRUE;
		return 0;
	}

	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/){
		if(bmp == NULL){
			label.DestroyWindow();
			label.Detach();
		}
		DestroyWindow();

		return 0;
	}

	LRESULT onPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		//if we don't have any background let windows do the painting
		if(bmp == NULL){
			bHandled = FALSE;
			return 0;
		}

		PAINTSTRUCT ps;
		HDC hdc = ::BeginPaint(m_hWnd,&ps);

		HDC hdcMem = CreateCompatibleDC(NULL);
		HBITMAP hbmT = (HBITMAP)::SelectObject(hdcMem,bmp);

		BITMAP bm;
		GetObject(bmp,sizeof(bm),&bm);

		//transfer the selected bitmap to the background
		BitBlt(hdc,0,0,bm.bmWidth,bm.bmHeight,hdcMem,0,0,SRCCOPY);

		SelectObject(hdcMem,hbmT);
		DeleteDC(hdcMem);

		//cofigure the text and border
		HFONT oldFont = (HFONT)SelectObject(hdc, font);
		::SetBkMode(hdc, TRANSPARENT);
		::SetTextColor(hdc, SETTING(POPUP_TEXTCOLOR));

		int xBorder = bm.bmWidth / 10;
		int yBorder = bm.bmHeight / 10;
		CRect rc(xBorder, yBorder, bm.bmWidth - xBorder, bm.bmHeight - yBorder);

		::DrawText(hdc, msg.c_str(), msg.length(), rc, DT_CENTER | DT_WORDBREAK);

		SelectObject(hdc, oldFont);
		::EndPaint(m_hWnd,&ps);

		return 0;

	}

	u_int32_t id;
	u_int32_t visible;
	HWND owner;

private:
	tstring  msg;
	CStatic label;
	HBITMAP bmp;
	LOGFONT logFont;
	HFONT   font;
};


#endif