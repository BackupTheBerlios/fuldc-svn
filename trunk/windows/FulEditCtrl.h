#ifndef _CFULEDITCTRL_H_
#define _CFULEDITCTRL_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CFulEditCtrl : /*public CRichEditCtrl*/ public CWindowImpl<CFulEditCtrl, CRichEditCtrl>
{
public:
	BEGIN_MSG_MAP(CFulEditCtrl)
		MESSAGE_HANDLER(WM_SIZE, onSize)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, onLButtonDown)
		MESSAGE_HANDLER(WM_FINDREPLACE, onFind)
	END_MSG_MAP()

	CFulEditCtrl(void);
	~CFulEditCtrl(void);

	bool	AddLine(const string & line, bool timeStamps = false);
	void	SetTextColor( COLORREF color );
	void	ScrollEnd();
	int		TextUnderCursor(POINT p, string& x);
	void	Find();
	bool	LastSeen(string & nick);
	void	LastLog(string * str, int nrLines = 100);
	
	LRESULT onSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onFind(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		
	void	StripIsp(bool strip = false) { stripIsp = strip; }
	void	SetNick(const string aNick) { nick = aNick; }

private:
	void	AddInternalLine(string &aLine);
	void	Colorize(int begin);
	int		Highlight(ColorSettings* cs, CHARFORMAT2 &cf, string &line, int pos, int &lineIndex);
	int		FullTextMatch(ColorSettings* cs, CHARFORMAT2 &cf, string &line, int pos, int &lineIndex);
	int		RegExpMatch(ColorSettings* cs, CHARFORMAT2 &cf, string &line, int pos, int &lineIndex);
	void	AddLogLine(string & line);

	bool		matchedSound;
	bool		matchedPopup;
	bool		matchedTab;
	bool		stripIsp;
	bool		noScroll;
	bool		logged;
	string		nick;
	CHARFORMAT2 selFormat;
	char*		findBuffer;
	int			curFindPos;
	const int	findBufferSize;
    static UINT	WM_FINDREPLACE;
	StringList	urls;
		
	deque<string> lastlog;
};

#endif