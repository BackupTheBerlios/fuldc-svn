#ifndef COLORSETTINGS_H
#define COLORSETTINGS_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "DcPlusPlus.h"

class ColorSettings
{
public:
	ColorSettings(): bTimestamps(false), bUsers(false), bMyNick(false){	}
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
	GETSET(int,  iMatchType, MatchType);
	GETSET(int,  iBgColor, BgColor);
	GETSET(int,  iFgColor, FgColor);
	GETSET(bool, bHasBgColor, HasBgColor);
	GETSET(bool, bHasFgColor, HasFgColor);
	GETSETREF(string, strSoundFile, SoundFile);

	

	void setMatch(string match){
		if( match.compare("$ts$") == 0){
			bTimestamps = true;
		}else if(match.compare("$users$") == 0) {
			bUsers = true;
		}else if(match.compare("$mynick$") == 0) {
			bMyNick = true;
		}
		strMatch = match;
	}

	bool getUsers() { return bUsers; }
	bool getTimestamps() { return bTimestamps; }
	bool getMyNick() { return bMyNick; }

	const string & getMatch() { return strMatch; }
	
private:
	//string to match against
	string strMatch;

	bool bTimestamps;
	bool bUsers;
	bool bMyNick;

};

typedef vector< ColorSettings* > colorVector;

#endif