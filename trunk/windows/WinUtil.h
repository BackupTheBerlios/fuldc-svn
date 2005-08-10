/*
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(WIN_UTIL_H)
#define WIN_UTIL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/Util.h"
#include "../client/SettingsManager.h"
#include "../client/User.h"
#include "../client/MerkleTree.h"
#include "../client/WebShortcuts.h"

#include "StaticFrm.h"
#include "UserInfoBase.h"
#include "FulEditCtrl.h"



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
	static int dirMaskedIndex;
	static TStringList lastDirs;
	static HWND mainWnd;
	static HWND mdiClient;
	static FlatTabCtrl* tabCtrl;
	static HHOOK hook;
	static tstring tth;
	static StringPairList initialDirs;
	static DWORD helpCookie;
	static HWND findDialog;
	static const time_t startTime;
	static DWORD comCtlVersion;

	static void SearchSite(WebShortcut* ws, tstring strSearchString);
	static int checkParam( const tstring& param );


	static void init(HWND hWnd);
	static void uninit();

	static void decodeFont(const tstring& setting, LOGFONT &dest);
	static tstring encodeFont(LOGFONT const& font);
	
	static void SetIcon(HWND hWnd, tstring file, bool big = false);
	static void setClipboard(const tstring & str);
	static void search(tstring searchTerm, int searchMode, bool tth = false);
	static void AppendSearchMenu(CMenu& menu);
	static bool flashWindow();

	static tstring DiskSpaceInfo(bool onlyTotal = false);
	static tstring Help(const tstring& command);
	static tstring Uptime();
	static tstring UselessInfo();

	static void addInitalDir(const User::Ptr& user, string dir) {
		// Clear out previos initial dirs, just in case
		/// @todo clean up
		getInitialDir(user);
		while(initialDirs.size() > 30) {
			initialDirs.erase(initialDirs.begin());
		}
		initialDirs.push_back(make_pair(user->getCID().toBase32(), dir));
	}

	static string getInitialDir(const User::Ptr& user) {
		for(StringPairIter i = initialDirs.begin(); i != initialDirs.end(); ++i) {
			if(i->first == user->getCID().toBase32()) {
				string dir = i->second;
				initialDirs.erase(i);
				return dir;
			}
		}
		return Util::emptyString;
	}

	static bool getVersionInfo(OSVERSIONINFOEX& ver);

	/**
	 * Check if this is a common /-command.
	 * @param cmd The whole text string, will be updated to contain only the command.
	 * @param param Set to any parameters.
	 * @param message Message that should be sent to the chat.
	 * @param status Message that should be shown in the status line.
	 * @return True if the command was processed, false otherwise.
	 */
	static bool checkCommand(tstring& cmd, tstring& param, tstring& message, tstring& status);

	static int getTextWidth(const tstring& str, HWND hWnd);
	static int getTextWidth(const tstring& str, HDC dc);
	static int getTextWidth(HWND wnd, HFONT fnt);
	static int getTextHeight(HWND wnd, HFONT fnt);
	static int getTextHeight(HDC dc, HFONT fnt);
	static int getTextHeight(HDC dc);
	static int getTextSpacing(HWND wnd, HFONT fnt); 


	static void addLastDir(const tstring& dir);

	static tstring getHelpFile() {
		return Text::toT(Util::getAppPath() + "DCPlusPlus.chm");
	}
	static bool browseFile(tstring& target, HWND owner = NULL, bool save = true, const tstring& initialDir = Util::emptyStringW, const TCHAR* types = NULL, const TCHAR* defExt = NULL);
	static bool browseDirectory(tstring& target, HWND owner = NULL);

	static void searchHash(const TTHValue* /*aHash*/);
	static void registerDchubHandler();
	static void registerADChubHandler();
	static void unRegisterDchubHandler();
	static void unRegisterADChubHandler();
	static void parseDchubUrl(const tstring& /*aUrl*/);
	static void parseADChubUrl(const tstring& /*aUrl*/);
	static bool urlDcADCRegistered;
	static bool urlMagnetRegistered;
	static void openLink(const tstring& url);
	static void openFile(const tstring& file) {
		::ShellExecute(NULL, NULL, file.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}

	static int getIconIndex(const tstring& aFileName);

	static int getDirIconIndex() { return dirIconIndex; }
	static int getDirMaskedIndex() { return dirMaskedIndex; }
	
	static int getOsMajor();
	static int getOsMinor();

	//returns the position where the context menu should be
	//opened if it was invoked from the keyboard.
	//aPt is relative to the screen not the control.
	static void getContextMenuPos(CListViewCtrl& aList, POINT& aPt);
	static void getContextMenuPos(CTreeViewCtrl& aTree, POINT& aPt);
	static void getContextMenuPos(CFulEditCtrl& aEdit,	POINT& aPt);
	
	static bool getUCParams(HWND parent, const UserCommand& cmd, StringMap& sm) throw();

	static void splitTokens(int* array, const string& tokens, int maxItems = -1) throw();
	static void saveHeaderOrder(CListViewCtrl& ctrl, SettingsManager::StrSetting order, 
		SettingsManager::StrSetting widths, int n, int* indexes, int* sizes) throw();

	static bool isShift() { return (GetKeyState(VK_SHIFT) & 0x8000) > 0; }

	template<class T> static HWND hiddenCreateEx(T& p) throw() {
		HWND active = (HWND)::SendMessage(mdiClient, WM_MDIGETACTIVE, 0, 0);
		::LockWindowUpdate(mdiClient);
		HWND ret = p.CreateEx(mdiClient);
		if(active && ::IsWindow(active))
			::SendMessage(mdiClient, WM_MDIACTIVATE, (WPARAM)active, 0);
		::LockWindowUpdate(0);
		return ret;
	}
	template<class T> static HWND hiddenCreateEx(T* p) throw() {
		return hiddenCreateEx(*p);
	}

private:
	static int CALLBACK browseCallbackProc(HWND hwnd, UINT uMsg, LPARAM /*lp*/, LPARAM pData);		
	
};

#endif // !defined(WIN_UTIL_H)

/**
 * @file
 * $Id: WinUtil.h,v 1.13 2004/02/15 16:58:49 trem Exp $
 */
