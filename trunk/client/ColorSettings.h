#ifndef COLORSETTINGS_H
#define COLORSETTINGS_H

#include "stdinc.h"
#include "DcPlusPlus.h"

class ColorSettings
{
public:
	ColorSettings(): bTimestamps(false), bUsers(false), bMyNick(false), bUsingRegexp(false), 
		strMatch(Util::emptyStringT), strSoundFile(Util::emptyStringT) {	}
	~ColorSettings(){};

	GETSET(bool, bWholeWord, WholeWord);
	GETSET(bool, bWholeLine, WholeLine);
	GETSET(bool, bIncludeNick, IncludeNick);
	GETSET(bool, bCaseSensitive, CaseSensitive);
	GETSET(bool, bPopup, Popup);
	GETSET(bool, bTab, Tab);
	GETSET(bool, bPlaySound, PlaySound);
	GETSET(bool, bBold, Bold);
	GETSET(bool, bUnderline, Underline);
	GETSET(bool, bItalic, Italic);
	GETSET(bool, bStrikeout, Strikeout);
	GETSET(bool, bLastLog, Log);
	GETSET(bool, bFlashWindow, FlashWindow);
	GETSET(int,  iMatchType, MatchType);
	GETSET(int,  iBgColor, BgColor);
	GETSET(int,  iFgColor, FgColor);
	GETSET(bool, bHasBgColor, HasBgColor);
	GETSET(bool, bHasFgColor, HasFgColor);
	GETSET(tstring, strSoundFile, SoundFile);
    	
	void setMatch(tstring match){
		if( match.compare(_T("$ts$")) == 0){
			bTimestamps = true;
		}else if(match.compare(_T("$users$")) == 0) {
			bUsers = true;
		}else if(match.find(_T("$mynick$")) != tstring::npos) {
			bMyNick = true;
		} else if(match.find(_T("$Re:")) == 0) {
			bUsingRegexp = true;
		}
		strMatch = match;
	}

	bool getUsers() { return bUsers; }
	bool getTimestamps() { return bTimestamps; }
	bool getMyNick() { return bMyNick; }
	bool usingRegexp() { return bUsingRegexp; }

	const tstring & getMatch() { return strMatch; }
	
private:
	//string to match against
	tstring strMatch;

	bool bTimestamps;
	bool bUsers;
	bool bMyNick;
	bool bUsingRegexp;

};
#endif