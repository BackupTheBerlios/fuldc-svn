#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"
#include "../client/highlightmanager.h"
#include "../client/LogManager.h"
#include "../client/ResourceManager.h"

#include "../client/pme.h"

#include "fuleditctrl.h"
#include "PopupManager.h"
#include <stack>
#include <MMSystem.h>

#include "WinUtil.h"

UINT CFulEditCtrl::WM_FINDREPLACE = RegisterWindowMessage(FINDMSGSTRING);

CFulEditCtrl::CFulEditCtrl(void): matchedPopup(false), nick(Util::emptyStringT), findBufferSize(100),
								logged(false), matchedSound(false), skipLog(false)
{
	findBuffer = new TCHAR[findBufferSize];
	findBuffer[0] = _T('\0');

	urls.push_back(_T("http://"));
	urls.push_back(_T("https://"));
	urls.push_back(_T("www."));
	urls.push_back(_T("ftp://"));
	urls.push_back(_T("ftps://"));
	urls.push_back(_T("mms://"));
	urls.push_back(_T("dchub://"));

	setFlag(HANDLE_SCROLL | POPUP | TAB | SOUND | HANDLE_URLS);

}
CFulEditCtrl::~CFulEditCtrl(void)
{
	delete[] findBuffer;
}

bool CFulEditCtrl::AddLine(const tstring & line, bool timeStamps) {
	bool noScroll = false;
	matchedTab = false;
	tstring aLine = line;
	if(GetWindowTextLength() > SETTING(CHATBUFFERSIZE)) {
		SetRedraw(false);
		SetSel(0, LineIndex(LineFromChar(2000)));
		ReplaceSel(_T(""));
		SetSel(GetTextLengthEx(GTL_NUMCHARS), GetTextLengthEx(GTL_NUMCHARS));
		ScrollCaret();
		SetRedraw(true);
	}
	if(Util::strnicmp(_T("<") + nick + _T(">"), aLine, nick.length() + 2) == 0)
		skipLog = true;

	if(isSet(STRIP_ISP) && aLine[0] == _T('<')) {
		u_int pos = aLine.find(_T("]"));
		if(pos < aLine.find(_T(">")) )
			aLine = _T("<") + aLine.substr(pos+1);
	}
		
	tstring::size_type pos = aLine.find(_T("> /me "));
	if( pos != tstring::npos)
		aLine = _T("** ") + aLine.substr(1, pos-1) +  aLine.substr(pos+5, aLine.length());

	if(timeStamps) {
		aLine = _T("\r\n[") + Util::getShortTimeStringW() + _T("] ") + aLine;
	} else {
		aLine = _T("\r\n") + aLine;
	}
	
	SetRedraw(FALSE);
	
	//Get the pos of the last char
	POINT pt = PosFromChar(GetTextLengthEx(GTL_NUMCHARS));
	CRect rc;
	GetClientRect(&rc);
	int l = -1;

	//check if the last char is visible, if not then save the
	//scrollbar position
	if(rc.PtInRect(pt)){
		noScroll = false;
	} else {
		noScroll = true;
		l = GetFirstVisibleLine();
	}
    
	AddInternalLine(aLine);
				
	//restore the scrollbar position
	if(noScroll) {
		LineScroll(l - GetFirstVisibleLine());
	} 

	SetRedraw();
	Invalidate();
	UpdateWindow();

	return matchedTab;
}

void CFulEditCtrl::Colorize(int begin) {
	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	
	ColorList *cList = HighlightManager::getInstance()->rLock();

	int end = GetTextLengthEx(GTL_NUMCHARS);
	TCHAR *buf = new TCHAR[end-begin+1];
	HideSelection(true, false);
	SetSel(begin, end);
	//otroligt fulhack, måste lagas riktigt nån gång
	SetSelectionCharFormat(selFormat);

	GetSelText(buf);
	SetSel(end, end);
	
	tstring line = buf;
	delete[] buf;

	logged = false;

	//compare the last line against all strings in the vector
	for(ColorIter i = cList->begin(); i != cList->end(); ++i) {
		ColorSettings* cs = *i;
		int pos;
		
		//set start position for find
		if( cs->getIncludeNick() ) {
			pos = 0;
		} else {
			pos = line.find(_T(">"));
			if(pos == tstring::npos)
				pos = line.find(_T("**")) + nick.length();
		}

		//prepare the charformat
		cf.dwMask = CFM_BOLD | CFM_UNDERLINE | CFM_STRIKEOUT | CFM_ITALIC;
		cf.dwEffects = 0;
		if(cs->getBold())		cf.dwEffects |= CFE_BOLD;
		if(cs->getItalic())		cf.dwEffects |= CFE_ITALIC;
		if(cs->getUnderline())	cf.dwEffects |= CFE_UNDERLINE;
		if(cs->getStrikeout())	cf.dwEffects |= CFE_STRIKEOUT;
		
		if(cs->getHasBgColor()){
			cf.dwMask |= CFM_BACKCOLOR;
			cf.crBackColor = cs->getBgColor();
		}
		if(cs->getHasFgColor()){
			cf.dwMask |= CFM_COLOR;
			cf.crTextColor = cs->getFgColor();
		}

		while( pos != string::npos ){
			if(cs->usingRegexp()) 
				pos = RegExpMatch(cs, cf, line, begin);
			else 
				pos = FullTextMatch(cs, cf, line, pos, begin);
		}

		matchedPopup = false;
		matchedSound = false;
		
	}//end for

	HighlightManager::getInstance()->rUnlock();

	HideSelection(false, false);
}//end Colorize

