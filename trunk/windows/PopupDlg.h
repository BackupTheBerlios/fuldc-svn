#ifndef POPUPWND_H
#define POPUPWND_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"
#include "WinUtil.h"

class PopupWnd : public CWindowImpl<PopupWnd, CWindow>
{
public:
	DECLARE_WND_CLASS("Popup");

	BEGIN_MSG_MAP(PopupWnd)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_PAINT, onPaint)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, onLButtonDown)
	END_MSG_MAP()

	PopupWnd(HWND hWnd, const string& aMsg, CRect rc, HBITMAP hBmp): visible(GET_TICK()) {
		if(aMsg.length() > SETTING(MAX_MSG_LENGTH)){
			msg = aMsg.substr(0, SETTING(MAX_MSG_LENGTH)-3);
			msg += "...";
		}
		else
			msg = aMsg;

		bmp = hBmp;

		parent = hWnd;

		if(bmp == NULL)
			Create(NULL, rc, NULL, WS_CAPTION | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_TOOLWINDOW );
		else
			Create(NULL, rc, NULL, WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_TOOLWINDOW );

		WinUtil::decodeFont(SETTING(POPUP_FONT), logFont);
		font = ::CreateFontIndirect(&logFont);

	}

	~PopupWnd(){
		DeleteObject(font);
	}

	LRESULT onLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/){
		CRect rc;
		GetWindowRect(rc);
		::PostMessage(parent, WM_SPEAKER, WM_CLOSE, (LPARAM)rc.bottom);
		return 0;
	}

	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled){
		//if we have a background don't create CStatic control
		if(bmp != NULL) {
			bHandled = false;
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

		SetWindowText("fulDC");

		label.SetFont(WinUtil::font);
		label.SetWindowText(msg.c_str());


		bHandled = false;
		return 1;
	}

	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled){
		if(bmp == NULL){
			label.DestroyWindow();
			label.Detach();
		}
		DestroyWindow();

		bHandled = false;
		return 1;
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

	u_int32_t visible;

private:
	string  msg;
	CStatic label;
	HBITMAP bmp;
	LOGFONT logFont;
	HFONT   font;
	HWND	parent;

};


#endif