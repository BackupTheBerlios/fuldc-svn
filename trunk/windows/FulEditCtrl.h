#ifndef _CFULEDITCTRL_H_
#define _CFULEDITCTRL_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "../client/Util.h"

class CFulEditCtrl : public CWindowImpl<CFulEditCtrl, CRichEditCtrl>, public Flags
{
public:
	BEGIN_MSG_MAP(CFulEditCtrl)
		MESSAGE_HANDLER(WM_SIZE, onSize)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, onLButtonDown)
		MESSAGE_HANDLER(WM_FINDREPLACE, onFind)
	END_MSG_MAP()

	CFulEditCtrl(void);
	~CFulEditCtrl(void);

	enum {
		STRIP_ISP		= 0x01, // Activate strip isp
		HANDLE_SCROLL	= 0x02, //Determines if the richedit will handle scrolling
		POPUP			= 0x04, //if not set, will not popup messages on matches
		SOUND			= 0x08, //if not set, will not play sound on matches
		TAB				= 0x10, //if not set, will not color the tab on matches
		HANDLE_URLS		= 0x20  //if not set, will not handle urls on LButtonDown
	};

	bool	AddLine(const tstring & line, bool timeStamps = false);
	void	SetTextColor( COLORREF color );
	void	ScrollToEnd();
	void	ScrollToBeginning();
	int		TextUnderCursor(POINT p, tstring& x);
	void	Find();
	bool	LastSeen(tstring & nick);
		
	deque<tstring>* LastLog();
	
	LRESULT onSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onFind(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		
	void	SetNick(const tstring& aNick) { nick = aNick; }

private:
	void	AddInternalLine(tstring &aLine);
	void	Colorize(int begin);
	int		FullTextMatch(ColorSettings* cs, CHARFORMAT2 &cf, tstring &line, int pos, int &lineIndex);
	int		RegExpMatch(ColorSettings* cs, CHARFORMAT2 &cf, tstring &line, int &lineIndex);
	void	AddLogLine(tstring & line);

	bool		matchedSound;
	bool		matchedPopup;
	bool		matchedTab;
	bool		logged;
	bool		skipLog;
	tstring		nick;
	CHARFORMAT2 selFormat;
	TCHAR*		findBuffer;
	int			curFindPos;
	const WORD	findBufferSize;
    static UINT	WM_FINDREPLACE;
	TStringList	urls;
		
	deque<tstring> lastlog;
};

#endif