void CFulEditCtrl::SetTextColor( COLORREF color ) {
	CHARFORMAT cf;
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = 0;
	cf.crTextColor = color;
	SetDefaultCharFormat(cf);

	//otroligt fulhack, måste lagas riktigt nån gång
	selFormat.cbSize = sizeof(CHARFORMAT2);
	GetSelectionCharFormat(selFormat);
}

int CFulEditCtrl::TextUnderCursor(POINT p, tstring& x) {
	
	int i = CharFromPos(p);
	int line = LineFromChar(i);
	int c = i - LineIndex(line);
	int len = LineLength(i) + 1;
	if(len < 3) {
		return 0;
	}

	TCHAR* buf = new TCHAR[len];
	GetLine(line, buf, len);
	x = tstring(buf, len-1);
	delete[] buf;

	tstring::size_type start = x.find_last_of(_T(" <\t\r\n"), c);
	if(start == tstring::npos)
		start = 0;
	else
		start++;

	return start;
}

void CFulEditCtrl::AddInternalLine(tstring & aLine) {
	int length = GetTextLengthEx(GTL_NUMCHARS)+1;
	AppendText(aLine.c_str());
	Colorize(length);
	
	SetSel(GetTextLengthEx(GTL_NUMCHARS), GetTextLengthEx(GTL_NUMCHARS));
	
	skipLog = false;
	ScrollCaret();
}

int CFulEditCtrl::FullTextMatch(ColorSettings* cs, CHARFORMAT2 &cf, tstring &line, int pos, int &lineIndex) {
	int index = tstring::npos;
	tstring searchString;

	if( cs->getMyNick() )
		searchString = nick;
	else
		searchString = cs->getMatch();
	
	//we don't have any nick to search for
	//happens in pm's have to find a solution for this
	if(searchString.empty())
		return tstring::npos;


	//do we want to highlight the timestamps
	if( cs->getTimestamps() ) {
		if( line[0] != _T('[') )
			return tstring::npos;
		index = 0;
	} else if( cs->getUsers() ) {
		index = line.find(_T("<"));
	}else{
		if( cs->getCaseSensitive() ) {
			index = line.find(searchString, pos);
		}else {
			//index = Util::findSubString(line, searchString, pos);	
			index = Text::toLower(line).find(Text::toLower(searchString), pos);
		}
	}
	//return if no matches where found
	if( index == tstring::npos )
		return tstring::npos;

	pos = index + searchString.length();
	
	//found the string, now make sure it matches
	//the way the user specified
	int length;
		
	if( !cs->getUsers() && !cs->getTimestamps() ) {
		length = searchString.length();
			
		switch(cs->getMatchType()){
			case 0:
				if(line[index-1] != _T(' ') && line[index-1] != _T('\r') )
					return tstring::npos;
				break;
			case 1:
				break;
			case 2:
				if(line[index+length] != _T(' ') && line[index+length] != _T('\r'))
					return tstring::npos;
				break;
			case 3:
				if( !( (index == 0 || line[index-1] == _T(' ')) && (line[index+length] == _T(' ') || line[index+length] == _T('\r')) ) )
					return tstring::npos;
				break;
		}
	}

	long begin, end;

	begin = lineIndex;
		
	if( cs->getTimestamps() ) {
		tstring::size_type pos = line.find(_T("]"));
		if( pos == tstring::npos ) 
			return tstring::npos;  //hmm no ]? this can't be right, return
		
		begin += index +1;
		end = begin + pos -1;
	} else if( cs->getUsers() ) {
		end = begin + line.find(_T(">"));
		begin += index +1;
	} else if( cs->getWholeLine() ) {
		end = begin + line.length();
	} else if( cs->getWholeWord() ) {
		int tmp;

		tmp = line.find_last_of(_T(" \t\r\n"), index);
		if(tmp != tstring::npos )
			begin += tmp+1;
		
		tmp = line.find_first_of(_T(" \t\r\n"), index);
		if(tmp != tstring::npos )
			end = lineIndex + tmp;
		else
			end = lineIndex + line.length();
	} else {
		begin += index;
		end = begin + searchString.length();
	}

	SetSel(begin, end);
		
	SetSelectionCharFormat(cf);

	SetSel(GetTextLength()-1, GetTextLength()-1);
	SetSelectionCharFormat(selFormat);

	if(cs->getPopup() && !matchedPopup && isSet(POPUP)) {
		matchedPopup = true;
		PopupManager::getInstance()->ShowMC(line);
	}
	if(cs->getTab() && isSet(TAB))
		matchedTab = true;

	if(cs->getLog() && !logged && !skipLog){
		logged = true;
		AddLogLine(line);
	}

	if(cs->getPlaySound() && !matchedSound && isSet(SOUND)){
		matchedSound = true;
		PlaySound(cs->getSoundFile().c_str(), NULL, SND_ASYNC | SND_FILENAME | SND_NOWAIT);
	}
					
	if( cs->getTimestamps() || cs->getUsers() )
		return tstring::npos;
	
	return pos;
}

