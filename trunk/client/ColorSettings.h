#ifndef COLORSETTINGS_H
#define COLORSETTINGS_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "DcPlusPlus.h"
#include <richedit.h>

class ColorSettings
{
public:
	ColorSettings(): bTimestamps(false), bUsers(false), bMyNick(false){
		settings.cbSize = sizeof(CHARFORMAT2);
		settings.dwMask = CFM_BOLD ^ CFM_ITALIC ^ CFM_UNDERLINE ^ CFM_STRIKEOUT;
		settings.dwEffects = 0;
	}
	~ColorSettings(){};

	GETSET(bool, bWholeWord, WholeWord);
	GETSET(bool, bWholeLine, WholeLine);
	GETSET(bool, bIncludeNick, IncludeNick);
	GETSET(bool, bCaseSensitive, CaseSensitive);
	GETSET(bool, bPopup, Popup);
	GETSET(bool, bTab, Tab);
	GETSET(bool, bPlaySound, PlaySound);

	void setBold(bool bold) { 
		if( bold )
			settings.dwEffects ^= CFE_BOLD; 
	}
	void setUnderline(bool underline) {
		if( underline )
			settings.dwEffects ^= CFE_UNDERLINE;
	}
	void setItalic(bool italic) {
		if( italic ) 
			settings.dwEffects ^= CFE_ITALIC;
	}
	void setStrikeout(bool strikeout) {
		if( strikeout )
			settings.dwEffects ^= CFE_STRIKEOUT;
	}

	void setMatchType(int match ) {
		if( match >= 0 && match <= 3)
			iMatchType = match;
		else 
			iMatchType = 1;
	}

	void setBgColor(COLORREF bgColor, bool use) {
		if( use )
			settings.dwMask ^= CFM_BACKCOLOR;

		settings.crBackColor = bgColor;
		bBgColor = use;
	}
	
	void setFgColor(COLORREF fgColor, bool use) {
		if( use )
			settings.dwMask ^= CFM_COLOR;
		settings.crTextColor = fgColor;

		bFgColor = use;
	}

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

	void setSoundFile( string aFile ){
		soundFile = aFile;
	}

	bool getBold() { return settings.dwEffects & CFE_BOLD; }
	bool getItalic() { return settings.dwEffects & CFE_ITALIC; }
	bool getUnderline() { return settings.dwEffects & CFE_UNDERLINE; }
	bool getStrikeout() { return settings.dwEffects & CFE_STRIKEOUT; }
	
	bool getUsers() { return bUsers; }
	bool getTimestamps() { return bTimestamps; }
	bool getMyNick() { return bMyNick; }

	bool hasBgColor() { return bBgColor; }
	bool hasFgColor() { return bFgColor; }

	int getMatchType() { return iMatchType; }
	const string & getMatch() { return strMatch; }
	const string & getSoundFile() { return soundFile; }

	COLORREF getFgColor() { return settings.crTextColor; }
	COLORREF getBgColor() { return settings.crBackColor; }

	CHARFORMAT2& getSettings() { return settings; }

private:
	//string to match against
	string strMatch;

	//path to the sound to play
	string soundFile;

	//how the match is performed
	//0 - match the beginning of a word
	//1 - match any occurence
	//2 - match the end of a word
	//3 - match whole word
	int iMatchType;
	
	bool bTimestamps;
	bool bUsers;
	bool bMyNick;

	bool bBgColor;
	bool bFgColor;

	CHARFORMAT2 settings;
};

typedef vector< ColorSettings > colorVector;

#endif