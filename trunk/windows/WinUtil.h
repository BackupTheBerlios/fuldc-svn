/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

#ifndef __WINUTIL_H
#define __WINUTIL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/Util.h"
#include "../client/SettingsManager.h"
#include "../client/User.h"
#include "../client/WebShortcuts.h"

#include "StaticFrm.h"
#include "UserInfoBase.h"

// Some utilities for handling HLS colors, taken from Jean-Michel LE FOL's codeproject
// article on WTL OfficeXP Menus
typedef DWORD HLSCOLOR;
#define HLS(h,l,s) ((HLSCOLOR)(((BYTE)(h)|((WORD)((BYTE)(l))<<8))|(((DWORD)(BYTE)(s))<<16)))
#define HLS_H(hls) ((BYTE)(hls))
#define HLS_L(hls) ((BYTE)(((WORD)(hls)) >> 8))
#define HLS_S(hls) ((BYTE)((hls)>>16))

HLSCOLOR RGB2HLS (COLORREF rgb);
COLORREF HLS2RGB (HLSCOLOR hls);

COLORREF HLS_TRANSFORM (COLORREF rgb, int percent_L, int percent_S);

class FlatTabCtrl;
class UserCommand;


class WinUtil {
public:
	static CImageList fileImages;
	static int fileImageCount;
	static CImageList userImages;

	typedef HASH_MAP<string, int> ImageMap;
	typedef ImageMap::iterator ImageIter;
	static ImageMap fileIndexes;
	static HBRUSH bgBrush;
	static COLORREF textColor;
	static COLORREF bgColor;
	static HFONT font;
	static int fontHeight;
	static HFONT boldFont;
	static HFONT systemFont;
	static HFONT monoFont;
	static HFONT tabFont;
	static CMenu mainMenu;
	static int dirIconIndex;
	static StringList lastDirs;
	static HWND mainWnd;
	static HWND mdiClient;
	static FlatTabCtrl* tabCtrl;
	static string commands;
	static HHOOK hook;
	static HWND findDialog;
	static const u_int32_t startTime;

	static void SearchSite(WebShortcut* ws, string strSearchString);


	static void init(HWND hWnd);
	static void uninit();

	static void decodeFont(const string& setting, LOGFONT &dest);

	static void SetIcon(HWND hWnd, string file, bool big = false);
	static void copyToClipboard(const string & aStr);
	static void search(string searchTerm, int searchMode);
	static void AppendSearchMenu(CMenu& menu);

	/**
	 * Check if this is a common /-command.
	 * @param cmd The whole text string, will be updated to contain only the command.
	 * @param param Set to any parameters.
	 * @param message Message that should be sent to the chat.
	 * @param status Message that should be shown in the status line.
	 * @return True if the command was processed, false otherwise.
	 */
	static bool checkCommand(string& cmd, string& param, string& message, string& status);

	static int getTextWidth(const string& str, HWND hWnd);
	static int getTextWidth(const string& str, HDC dc);
	static int getTextWidth(HWND wnd, HFONT fnt);
	static int getTextHeight(HWND wnd, HFONT fnt);
	static int getTextHeight(HDC dc, HFONT fnt);
	static int getTextHeight(HDC dc);
	static int getTextSpacing(HWND wnd, HFONT fnt); 

	static void addLastDir(const string& dir);
	
	static string encodeFont(LOGFONT const& font);
		
	static bool browseFile(string& target, HWND owner = NULL, bool save = true, const string& initialDir = Util::emptyString, const char* types = NULL, const char* defExt = NULL);
	static bool browseDirectory(string& target, HWND owner = NULL);

	static void openLink(const string& url);
	static void openFile(const string& file) {
		::ShellExecute(NULL, NULL, file.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}

	static int getIconIndex(const string& aFileName);

	static int getDirIconIndex() {
		return dirIconIndex;
	}
	
	static bool getUCParams(HWND parent, const UserCommand& cmd, StringMap& sm) throw();

	static void splitTokens(int* array, const string& tokens, int maxItems = -1) throw();
	static void saveHeaderOrder(CListViewCtrl& ctrl, SettingsManager::StrSetting order, 
		SettingsManager::StrSetting widths, int n, int* indexes, int* sizes) throw();
	
private:
	static int CALLBACK browseCallbackProc(HWND hwnd, UINT uMsg, LPARAM /*lp*/, LPARAM pData);		
	
};

#endif // __WINUTIL_H

/**
 * @file
 * $Id: WinUtil.h,v 1.13 2004/02/15 16:58:49 trem Exp $
 */