int CFulEditCtrl::RegExpMatch(ColorSettings* cs, CHARFORMAT2 &cf, tstring &line, int &lineIndex) {
	int begin = 0, end = 0;
	bool found = false;
		
	PME regexp(cs->getMatch().substr(4), _T("gims"));
				
	while( regexp.match(line) > 0 ){
		found = true;

		if( regexp.NumBackRefs() == 1){
			begin = lineIndex + regexp.GetStartPos(0);
			end = begin + regexp.GetLength(0);

			SetSel(begin, end);
			SetSelectionCharFormat(cf);

			SetSel(GetTextLength()-1, GetTextLength()-1);
			SetSelectionCharFormat(selFormat);
		} else {
			for(int j = 1; j < regexp.NumBackRefs(); ++j) {
				begin = lineIndex + regexp.GetStartPos(j);
				end = begin + regexp.GetLength(j);
				
				SetSel(begin, end);
				SetSelectionCharFormat(cf);

				SetSel(GetTextLength()-1, GetTextLength()-1);
				SetSelectionCharFormat(selFormat);
			}
		}
	}

	if(!found)
		return tstring::npos;
	

	SetSel(GetTextLength()-1, GetTextLength()-1);
	SetSelectionCharFormat(selFormat);

	if(cs->getPopup() && !matchedPopup && isSet(POPUP)) {
		matchedPopup = true;
		PopupManager::getInstance()->ShowMC(line);
	}

	if(cs->getTab() && isSet(TAB))
		matchedTab = true;

	if(cs->getLog() && !logged && !skipLog){
		logged = true;
		AddLogLine(line);
	}

	if(cs->getPlaySound() && !matchedSound && isSet(SOUND)){
		matchedSound = true;
		PlaySound(cs->getSoundFile().c_str(), NULL, SND_ASYNC | SND_FILENAME | SND_NOWAIT);
	}
	
	return tstring::npos;
}

LRESULT CFulEditCtrl::onSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled){
	if(isSet(HANDLE_SCROLL))
		ScrollToEnd();
			
	bHandled = FALSE;
	return 0;
}

LRESULT CFulEditCtrl::onLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
	
	//set this here since it will be false in most cases anyway
	bHandled = FALSE;

	if(!isSet(HANDLE_URLS)){
		return 1;
	}

	POINT mousePT = {GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam)};
	int ch = CharFromPos(mousePT);
	POINT charPT = PosFromChar(ch);

	//since CharFromPos returns the last character even if the pointer is past the end of text
	//we have to check if the pointer was actually above the last char

	//check xpos
	if(mousePT.x < charPT.x || mousePT.x > (charPT.x + WinUtil::getTextWidth(m_hWnd, WinUtil::font))) 
		return 1;
	
	//check ypos
	if( mousePT.y > (charPT.y + WinUtil::getTextHeight(m_hWnd, WinUtil::font)) )
		return 1;
	

	FINDTEXT ft;
	ft.chrg.cpMin = ch;
	ft.chrg.cpMax = -1;
	ft.lpstrText = _T("\r");

	int begin = (int)SendMessage(EM_FINDTEXT, 0, (LPARAM)&ft) + 1;
	int rEnd = (int)SendMessage(EM_FINDTEXT, FR_DOWN, (LPARAM)&ft);

	if(begin < 0)
		begin = 0;
	if(rEnd == -1)
		rEnd = GetTextLengthEx(GTL_NUMCHARS);
		
	TCHAR *buf = new TCHAR[rEnd-begin+1];
	
	GetTextRange(begin, rEnd, buf);
	
	
	tstring tmp = buf;
	delete[] buf;
	
	int start = tmp.find_last_of(_T(" \t\r\n"), ch-begin) +1;
	int end = tmp.find_first_of(_T(" \t\r\n"), start+1);
	if(end == tstring::npos)
		end = tmp.length();
	tstring url = tmp.substr(start, end-start);
	
	
	bool found = false;
	TStringIter i = urls.begin();

	for(; i != urls.end(); ++i) {
		if(Util::strnicmp(url.c_str(), (*i).c_str(), (*i).length()) == 0){
			found = true;
			WinUtil::openLink(url);
			bHandled = TRUE;
			return 0;
		}
	}

	return 1;
}

