/* 
* Copyright (C) 2003-2005 P�r Bj�rklund, per.bjorklund@gmail.com
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

#ifndef _CFULEDITCTRL_H_
#define _CFULEDITCTRL_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "../client/Util.h"

class CFulEditCtrl : public CWindowImpl<CFulEditCtrl, CRichEditCtrl >, public Flags
{
public:
	BEGIN_MSG_MAP(CFulEditCtrl)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_SIZE, onSize)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, onLButtonDown)
		MESSAGE_HANDLER(WM_FINDREPLACE, onFind)
		MESSAGE_HANDLER(WM_MOUSEMOVE, onMouseMove)
		MESSAGE_HANDLER(WM_SETCURSOR, onSetCursor)
	END_MSG_MAP()

	CFulEditCtrl(void);
	~CFulEditCtrl(void);

	enum {
		STRIP_ISP		= 1, //Activate strip isp
		HANDLE_SCROLL	= 2, //Determines if the richedit will handle scrolling
		POPUP			= 4, //if not set, will not popup messages on matches
		SOUND			= 8, //if not set, will not play sound on matches
		TAB				= 16, //if not set, will not color the tab on matches
		HANDLE_URLS		= 32, //if not set, will not handle urls on LButtonDown
		
		//determines which choices should be shown in the context menu
		//these need to be set before calling create
		MENU_COPY			= 64,
		MENU_PASTE			= 128,
		MENU_SEARCH			= 256,
		MENU_SEARCH_TTH		= 512,
		MENU_SEARCH_MENU	= 1024
	};

	bool	AddLine(const tstring & line, bool timeStamps = false);
	
	void	SetTextColor( COLORREF color );
	void	ScrollToEnd();
	void	ScrollToBeginning();

	int		TextUnderCursor(POINT p, tstring& x);
	bool	LastSeen(tstring & nick);
	BOOL	ShowMenu(HWND hWnd, POINT &pt);
	
	deque<tstring>* LastLog();
	
	//MESSAGE HANDLERS
	LRESULT onSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onFind(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onMenuCommand(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

	//COMMAND ID HANDLERS
	LRESULT onFind(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void	SetNick(const tstring& aNick);

private:
	void	AddInternalLine(const tstring &aLine);
	void	Colorize(const tstring &aLine, int begin);
	int		FullTextMatch(ColorSettings* cs, CHARFORMAT2 &cf, const tstring &line, int pos, int &lineIndex);
	int		RegExpMatch(ColorSettings* cs, CHARFORMAT2 &cf, const tstring &line, int &lineIndex);
	void	AddLogLine(const tstring &aLine);
	void	CheckAction(ColorSettings* cs, const tstring& line);
	void	CheckUrls(const tstring &line, const int &lineIndex);

	bool		matchedSound;
	bool		matchedPopup;
	bool		matchedTab;
	bool		logged;
	bool		skipLog;
	bool		timeStamps;
	bool		showHandCursor;

	tstring		nick;
	tstring		searchTerm;

	CHARFORMAT2 selFormat;
	TCHAR*		findBuffer;
	int			curFindPos;
	const WORD	findBufferSize;
    static UINT	WM_FINDREPLACE;
	TStringList	urls;

	//cache this here to avoid having to compute it on every WM_MOUSEMOVE message
	int			fontHeight;
	
	CMenu		menu;
	CMenu		searchMenu;

	HCURSOR		handCursor;

	deque<tstring> lastlog;
};

#endif