LRESULT CFulEditCtrl::onFind(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/){
	LPFINDREPLACE fr = (LPFINDREPLACE)lParam;

	if(fr->Flags & FR_DIALOGTERM){
		WinUtil::findDialog = NULL;
	} else if(fr->Flags & FR_FINDNEXT){
		//prepare the flags used to search
		int flags = 0;
		if(fr->Flags & FR_WHOLEWORD)
			flags = FR_WHOLEWORD;
		if(fr->Flags & FR_MATCHCASE)
			flags |= FR_MATCHCASE;
		if(fr->Flags & FR_DOWN)
			flags |= FR_DOWN;

		//initiate the structure, cpMax -1 means the whole document is searched
		FINDTEXTEX ft;
		ft.chrg.cpMax = -1;
		ft.chrg.cpMin = curFindPos;
		ft.lpstrText = fr->lpstrFindWhat;
		
		//if we find the end of the document, notify the user and return
		int result = (int)SendMessage(EM_FINDTEXT, (WPARAM)flags, (LPARAM)&ft);
		if(-1 == result){
			MessageBox(CTSTRING(FINISHED_SEARCHING), _T(FULDC) _T(" ") _T(FULVERSIONSTRING), MB_OK | MB_ICONINFORMATION);
			curFindPos = 0;
			return 0;
		}

		//select the result and scroll it into view
		curFindPos = result +1 ;
		SetFocus();
		SetSel(result, result + _tcslen(ft.lpstrText));
		ScrollCaret();
	}

	return 0;
}


void CFulEditCtrl::ScrollToEnd() {
	SetRedraw(FALSE);
	SetSel(0, 0);
	ScrollCaret();
	int l = GetTextLength();
	SetSel(l, l);
	SendMessage(EM_SCROLLCARET, 0, 0);
	SetRedraw(TRUE);
	Invalidate();
}

void CFulEditCtrl::ScrollToBeginning() {
	SetRedraw(FALSE);
	SetSel(0, 0);
	SendMessage(EM_SCROLLCARET, 0, 0);
	SetRedraw(TRUE);
	Invalidate();
	UpdateWindow();
}

void CFulEditCtrl::Find() {
	LPFINDREPLACE fr = new FINDREPLACE;
	ZeroMemory(fr, sizeof(FINDREPLACE));
	fr->lStructSize = sizeof(FINDREPLACE);
	fr->hwndOwner = m_hWnd;
	fr->hInstance = NULL;
	fr->Flags = FR_DOWN;
	
	fr->lpstrFindWhat = findBuffer;
	fr->wFindWhatLen = findBufferSize;

	if(WinUtil::findDialog == NULL)
		WinUtil::findDialog = ::FindText(fr);

	curFindPos = 0;
}

bool CFulEditCtrl::LastSeen(tstring & nick){
	FINDTEXTEX ft;
	CHARRANGE sel;
	int result;
	
	bool found = false;
	tstring search = _T("<") + nick + _T(">");
		
	ft.chrg.cpMin = 0;
	ft.chrg.cpMax = -1;
	ft.lpstrText = search.c_str();
	
	//for some reason it can't search up, either msdn is wrong or i missed something =)
	while(-1 != (result = (int)::SendMessage(m_hWnd, EM_FINDTEXTEX, FR_DOWN | FR_WHOLEWORD, (LPARAM)&ft))){
		found = true;
		ft.chrg.cpMin = result+1;
		sel = ft.chrgText;
	}
	
	if(!found)
		return false;
	
	SetSel(sel);
	ScrollCaret();
	return true;
}

void CFulEditCtrl::AddLogLine(tstring & line){
	if(lastlog.size() == 100)
		lastlog.pop_front();
	
	lastlog.push_back(line);
}

deque<tstring> *CFulEditCtrl::LastLog(){
	return &lastlog;
}