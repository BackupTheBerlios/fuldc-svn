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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "WinUtil.h"
#include "PrivateFrame.h"
#include "SearchFrm.h"
#include "LineDlg.h"
#include "../client/WebShortcuts.h"

#include "../client/Util.h"
#include "../client/StringTokenizer.h"
#include "../client/ShareManager.h"
#include "../client/ClientManager.h"
#include "../client/TimerManager.h"
#include "../client/FavoriteManager.h"
#include "../client/ResourceManager.h"
#include "../client/QueueManager.h"
#include "../client/UploadManager.h"
#include "../client/HashManager.h"
#include "../client/LogManager.h"
#include "../client/version.h"

#include "HubFrame.h"
#include "../client/SimpleXML.h"

#include "../client/pme.h"

#include <direct.h>
#include <pdh.h>
#include <WinInet.h>

WinUtil::ImageMap WinUtil::fileIndexes;
int WinUtil::fileImageCount;
HBRUSH WinUtil::bgBrush = NULL;
COLORREF WinUtil::textColor = 0;
COLORREF WinUtil::bgColor = 0;
HFONT WinUtil::font = NULL;
int WinUtil::fontHeight = 0;
HFONT WinUtil::boldFont = NULL;
HFONT WinUtil::tabFont = NULL;
HFONT WinUtil::systemFont = NULL;
HFONT WinUtil::monoFont = NULL;
CMenu WinUtil::mainMenu;
CImageList WinUtil::fileImages;
CImageList WinUtil::userImages;
int WinUtil::dirIconIndex = 0;
int WinUtil::dirMaskedIndex = 0;
TStringList WinUtil::lastDirs;
HWND WinUtil::mainWnd = NULL;
HWND WinUtil::mdiClient = NULL;
FlatTabCtrl* WinUtil::tabCtrl = NULL;
HHOOK WinUtil::hook = NULL;
tstring WinUtil::tth;
StringPairList WinUtil::initialDirs;
DWORD WinUtil::helpCookie = 0;
bool WinUtil::urlDcADCRegistered = false;
bool WinUtil::urlMagnetRegistered = false;
HWND WinUtil::findDialog = NULL;
const time_t WinUtil::startTime = GET_TIME();
DWORD WinUtil::comCtlVersion = 0;

HLSCOLOR RGB2HLS (COLORREF rgb) {
	unsigned char minval = min(GetRValue(rgb), min(GetGValue(rgb), GetBValue(rgb)));
	unsigned char maxval = max(GetRValue(rgb), max(GetGValue(rgb), GetBValue(rgb)));
	float mdiff  = float(maxval) - float(minval);
	float msum   = float(maxval) + float(minval);

	float luminance = msum / 510.0f;
	float saturation = 0.0f;
	float hue = 0.0f; 

	if ( maxval != minval ) { 
		float rnorm = (maxval - GetRValue(rgb)  ) / mdiff;      
		float gnorm = (maxval - GetGValue(rgb)) / mdiff;
		float bnorm = (maxval - GetBValue(rgb) ) / mdiff;   

		saturation = (luminance <= 0.5f) ? (mdiff / msum) : (mdiff / (510.0f - msum));

		if (GetRValue(rgb) == maxval) hue = 60.0f * (6.0f + bnorm - gnorm);
		if (GetGValue(rgb) == maxval) hue = 60.0f * (2.0f + rnorm - bnorm);
		if (GetBValue(rgb) == maxval) hue = 60.0f * (4.0f + gnorm - rnorm);
		if (hue > 360.0f) hue = hue - 360.0f;
	}
	return HLS ((hue*255)/360, luminance*255, saturation*255);
}

static BYTE _ToRGB (float rm1, float rm2, float rh) {
	if      (rh > 360.0f) rh -= 360.0f;
	else if (rh <   0.0f) rh += 360.0f;

	if      (rh <  60.0f) rm1 = rm1 + (rm2 - rm1) * rh / 60.0f;   
	else if (rh < 180.0f) rm1 = rm2;
	else if (rh < 240.0f) rm1 = rm1 + (rm2 - rm1) * (240.0f - rh) / 60.0f;      

	return (BYTE)(rm1 * 255);
}

COLORREF HLS2RGB (HLSCOLOR hls) {
	float hue        = ((int)HLS_H(hls)*360)/255.0f;
	float luminance  = HLS_L(hls)/255.0f;
	float saturation = HLS_S(hls)/255.0f;

	if ( saturation == 0.0f ) {
		return RGB (HLS_L(hls), HLS_L(hls), HLS_L(hls));
	}
	float rm1, rm2;

	if ( luminance <= 0.5f ) rm2 = luminance + luminance * saturation;  
	else                     rm2 = luminance + saturation - luminance * saturation;
	rm1 = 2.0f * luminance - rm2;   
	BYTE red   = _ToRGB (rm1, rm2, hue + 120.0f);   
	BYTE green = _ToRGB (rm1, rm2, hue);
	BYTE blue  = _ToRGB (rm1, rm2, hue - 120.0f);

	return RGB (red, green, blue);
}

COLORREF HLS_TRANSFORM (COLORREF rgb, int percent_L, int percent_S) {
	HLSCOLOR hls = RGB2HLS (rgb);
	BYTE h = HLS_H(hls);
	BYTE l = HLS_L(hls);
	BYTE s = HLS_S(hls);

	if ( percent_L > 0 ) {
		l = BYTE(l + ((255 - l) * percent_L) / 100);
	} else if ( percent_L < 0 )	{
		l = BYTE((l * (100+percent_L)) / 100);
	}
	if ( percent_S > 0 ) {
		s = BYTE(s + ((255 - s) * percent_S) / 100);
	} else if ( percent_S < 0 ) {
		s = BYTE((s * (100+percent_S)) / 100);
	}
	return HLS2RGB (HLS(h, l, s));
}


bool WinUtil::getVersionInfo(OSVERSIONINFOEX& ver) {
	memset(&ver, 0, sizeof(OSVERSIONINFOEX));
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if(!GetVersionEx((OSVERSIONINFO*)&ver)) {
		ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if(!GetVersionEx((OSVERSIONINFO*)&ver)) {
			return false;
		}
	}
	return true;
}

static LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
	if(code == HC_ACTION) {
		if(wParam == VK_CONTROL && LOWORD(lParam) == 1) {
			if(lParam & 0x80000000) {
				WinUtil::tabCtrl->endSwitch();
			} else {
				WinUtil::tabCtrl->startSwitch();
			}
		}
	}
	return CallNextHookEx(WinUtil::hook, code, wParam, lParam);
}


void WinUtil::init(HWND hWnd) {
	mainWnd = hWnd;

	mainMenu.CreateMenu();

	CMenuHandle file;
	file.CreatePopupMenu();

	file.AppendMenu(MF_STRING, IDC_OPEN_FILE_LIST, CTSTRING(MENU_OPEN_FILE_LIST));
	file.AppendMenu(MF_STRING, IDC_OPEN_OWN_LIST, CTSTRING(MENU_OPEN_OWN_LIST));
	file.AppendMenu(MF_STRING, IDC_REFRESH_FILE_LIST, CTSTRING(MENU_REFRESH_FILE_LIST));
	file.AppendMenu(MF_STRING, IDC_OPEN_DOWNLOADS, CTSTRING(MENU_OPEN_DOWNLOADS_DIR));
	file.AppendMenu(MF_SEPARATOR);
	file.AppendMenu(MF_STRING, ID_FILE_QUICK_CONNECT, CTSTRING(MENU_QUICK_CONNECT));
	file.AppendMenu(MF_STRING, IDC_FOLLOW, CTSTRING(MENU_FOLLOW_REDIRECT));
	file.AppendMenu(MF_STRING, ID_FILE_RECONNECT, CTSTRING(MENU_RECONNECT));
	file.AppendMenu(MF_SEPARATOR);
	file.AppendMenu(MF_STRING, ID_FILE_SETTINGS, CTSTRING(MENU_SETTINGS));
	file.AppendMenu(MF_SEPARATOR);
	file.AppendMenu(MF_STRING, ID_APP_EXIT, CTSTRING(MENU_EXIT));

	mainMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)file, CTSTRING(MENU_FILE));

	CMenuHandle view;
	view.CreatePopupMenu();

	view.AppendMenu(MF_STRING, ID_FILE_CONNECT, CTSTRING(MENU_PUBLIC_HUBS));
	view.AppendMenu(MF_STRING, IDC_QUEUE, CTSTRING(MENU_DOWNLOAD_QUEUE));
	view.AppendMenu(MF_STRING, IDC_VIEW_WAITING_USERS, CTSTRING(WAITING_USERS));
	view.AppendMenu(MF_STRING, IDC_FINISHED, CTSTRING(FINISHED_DOWNLOADS));
	view.AppendMenu(MF_STRING, IDC_FINISHED_UL, CTSTRING(FINISHED_UPLOADS));
	view.AppendMenu(MF_STRING, IDC_FAVORITES, CTSTRING(MENU_FAVORITE_HUBS));
	view.AppendMenu(MF_STRING, IDC_FAVUSERS, CTSTRING(MENU_FAVORITE_USERS));
	view.AppendMenu(MF_STRING, ID_FILE_SEARCH, CTSTRING(MENU_SEARCH));
	view.AppendMenu(MF_STRING, IDC_FILE_ADL_SEARCH, CTSTRING(MENU_ADL_SEARCH));
	view.AppendMenu(MF_STRING, IDC_SEARCH_SPY, CTSTRING(MENU_SEARCH_SPY));
	view.AppendMenu(MF_STRING, IDC_NOTEPAD, CTSTRING(MENU_NOTEPAD));
	view.AppendMenu(MF_STRING, IDC_NET_STATS, CTSTRING(MENU_NETWORK_STATISTICS));
	view.AppendMenu(MF_STRING, IDC_HASH_PROGRESS, CTSTRING(MENU_HASH_PROGRESS));
	view.AppendMenu(MF_STRING, IDC_SYSTEM_LOG, CTSTRING(MENU_SYSTEM_LOG));
	view.AppendMenu(MF_SEPARATOR);
	view.AppendMenu(MF_STRING, ID_VIEW_TOOLBAR, CTSTRING(MENU_TOOLBAR));
	view.AppendMenu(MF_STRING, ID_VIEW_STATUS_BAR, CTSTRING(MENU_STATUS_BAR));
	view.AppendMenu(MF_STRING, ID_VIEW_TRANSFER_VIEW, CTSTRING(MENU_TRANSFER_VIEW));

	mainMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)view, CTSTRING(MENU_VIEW));

	CMenuHandle window;
	window.CreatePopupMenu();

	window.AppendMenu(MF_STRING, ID_WINDOW_CASCADE, CTSTRING(MENU_CASCADE));
	window.AppendMenu(MF_STRING, ID_WINDOW_TILE_HORZ, CTSTRING(MENU_HORIZONTAL_TILE));
	window.AppendMenu(MF_STRING, ID_WINDOW_TILE_VERT, CTSTRING(MENU_VERTICAL_TILE));
	window.AppendMenu(MF_STRING, ID_WINDOW_ARRANGE, CTSTRING(MENU_ARRANGE));
	window.AppendMenu(MF_STRING, ID_WINDOW_MINIMIZE_ALL, CTSTRING(MENU_MINIMIZE_ALL));
	window.AppendMenu(MF_STRING, ID_WINDOW_RESTORE_ALL, CTSTRING(MENU_RESTORE_ALL));
	window.AppendMenu(MF_SEPARATOR);
	window.AppendMenu(MF_STRING, IDC_CLOSE_DISCONNECTED, CTSTRING(MENU_CLOSE_DISCONNECTED));
	window.AppendMenu(MF_STRING, IDC_CLOSE_ALL_PM, CTSTRING(MENU_CLOSE_ALL_PM));
	window.AppendMenu(MF_STRING, IDC_CLOSE_ALL_OFFLINE_PM, CTSTRING(MENU_CLOSE_ALL_OFFLINE_PM));
	window.AppendMenu(MF_STRING, IDC_CLOSE_ALL_DIR_LIST, CTSTRING(MENU_CLOSE_ALL_DIR_LIST));
	window.AppendMenu(MF_STRING, IDC_CLOSE_ALL_SEARCH_FRAME, CTSTRING(MENU_CLOSE_ALL_SEARCHFRAME));

	mainMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)window, CTSTRING(MENU_WINDOW));

	CMenuHandle help;
	help.CreatePopupMenu();

	help.AppendMenu(MF_STRING, IDC_HELP_CONTENTS, CTSTRING(MENU_CONTENTS));
	help.AppendMenu(MF_SEPARATOR);
	help.AppendMenu(MF_STRING, IDC_HELP_CHANGELOG, CTSTRING(MENU_CHANGELOG));
	help.AppendMenu(MF_STRING, ID_APP_ABOUT, CTSTRING(MENU_ABOUT));
	help.AppendMenu(MF_SEPARATOR);
	help.AppendMenu(MF_STRING, IDC_HELP_FULPAGE, CTSTRING(MENU_FULPAGE));
	help.AppendMenu(MF_STRING, IDC_HELP_HOMEPAGE, CTSTRING(MENU_HOMEPAGE));
	help.AppendMenu(MF_STRING, IDC_HELP_DOWNLOADS, CTSTRING(MENU_HELP_DOWNLOADS));
	//help.AppendMenu(MF_STRING, IDC_HELP_GEOIPFILE, CTSTRING(MENU_HELP_GEOIPFILE));
	help.AppendMenu(MF_STRING, IDC_HELP_TRANSLATIONS, CTSTRING(MENU_HELP_TRANSLATIONS));
	help.AppendMenu(MF_STRING, IDC_HELP_FAQ, CTSTRING(MENU_FAQ));
	help.AppendMenu(MF_STRING, IDC_HELP_HELP_FORUM, CTSTRING(MENU_HELP_FORUM));
	help.AppendMenu(MF_STRING, IDC_HELP_DISCUSS, CTSTRING(MENU_DISCUSS));
	//help.AppendMenu(MF_STRING, IDC_HELP_REQUEST_FEATURE, CTSTRING(MENU_REQUEST_FEATURE));
	//help.AppendMenu(MF_STRING, IDC_HELP_REPORT_BUG, CTSTRING(MENU_REPORT_BUG));
	help.AppendMenu(MF_STRING, IDC_HELP_DONATE, CTSTRING(MENU_DONATE));

	mainMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)help, CTSTRING(MENU_HELP));

/** @todo fix this so that the system icon is used for dirs as well (we need
			  to mask it so that incomplete folders appear correct */
#if 0	
	if(BOOLSETTING(USE_SYSTEM_ICONS)) {
		SHFILEINFO fi;
		fileImages.Create(16, 16, ILC_COLOR32 | ILC_MASK, 16, 16);
		::SHGetFileInfo(_T("."), FILE_ATTRIBUTE_DIRECTORY, &fi, sizeof(fi), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
		fileImages.AddIcon(fi.hIcon);
		fileImages.AddIcon(ic);
		::DestroyIcon(fi.hIcon);
	} else {
		fileImages.CreateFromImage(_T("icons\\folders.bmp"), 16, 3, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED | LR_LOADFROMFILE);
	}
#endif

	fileImages.CreateFromImage(_T("icons\\folders.bmp"), 16, 3, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED | LR_LOADFROMFILE);
	dirIconIndex = fileImageCount++;
	dirMaskedIndex = fileImageCount++;

	fileImageCount++;

	userImages.CreateFromImage(_T("icons\\users.bmp"), 16, 8, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED | LR_LOADFROMFILE);

	LOGFONT lf, lf2;
	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);
	SettingsManager::getInstance()->setDefault(SettingsManager::TEXT_FONT, Text::fromT(encodeFont(lf)));
	decodeFont(Text::toT(SETTING(TEXT_FONT)), lf);
	::GetObject((HFONT)GetStockObject(ANSI_FIXED_FONT), sizeof(lf2), &lf2);
	
	lf2.lfHeight = lf.lfHeight;
	lf2.lfWeight = lf.lfWeight;
	lf2.lfItalic = lf.lfItalic;

	bgBrush = CreateSolidBrush(SETTING(BACKGROUND_COLOR));
	textColor = SETTING(TEXT_COLOR);
	bgColor = SETTING(BACKGROUND_COLOR);
	font = ::CreateFontIndirect(&lf);
	fontHeight = WinUtil::getTextHeight(mainWnd, font);
	lf.lfWeight = FW_BOLD;
	boldFont = ::CreateFontIndirect(&lf);
	lf.lfWeight = FW_BLACK;
	lf.lfHeight *= 3;
	lf.lfHeight /= 4;
	lf.lfWidth  *= 3;
	lf.lfWidth  /= 4;
	tabFont = ::CreateFontIndirect(&lf);
	systemFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
	monoFont = (HFONT)::GetStockObject(BOOLSETTING(USE_OEM_MONOFONT)?OEM_FIXED_FONT:ANSI_FIXED_FONT);

	if(BOOLSETTING(URL_HANDLER)) {
		registerDchubHandler();
		registerADChubHandler();
		urlDcADCRegistered = true;
	} 

	hook = SetWindowsHookEx(WH_KEYBOARD, &KeyboardProc, NULL, GetCurrentThreadId());

	HtmlHelp(NULL, NULL, HH_INITIALIZE, (DWORD)&helpCookie);
}

void WinUtil::uninit() {
	HtmlHelp(NULL, NULL, HH_UNINITIALIZE, helpCookie);

	fileImages.Destroy();
	userImages.Destroy();
	::DeleteObject(font);
	::DeleteObject(boldFont);
	::DeleteObject(bgBrush);
	::DeleteObject(monoFont);
	::DeleteObject(tabFont);

	mainMenu.DestroyMenu();

	UnhookWindowsHookEx(hook);

}

void WinUtil::decodeFont(const tstring& setting, LOGFONT &dest) {
	StringTokenizer<tstring> st(setting, _T(','));
	TStringList &sl = st.getTokens();
	
	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(dest), &dest);
	tstring face;
	if(sl.size() == 4)
	{
		face = sl[0];
		dest.lfHeight = Util::toInt(Text::fromT(sl[1]));
		dest.lfWeight = Util::toInt(Text::fromT(sl[2]));
		dest.lfItalic = (BYTE)Util::toInt(Text::fromT(sl[3]));
	}
	
	if(!face.empty()) {
		::ZeroMemory(dest.lfFaceName, LF_FACESIZE);
		_tcscpy(dest.lfFaceName, face.c_str());
	}
}

int CALLBACK WinUtil::browseCallbackProc(HWND hwnd, UINT uMsg, LPARAM /*lp*/, LPARAM pData) {
	switch(uMsg) {
	case BFFM_INITIALIZED: 
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;
	}
	return 0;
}

bool WinUtil::browseDirectory(tstring& target, HWND owner /* = NULL */) {
	TCHAR buf[MAX_PATH];
	BROWSEINFO bi;
	LPMALLOC ma;
	
	ZeroMemory(&bi, sizeof(bi));
	
	bi.hwndOwner = owner;
	bi.pszDisplayName = buf;
	bi.lpszTitle = CTSTRING(CHOOSE_FOLDER);
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
	bi.lParam = (LPARAM)target.c_str();
	bi.lpfn = &browseCallbackProc;
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if(pidl != NULL) {
		SHGetPathFromIDList(pidl, buf);
		target = buf;
		
		if(target.size() > 0 && target[target.size()-1] != L'\\')
			target+=L'\\';
		
		if(SHGetMalloc(&ma) != E_FAIL) {
			ma->Free(pidl);
			ma->Release();
		}
		return true;
	}
	return false;
}

bool WinUtil::browseFile(tstring& target, HWND owner /* = NULL */, bool save /* = true */, const tstring& initialDir /* = Util::emptyString */, const TCHAR* types /* = NULL */, const TCHAR* defExt /* = NULL */) {
	TCHAR buf[MAX_PATH];
	OPENFILENAME ofn = { 0 };       // common dialog box structure
	target = Text::toT(Util::validateFileName(Text::fromT(target)));
	_tcscpy(buf, target.c_str());
	// Initialize OPENFILENAME
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.hwndOwner = owner;
	ofn.lpstrFile = buf;
	ofn.lpstrFilter = types;
	ofn.lpstrDefExt = defExt;
	ofn.nFilterIndex = 1;

	if(!initialDir.empty()) {
		ofn.lpstrInitialDir = initialDir.c_str();
	}
	ofn.nMaxFile = sizeof(buf);
	ofn.Flags = (save ? 0: OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST);
	
	// Display the Open dialog box. 
	if ( (save ? GetSaveFileName(&ofn) : GetOpenFileName(&ofn) ) ==TRUE) {
		target = ofn.lpstrFile;
		return true;
	}
	return false;
}

void WinUtil::setClipboard(const tstring& str) {
	if(!::OpenClipboard(mainWnd)) {
		return;
	}

	EmptyClipboard();

#ifdef UNICODE	
	OSVERSIONINFOEX ver;
	if( WinUtil::getVersionInfo(ver) ) {
		if( ver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) {
			string tmp = Text::wideToAcp(str);

			HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (tmp.size() + 1) * sizeof(char)); 
			if (hglbCopy == NULL) { 
				CloseClipboard(); 
				return; 
			} 

			// Lock the handle and copy the text to the buffer. 
			char* lptstrCopy = (char*)GlobalLock(hglbCopy); 
			strcpy(lptstrCopy, tmp.c_str());
			GlobalUnlock(hglbCopy);

			SetClipboardData(CF_TEXT, hglbCopy);

			CloseClipboard();

			return;
		}
	}
#endif

	// Allocate a global memory object for the text. 
	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (str.size() + 1) * sizeof(TCHAR)); 
	if (hglbCopy == NULL) { 
		CloseClipboard(); 
		return; 
	} 

	// Lock the handle and copy the text to the buffer. 
	TCHAR* lptstrCopy = (TCHAR*)GlobalLock(hglbCopy); 
	_tcscpy(lptstrCopy, str.c_str());
	GlobalUnlock(hglbCopy); 

	// Place the handle on the clipboard.
#ifdef UNICODE
    SetClipboardData(CF_UNICODETEXT, hglbCopy); 
#else
	SetClipboardData(CF_TEXT hglbCopy);
#endif

	CloseClipboard();
}

void WinUtil::splitTokens(int* array, const string& tokens, int maxItems /* = -1 */) throw() {
	StringTokenizer<string> t(tokens, _T(','));
	StringList& l = t.getTokens();
	if(maxItems == -1)
		maxItems = l.size();
	
	int k = 0;
	for(StringList::const_iterator i = l.begin(); i != l.end() && k < maxItems; ++i, ++k) {
		array[k] = Util::toInt(*i);
	}
}

bool WinUtil::getUCParams(HWND parent, const UserCommand& uc, StringMap& sm) throw() {
	string::size_type i = 0;
	StringMap done;

	while( (i = uc.getCommand().find("%[line:", i)) != string::npos) {
		i += 7;
		string::size_type j = uc.getCommand().find(']', i);
		if(j == string::npos)
			break;

		string name = uc.getCommand().substr(i, j-i);
		if(done.find(name) == done.end()) {
			LineDlg dlg;
			dlg.title = Text::toT(uc.getName());
			dlg.description = Text::toT(name);
			dlg.line = Text::toT(sm["line:" + name]);
			if(dlg.DoModal(parent) == IDOK) {
				sm["line:" + name] = Text::fromT(dlg.line);
				done[name] = Text::fromT(dlg.line);
			} else {
				return false;
			}
		}
		i = j + 1;
	}
	return true;
}

#define LINE2 _T("-- http://dcplusplus.sourceforge.net  <DC++ ") _T(VERSIONSTRING) _T(">")
TCHAR *msgs[] = { _T("\r\n-- I'm a happy dc++ user. You could be happy too.\r\n") LINE2,
_T("\r\n-- Neo-...what? Nope...never heard of it...\r\n") LINE2,
_T("\r\n-- Evolution of species: Ape --> Man\r\n-- Evolution of science: \"The Earth is Flat\" --> \"The Earth is Round\"\r\n-- Evolution of sharing: NMDC --> DC++\r\n") LINE2,
_T("\r\n-- I share, therefore I am.\r\n") LINE2,
_T("\r\n-- I came, I searched, I found...\r\n") LINE2,
_T("\r\n-- I came, I shared, I sent...\r\n") LINE2,
_T("\r\n-- I can set away mode, can't you?\r\n") LINE2,
_T("\r\n-- I don't have to see any ads, do you?\r\n") LINE2,
_T("\r\n-- I don't have to see those annoying kick messages, do you?\r\n") LINE2,
_T("\r\n-- I can resume my files to a different filename, can you?\r\n") LINE2,
_T("\r\n-- I can share huge amounts of files, can you?\r\n") LINE2,
_T("\r\n-- My client doesn't spam the chat with useless debug messages, does yours?\r\n") LINE2,
_T("\r\n-- I can add multiple users to the same download and have the client connect to another automatically when one goes offline, can you?\r\n") LINE2,
_T("\r\n-- These addies are pretty annoying, aren't they? Get revenge by sending them yourself!\r\n") LINE2,
_T("\r\n-- My client supports TTH hashes, does yours?\r\n") LINE2,
_T("\r\n-- My client supports XML file lists, does yours?\r\n") LINE2
};

#define MSGS 16

bool WinUtil::checkCommand(tstring& cmd, tstring& param, tstring& message, tstring& status) {
	string::size_type i = cmd.find(' ');
	if(i != string::npos) {
		param = cmd.substr(i+1);
		cmd = cmd.substr(1, i - 1);
	} else {
		cmd = cmd.substr(1);
	}

	if(Util::stricmp(cmd.c_str(), _T("showlog")) == 0) {
		StringMap params;
		string path;
		if(Util::stricmp(param.c_str(), _T("system")) == 0) {
			path = LogManager::getInstance()->getLogFilename(LogManager::STATUS, params);
		} else if(Util::stricmp(param.c_str(), _T("downloads")) == 0) {
			path = LogManager::getInstance()->getLogFilename(LogManager::DOWNLOAD, params);
		} else if(Util::stricmp(param.c_str(), _T("uploads")) == 0) {
			path = LogManager::getInstance()->getLogFilename(LogManager::UPLOAD, params);
		} else {
			return false;
		}

		if(!path.empty())
			WinUtil::openFile(Text::toT(path));

	} else if(Util::stricmp(cmd.c_str(), _T("refresh"))==0) {
		try {
			ShareManager::getInstance()->setDirty();
			if(!param.empty()) {
				if( ShareManager::REFRESH_PATH_NOT_FOUND == ShareManager::getInstance()->refresh( Text::fromT(param) ) )
					status = TSTRING(DIRECTORY_NOT_FOUND);
			} else {
				ShareManager::getInstance()->refresh(true);
			}
		} catch(const ShareException& e) {
			status = Text::toT(e.getError());
		}
	} else if(Util::stricmp(cmd.c_str(), _T("refreshi")) == 0) {
		try {
			ShareManager::getInstance()->refresh(false, true, false, true);
		} catch( const ShareException& e) {
			status = Text::toT(e.getError());
		}
	} else if(Util::stricmp(cmd.c_str(), _T("share")) == 0){
		if(!param.empty()){
			try{
				string path = Text::fromT( param );
				ShareManager::getInstance()->addDirectory( path, Util::getLastDir( path ) );
				status = TSTRING(ADDED) + L" " + param;
			}catch(ShareException &se){
				status = Text::toT(se.getError());
			}
		}	
	} else if(Util::stricmp(cmd.c_str(), _T("unshare")) == 0) {
		if(!param.empty()){
			string t = Text::fromT(param);
			ShareManager::getInstance()->removeDirectory( t );
			HashManager::getInstance()->stopHashing( t );
			status = TSTRING(REMOVED) + _T(" ") + param;
		}
	} else if(Util::stricmp(cmd.c_str(), _T("slots"))==0) {
		if(param.empty()) {
			int slots = SettingsManager::getInstance()->get(SettingsManager::SLOTS);
			status = TSTRING(CURRENT_SLOTS) + _T(" ") + Util::toStringW(slots);
		}else {
			int j = Util::toInt(param);
			if(j > 0) {
				SettingsManager::getInstance()->set(SettingsManager::SLOTS, j);
				status = TSTRING(SLOTS_SET);
				ClientManager::getInstance()->infoUpdated();
			} else {
				status = TSTRING(INVALID_NUMBER_OF_SLOTS);
			}
		}
	} else if(Util::stricmp(cmd.c_str(), _T("search")) == 0) {
		if(!param.empty()) {
			SearchFrame::openWindow(param);
		} else {
			status = TSTRING(SPECIFY_SEARCH_STRING);
		}
	} else if(Util::stricmp(cmd.c_str(), _T("dc++")) == 0) {
		message = msgs[GET_TICK() % MSGS];
	} else if(Util::stricmp(cmd.c_str(), _T("away")) == 0) {
		if(Util::getAway() && param.empty()) {
			Util::setAway(false);
			status = TSTRING(AWAY_MODE_OFF);
		} else {
			Util::setAway(true);
			Util::setAwayMessage(Text::fromT(param));
			status = TSTRING(AWAY_MODE_ON) + Text::toT(Util::getAwayMessage());
		}
	} else if(Util::stricmp(cmd.c_str(), _T("back")) == 0) {
		Util::setAway(false);
		status = TSTRING(AWAY_MODE_OFF);
	} else if(Util::stricmp(cmd.c_str(), _T("dslots")) == 0) {
		if(param.empty()) {
			int slots = SettingsManager::getInstance()->get(SettingsManager::DOWNLOAD_SLOTS);
			status = TSTRING(CURRENT_DSLOTS) + _T(" ") + Util::toStringW(slots);
		} else {
			int nr = Util::toInt(param);
			if( nr >= 0 ){
				SettingsManager::getInstance()->set(SettingsManager::DOWNLOAD_SLOTS, nr);
				status = TSTRING(DSLOTS_SET);
			} else {
				status = TSTRING(INVALID_SLOTS);
			}
		} 
	}else if(Util::stricmp(cmd.c_str(), _T("fuldc")) == 0) {
		message = _T("http://ful.dcportal.net <fulDC ") _T(FULVERSIONSTRING) _T(">");
	} else if(Util::stricmp(cmd.c_str(), _T("info")) == 0) {
		message = WinUtil::UselessInfo();
	} else if(Util::stricmp(cmd.c_str(), _T("fuptime")) == 0) {
		message = TSTRING(FULDC_UPTIME) + _T(" ") + Util::formatTimeW(GET_TIME() - WinUtil::startTime, false);
	} else if(Util::stricmp(cmd.c_str(), _T("uptime")) == 0) {
		message = TSTRING(SYSTEM_UPTIME) + _T(" ") + WinUtil::Uptime();
	} else if(WebShortcuts::getInstance()->getShortcutByKey(cmd) != NULL) {
		WinUtil::SearchSite(WebShortcuts::getInstance()->getShortcutByKey(cmd), param);
	} else if(Util::stricmp(cmd.c_str(), _T("rebuild")) == 0) {
		HashManager::getInstance()->rebuild();
	}else if(Util::stricmp(cmd.c_str(), _T("df")) == 0) {
		message = WinUtil::DiskSpaceInfo();
	} else if(Util::stricmp(cmd.c_str(), _T("regen")) == 0) {
		ShareManager::getInstance()->generateXmlList(true);
	} else if(Util::stricmp(cmd.c_str(), _T("help")) == 0) {
		HtmlHelp(mainWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDC_CHATCOMMANDS);
	} else {
		return false;
	}

	return true;
}

void WinUtil::searchHash(const TTHValue* aHash) {
	 if(aHash != NULL) {
		 SearchFrame::openWindow(Text::toT(aHash->toBase32()), 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
	 }
 }

 void WinUtil::registerDchubHandler() {
	HKEY hk;
	TCHAR Buf[512];
	tstring app = _T("\"") + Text::toT(Util::getAppName()) + _T("\" %1");
	Buf[0] = 0;

	if(::RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("dchub\\Shell\\Open\\Command"), 0, KEY_WRITE | KEY_READ, &hk) == ERROR_SUCCESS) {
		DWORD bufLen = sizeof(Buf);
		DWORD type;
		::RegQueryValueEx(hk, NULL, 0, &type, (LPBYTE)Buf, &bufLen);
		::RegCloseKey(hk);
	}

	if(Util::stricmp(app.c_str(), Buf) != 0) {
		if (::RegCreateKeyEx(HKEY_CLASSES_ROOT, _T("dchub"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL))  {
			LogManager::getInstance()->message(STRING(ERROR_CREATING_REGISTRY_KEY_DCHUB));
			return;
		}
	
		TCHAR* tmp = _T("URL:Direct Connect Protocol");
		::RegSetValueEx(hk, NULL, 0, REG_SZ, (LPBYTE)tmp, sizeof(TCHAR) * (_tcslen(tmp) + 1));
		::RegSetValueEx(hk, _T("URL Protocol"), 0, REG_SZ, (LPBYTE)_T(""), sizeof(TCHAR));
		::RegCloseKey(hk);

		::RegCreateKeyEx(HKEY_CLASSES_ROOT, _T("dchub\\Shell\\Open\\Command"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL);
		::RegSetValueEx(hk, _T(""), 0, REG_SZ, (LPBYTE)app.c_str(), sizeof(TCHAR) * (app.length() + 1));
		::RegCloseKey(hk);

		::RegCreateKeyEx(HKEY_CLASSES_ROOT, _T("dchub\\DefaultIcon"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL);
		app = Text::toT(Util::getAppName());
		::RegSetValueEx(hk, _T(""), 0, REG_SZ, (LPBYTE)app.c_str(), sizeof(TCHAR) * (app.length() + 1));
		::RegCloseKey(hk);
	}
}

 void WinUtil::unRegisterDchubHandler() {
	SHDeleteKey(HKEY_CLASSES_ROOT, _T("dchub"));
 }

 void WinUtil::registerADChubHandler() {
	 HKEY hk;
	 TCHAR Buf[512];
	 tstring app = _T("\"") + Text::toT(Util::getAppName()) + _T("\" %1");
	 Buf[0] = 0;

	 if(::RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("adc\\Shell\\Open\\Command"), 0, KEY_WRITE | KEY_READ, &hk) == ERROR_SUCCESS) {
		 DWORD bufLen = sizeof(Buf);
		 DWORD type;
		 ::RegQueryValueEx(hk, NULL, 0, &type, (LPBYTE)Buf, &bufLen);
		 ::RegCloseKey(hk);
	 }

	 if(Util::stricmp(app.c_str(), Buf) != 0) {
		 if (::RegCreateKeyEx(HKEY_CLASSES_ROOT, _T("adc"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL))  {
			 LogManager::getInstance()->message(STRING(ERROR_CREATING_REGISTRY_KEY_ADC));
			 return;
		 }

		 TCHAR* tmp = _T("URL:Direct Connect Protocol");
		 ::RegSetValueEx(hk, NULL, 0, REG_SZ, (LPBYTE)tmp, sizeof(TCHAR) * (_tcslen(tmp) + 1));
		 ::RegSetValueEx(hk, _T("URL Protocol"), 0, REG_SZ, (LPBYTE)_T(""), sizeof(TCHAR));
		 ::RegCloseKey(hk);

		 ::RegCreateKeyEx(HKEY_CLASSES_ROOT, _T("adc\\Shell\\Open\\Command"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL);
		 ::RegSetValueEx(hk, _T(""), 0, REG_SZ, (LPBYTE)app.c_str(), sizeof(TCHAR) * (app.length() + 1));
		 ::RegCloseKey(hk);

		 ::RegCreateKeyEx(HKEY_CLASSES_ROOT, _T("adc\\DefaultIcon"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL);
		 app = Text::toT(Util::getAppName());
		 ::RegSetValueEx(hk, _T(""), 0, REG_SZ, (LPBYTE)app.c_str(), sizeof(TCHAR) * (app.length() + 1));
		 ::RegCloseKey(hk);
	 }
 }

 void WinUtil::unRegisterADChubHandler() {
	SHDeleteKey(HKEY_CLASSES_ROOT, _T("adc"));
 }

void WinUtil::openLink(const tstring& url) {
	CRegKey key;
	TCHAR regbuf[MAX_PATH];
	ULONG len = MAX_PATH;
	tstring x;

	tstring::size_type i = url.find(_T("://"));
	if(i != string::npos) {
		x = url.substr(0, i);
	} else {
		x = _T("http");
	}
	x += _T("\\shell\\open\\command");
	if(key.Open(HKEY_CLASSES_ROOT, x.c_str(), KEY_READ) == ERROR_SUCCESS) {
		if(key.QueryStringValue(NULL, regbuf, &len) == ERROR_SUCCESS) {
			/*
			 * Various values (for http handlers):
			 *  C:\PROGRA~1\MOZILL~1\FIREFOX.EXE -url "%1"
			 *  "C:\Program Files\Internet Explorer\iexplore.exe" -nohome
			 *  "C:\Apps\Opera7\opera.exe"
			 *  C:\PROGRAMY\MOZILLA\MOZILLA.EXE -url "%1"
			 *  C:\PROGRA~1\NETSCAPE\NETSCAPE\NETSCP.EXE -url "%1"
			 */
			tstring cmd(regbuf); // otherwise you consistently get two trailing nulls
			
			if(cmd.length() > 1) {
				string::size_type start,end;
				if(cmd[0] == '"') {
					start = 1;
					end = cmd.find('"', 1);
				} else {
					start = 0;
					end = cmd.find(' ', 1);
				}
				if(end == string::npos)
					end = cmd.length();

				tstring cmdLine(cmd);
				cmd = cmd.substr(start, end-start);
				size_t arg_pos;
				if((arg_pos = cmdLine.find(_T("%1"))) != string::npos) {
					cmdLine.replace(arg_pos, 2, url);
				} else {
					cmdLine.append(_T(" \"") + url + _T('\"'));
				}

				STARTUPINFO si = { sizeof(si), 0 };
				PROCESS_INFORMATION pi = { 0 };
				AutoArray<TCHAR> buf(cmdLine.length() + 1);
				_tcscpy(buf, cmdLine.c_str());
				if(::CreateProcess(cmd.c_str(), buf, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
					::CloseHandle(pi.hThread);
					::CloseHandle(pi.hProcess);
					return;
				}
			}
		}
	}

	::ShellExecute(NULL, NULL, url.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void WinUtil::parseDchubUrl(const tstring& aUrl) {
	string server, file;
	u_int16_t port = 411;
	Util::decodeUrl(Text::fromT(aUrl), server, port, file);
	if(!server.empty()) {
		HubFrame::openWindow(Text::toT(server + ":" + Util::toString(port)));
	}
	if(!file.empty()) {
		if(file[0] == '/') // Remove any '/' in from of the file
			file = file.substr(1);
		try {
			/// @todo check this...
			QueueManager::getInstance()->addList(ClientManager::getInstance()->getLegacyUser(file), QueueItem::FLAG_CLIENT_VIEW);
		} catch(const Exception&) {
			// ...
		}
	}
}

void WinUtil::parseADChubUrl(const tstring& aUrl) {
	string server, file;
	u_int16_t port = 0; //make sure we get a port since adc doesn't have a standard one
	Util::decodeUrl(Text::fromT(aUrl), server, port, file);
	if(!server.empty() && port > 0) {
		HubFrame::openWindow(Text::toT("adc://" + server + ":" + Util::toString(port)));
	}
}

void WinUtil::saveHeaderOrder(CListViewCtrl& ctrl, SettingsManager::StrSetting order, 
							  SettingsManager::StrSetting widths, int n, 
							  int* indexes, int* sizes) throw() {
	string tmp;

	ctrl.GetColumnOrderArray(n, indexes);
	int i;
	for(i = 0; i < n; ++i) {
		tmp += Util::toString(indexes[i]);
		tmp += ',';
	}
	tmp.erase(tmp.size()-1, 1);
	SettingsManager::getInstance()->set(order, tmp);
	tmp.clear();
	for(i = 0; i < n; ++i) {
		sizes[i] = ctrl.GetColumnWidth(i);
		tmp += Util::toString(sizes[i]);
		tmp += ',';
	}
	tmp.erase(tmp.size()-1, 1);
	SettingsManager::getInstance()->set(widths, tmp);
}

int WinUtil::getIconIndex(const tstring& aFileName) {
	if(BOOLSETTING(USE_SYSTEM_ICONS)) {
		SHFILEINFO fi;
		string x = Text::toLower(Util::getFileExt(Text::fromT(aFileName)));
		if(!x.empty()) {
			ImageIter j = fileIndexes.find(x);
			if(j != fileIndexes.end())
				return j->second;
		}
		tstring fn = Text::toT(Text::toLower(Util::getFileName(Text::fromT(aFileName))));
		::SHGetFileInfo(fn.c_str(), FILE_ATTRIBUTE_NORMAL, &fi, sizeof(fi), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
		fileImages.AddIcon(fi.hIcon);
		::DestroyIcon(fi.hIcon);

		fileIndexes[x] = fileImageCount++;
		return fileImageCount - 1;
	} else {
		return 2;
	}
}

int WinUtil::getOsMajor() {
	OSVERSIONINFOEX ver;
	memset(&ver, 0, sizeof(OSVERSIONINFOEX));
	if(!GetVersionEx((OSVERSIONINFO*)&ver)) 
	{
		ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	}
	GetVersionEx((OSVERSIONINFO*)&ver);
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	return ver.dwMajorVersion;
}

int WinUtil::getOsMinor() 
{
	OSVERSIONINFOEX ver;
	memset(&ver, 0, sizeof(OSVERSIONINFOEX));
	if(!GetVersionEx((OSVERSIONINFO*)&ver)) 
	{
		ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	}
	GetVersionEx((OSVERSIONINFO*)&ver);
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	return ver.dwMinorVersion;
}

tstring WinUtil::getNicks(const CID& cid) throw() {
	return Text::toT(Util::toString(ClientManager::getInstance()->getNicks(cid)));
}

pair<tstring, bool> WinUtil::getHubNames(const CID& cid) throw() {
	StringList hubs = ClientManager::getInstance()->getHubNames(cid);
	if(hubs.empty()) {
		return make_pair(TSTRING(OFFLINE), false);
	} else {
		return make_pair(Text::toT(Util::toString(hubs)), true);
	}
}

void WinUtil::getContextMenuPos(CListViewCtrl& aList, POINT& aPt) {
	int pos = aList.GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED);
	if(pos >= 0) {
		CRect lrc;
		aList.GetItemRect(pos, &lrc, LVIR_LABEL);
		aPt.x = lrc.left;
		aPt.y = lrc.top + (lrc.Height() / 2);
	} else {
		aPt.x = aPt.y = 0;
	}
	aList.ClientToScreen(&aPt);
}

void WinUtil::getContextMenuPos(CTreeViewCtrl& aTree, POINT& aPt) {
	CRect trc;
	HTREEITEM ht = aTree.GetSelectedItem();
	if(ht) {
		aTree.GetItemRect(ht, &trc, TRUE);
		aPt.x = trc.left;
		aPt.y = trc.top + (trc.Height() / 2);
	} else {
		aPt.x = aPt.y = 0;
	}
	aTree.ClientToScreen(&aPt);
}
void WinUtil::getContextMenuPos(CFulEditCtrl& aEdit, POINT& aPt) {
	CRect erc;
	aEdit.GetRect(&erc);
	aPt.x = erc.Width() / 2;
	aPt.y = erc.Height() / 2;
	aEdit.ClientToScreen(&aPt);
}

void WinUtil::openFolder(const tstring& file) {
	if (File::getSize(Text::fromT(file)) != -1)
		::ShellExecute(NULL, NULL, Text::toT("explorer.exe").c_str(), Text::toT("/e, /select, \"" + (Text::fromT(file)) + "\"").c_str(), NULL, SW_SHOWNORMAL);
	else
		::ShellExecute(NULL, NULL, Text::toT("explorer.exe").c_str(), Text::toT("/e, \"" + Util::getFilePath(Text::fromT(file)) + "\"").c_str(), NULL, SW_SHOWNORMAL);
}

void WinUtil::SearchSite(WebShortcut* ws, tstring strSearchString) {
	if(ws == NULL)
		return;

	if(ws->clean) {
		PME regexp;

		tstring strSearch = strSearchString;
		tstring strStoplistText = _T("xvid|divx|dvdrip|dvdr|dvd-r|pal|ntsc|screener|dvdscr|complete|proper|.*|.ws.|ac3|internal|directoryfix|pdtv|hdtv|rerip|tvrip|swedish");
		size_t intPos = 0;

		// Convert the stoplist string to a vector
		StringTokenizer<tstring> t(strStoplistText, _T('|') );
		TStringList strStoplist = t.getTokens();
		
		// To lower case
		strSearch = Text::toLower(strSearch);
		// Loop all words and remove those that exists in the search string
		for (unsigned int i = 0; i < strStoplist.size(); i++) {
			int pos = strSearch.find(strStoplist[i]);
			while (pos != tstring::npos ) {
				strSearch = strSearch.substr(0, pos) + strSearch.substr(pos + strStoplist[i].length());
				pos = strSearch.find(strStoplist[i]);
			}
		}
		// Just include the text until the first "-"
		intPos = strSearch.find(_T("-"));
		if (intPos > 0) {
			strSearch = strSearch.substr(0, intPos);
		}
		// Exchange all "." with " "
		intPos = 0;
		while ( (intPos = strSearch.find_first_of(_T("._"), intPos)) != string::npos) {
			strSearch.replace(intPos, 1, _T(" "));
		}
		
		// Remove 4 digits (year)
		regexp.Init(_T("\\d{4}"), _T("i"));
		strSearch = regexp.sub(strSearch, Util::emptyStringT);
				
		// search for "s01e01" and remove
		regexp.Init(_T("s\\d{2}(e\\d{2})?"), _T("i"));
		strSearch = regexp.sub(strSearch, Util::emptyStringT);

		// search for "1x01" and remove
		//regExp.Parse("{ [0-9]x[0-9][0-9]}", false);
		regexp.Init(_T("\\dx\\d{2}"), _T("i"));
		strSearch = regexp.sub(strSearch, Util::emptyStringT);
		
		// Remove trailing spaces
		intPos = strSearch.length() - 1;
		while (intPos > 0) {
			if (strSearch[intPos] != _T(' ')) {
				break;
			}
			intPos--;
		}
		strSearchString = strSearch.substr(0, intPos + 1);
	}
	
	if(Util::stricmp(ws->url.c_str(), _T("%s")) == 0) {
		WinUtil::openLink(strSearchString);
		return;
	}

	if(ws->url.find(_T("google.")) != tstring::npos)
		strSearchString = Text::utf8ToWide(Util::encodeURI(Text::wideToUtf8(strSearchString)));
	else
		strSearchString	= Text::acpToWide(Util::encodeURI(Text::wideToAcp(strSearchString)));

	TCHAR *buf = new TCHAR[ws->url.length() + strSearchString.length()];
	_stprintf(buf, ws->url.c_str(), strSearchString.c_str());

	DWORD escapedSize = 2048;
	TCHAR* escapedBuf = new TCHAR[escapedSize];


	HRESULT res = UrlCanonicalize(buf, escapedBuf, &escapedSize, URL_DONT_SIMPLIFY | URL_ESCAPE_UNSAFE);
	
	if( S_OK == res)
		WinUtil::openLink(escapedBuf);
	
	delete[] buf;
	delete[] escapedBuf;
}

void WinUtil::search(tstring searchTerm, int searchMode, bool tth) {
	if(!searchTerm.empty()) {
		//skapa listan över icke tillåtna karaktärer
		TCHAR chars[33] = {_T('<'), _T('>'), _T(','), _T(';'), _T('.'), _T(':'), _T('-'), _T('_'), _T('!'),
						_T('\"'), _T('@'), _T('#'), _T('£'), _T('$'), _T('%'), _T('&'), _T('/'), _T('{'),
						_T('('), _T('['), _T(')'), _T(']'), _T('='), _T('}'), _T('?'), _T('+'), _T('´'),
						_T('`'), _T('*'), _T('^'), _T('\\'), _T('\r'), _T('\n')};
		
		size_t length = searchTerm.length()-1;
		try{
			//gå igenom hela listan och leta efter tecknen
			for(int i = 0; i < 31; ++i) {
				//om vi hittar ett tecken, ta bort det och börja om från början
				if(searchTerm[0] == chars[i]) {
					searchTerm.erase((tstring::size_type)0, (tstring::size_type)1);
					i = 0;
					--length;
				}
				if(searchTerm[length] == chars[i]) {
					searchTerm.erase(length, length);
					i = 0;
					--length;
				}
			}
		}catch(exception) {
			return;
		}
		if(!searchTerm.empty()) {
			if(0 == searchMode) {
				if(tth)
					SearchFrame::openWindow(searchTerm, 0, SearchManager::SIZE_ATLEAST, SearchManager::TYPE_TTH);
				else
					SearchFrame::openWindow(searchTerm, 0, SearchManager::SIZE_ATLEAST, SearchManager::TYPE_ANY);
			}else{
				--searchMode;
				if(searchMode < (int)WebShortcuts::getInstance()->list.size()) {
					WebShortcut *ws = WebShortcuts::getInstance()->list[searchMode];
					if(ws != NULL)
						WinUtil::SearchSite(ws, searchTerm);
				}
			}
		}
	}
}

void WinUtil::AppendSearchMenu(CMenu& menu) {
	while(menu.GetMenuItemCount() > 0) {
		menu.RemoveMenu(0, MF_BYPOSITION);
	}

	WebShortcut::Iter i = WebShortcuts::getInstance()->list.begin();
	for(; i != WebShortcuts::getInstance()->list.end(); ++i) {
		menu.AppendMenu(MF_STRING, (UINT_PTR)0, (LPCTSTR)(*i)->name.c_str());
	}
}

void WinUtil::SetIcon(HWND hWnd, tstring file, bool big) {
	tstring path = _T("icons\\") + file;
	HICON hIconSm = (HICON)::LoadImage(NULL, path.c_str(), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_LOADFROMFILE | LR_SHARED);
	::SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSm);

	if(big){
		HICON hIcon   = (HICON)::LoadImage(NULL, path.c_str(), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR | LR_LOADFROMFILE | LR_SHARED);
		::SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	}
	
}

tstring WinUtil::encodeFont(LOGFONT const& font){
	tstring res(font.lfFaceName);
	res += L',';
	res += Text::utf8ToWide(Util::toString(font.lfHeight));
	res += L',';
	res += Text::utf8ToWide(Util::toString(font.lfWeight));
	res += L',';
	res += Text::utf8ToWide(Util::toString(font.lfItalic));
	return res;
}

void WinUtil::addLastDir(const tstring& dir) {
	if(find(lastDirs.begin(), lastDirs.end(), dir) != lastDirs.end()) {
		return;
	}
	if(lastDirs.size() == 10) {
		lastDirs.erase(lastDirs.begin());
	}
	lastDirs.push_back(dir);
}

int WinUtil::getTextWidth(const tstring& str, HWND hWnd) {
	HDC dc = ::GetDC(hWnd);
	int sz = getTextWidth(str, dc);
	::ReleaseDC(hWnd, dc);
	return sz;
}
int WinUtil::getTextWidth(const tstring& str, HDC dc) {
	SIZE sz = { 0, 0 };
	::GetTextExtentPoint32(dc, str.c_str(), str.length(), &sz);
	return sz.cx;		
}

int WinUtil::getTextWidth(HWND wnd, HFONT fnt) {
	HDC dc = ::GetDC(wnd);
	HGDIOBJ old = ::SelectObject(dc, fnt);
	TEXTMETRIC tm;
	::GetTextMetrics(dc, &tm);
	::SelectObject(dc, old);
	::ReleaseDC(wnd, dc);
	return tm.tmAveCharWidth;
}

int WinUtil::getTextHeight(HWND wnd, HFONT fnt) {
	HDC dc = ::GetDC(wnd);
	int h = getTextHeight(dc, fnt);
	::ReleaseDC(wnd, dc);
	return h;
}

int WinUtil::getTextHeight(HDC dc, HFONT fnt) {
	HGDIOBJ old = ::SelectObject(dc, fnt);
	int h = getTextHeight(dc);
	::SelectObject(dc, old);
	return h;
}

int WinUtil::getTextHeight(HDC dc) {
	TEXTMETRIC tm;
	::GetTextMetrics(dc, &tm);
	return tm.tmHeight;
}

int WinUtil::getTextSpacing(HWND wnd, HFONT fnt) {
	HDC dc = ::GetDC(wnd);
	HGDIOBJ old = ::SelectObject(dc, fnt);
	TEXTMETRIC tm;
	::GetTextMetrics(dc, &tm);
	::SelectObject(dc, old);
	::ReleaseDC(wnd, dc);
	return tm.tmInternalLeading;
}

tstring WinUtil::DiskSpaceInfo(bool onlyTotal /* = false */) {
	ULONG drives = _getdrives();
	TCHAR drive[3] = { _T('C'), _T(':'), _T('\0') };
	tstring ret = Util::emptyStringT;
	int64_t free = 0, totalFree = 0, size = 0, totalSize = 0;

	drives = ( drives >> 2);
	
	while(drives != 0) {
		if(drives & 1 && GetDriveType(drive) != DRIVE_CDROM){
			if(GetDiskFreeSpaceEx(drive, NULL, (PULARGE_INTEGER)&size, (PULARGE_INTEGER)&free)){
				totalFree += free;
				totalSize += size;
			}
		}

		++drive[0];
		drives = (drives >> 1);
	}

	if(totalSize != 0)
		if( !onlyTotal )
			ret += _T("HDD space (free/total): ") + Util::formatBytesW(totalFree) + _T("/") + Util::formatBytesW(totalSize);
		else
			ret += Util::formatBytesW(totalFree) + _T("/") + Util::formatBytesW(totalSize);

	return ret;
}

tstring WinUtil::Help(const tstring& cmd) {
	string command = Text::fromT(cmd);
	string xmlString;
	const size_t BUF_SIZE = 64*1024;
	char *buf = new char[BUF_SIZE];
	u_int32_t pos = 0;
	SimpleXML xml;

	//try to read the xml from the file
	try{
		::File f(Util::getAppPath() + "Help.xml", File::READ, File::OPEN);
		for(;;) {
			size_t tmp = BUF_SIZE;
			pos = f.read(buf, tmp);
			xmlString.append(buf, pos);
			if(pos < BUF_SIZE)
				break;
		}
		f.close();
		delete[] buf;
	}catch (Exception& e) { 
		delete[] buf;
		return Text::toT(e.getError());
	}

	tstring ret = Util::emptyStringT;
	try{
		xml.fromXML(xmlString);

		xml.stepIn();

		bool found = false;
		if(Text::toLower(command).find("dc++") != string::npos)
			found = xml.findChild("dc");
		else
			found = xml.findChild(Text::toLower(command));

		if(found)
			ret = Text::toT(xml.getChildData());
	} catch(const SimpleXMLException &e) {
		return Text::toT(e.getError());
	}

	return ret;
}

tstring WinUtil::Uptime() {
	HQUERY    hQuery	= NULL;
	HCOUNTER  hCounter	= NULL;
	tstring ret = Util::emptyStringT;
		
	if ( PdhOpenQuery( NULL, 0, &hQuery ) == ERROR_SUCCESS ) {
		//create some variables to fetch and store the counter path	
		TCHAR path[MAX_COUNTER_PATH];
		TCHAR tmp[MAX_COUNTER_PATH];
		DWORD maxPath = MAX_COUNTER_PATH;

		//fetch the first part of the counter path, the object name.
		if(PdhLookupPerfNameByIndex(NULL, 2, tmp, &maxPath) == ERROR_SUCCESS){
			//store it so that we can reuse the tmp variable.
			_tcscpy(path, _T("\\"));
			_tcscat(path, tmp);
			//restore maxPath to it's max length
			maxPath = MAX_COUNTER_PATH;

			//get the counter part of the path.
			if(PdhLookupPerfNameByIndex(NULL, 674, tmp, &maxPath) == ERROR_SUCCESS){
				_tcscat(path, _T("\\"));
				_tcscat(path, tmp);

				if(PdhAddCounter( hQuery, path, 0, &hCounter ) == ERROR_SUCCESS) {
					PDH_FMT_COUNTERVALUE  pdhCounterValue;

					if ( PdhCollectQueryData( hQuery ) == ERROR_SUCCESS )	{
						if ( PdhGetFormattedCounterValue( hCounter, PDH_FMT_LARGE, NULL, &pdhCounterValue ) == ERROR_SUCCESS )
							ret = Text::toT(Util::formatTime(pdhCounterValue.largeValue, false));
					}
				}
			}
		}
	}
	
	if ( hCounter )   
		PdhRemoveCounter( hCounter );
	if ( hQuery )
		PdhCloseQuery   ( hQuery );

	return ret;
}

tstring WinUtil::UselessInfo() {
	tstring result = _T("\n");
	TCHAR buf[255];
	
	MEMORYSTATUSEX mem;
	mem.dwLength = sizeof(MEMORYSTATUSEX);
	if( GlobalMemoryStatusEx(&mem) != 0){
		result += _T("Memory\n");
		result += _T("Physical memory (available/total): ");
		result += Util::formatBytesW( mem.ullAvailPhys ) + _T("/") + Util::formatBytesW( mem.ullTotalPhys );
		result += _T("\n");
		result += _T("Pagefile (available/total): ");
		result += Util::formatBytesW( mem.ullAvailPageFile ) + _T("/") + Util::formatBytesW( mem.ullTotalPageFile );
		result += _T("\n");
		result += _T("Virtual memory (available/total): ");
		result += Util::formatBytesW( mem.ullAvailVirtual ) + _T("/") + Util::formatBytesW( mem.ullTotalVirtual );
		result += _T("\n");
		result += _T("Memory load: ");
		result += Util::toStringW(mem.dwMemoryLoad);
		result += _T("%\n\n");
	}

	CRegKey key;
	ULONG len = 255;

	if(key.Open( HKEY_LOCAL_MACHINE, _T("Hardware\\Description\\System\\CentralProcessor\\0"), KEY_READ) == ERROR_SUCCESS) {
		result += _T("CPU\n");
        if(key.QueryStringValue(_T("ProcessorNameString"), buf, &len) == ERROR_SUCCESS){
			result += _T("Name: ");
			tstring tmp = buf;
            result += tmp.substr( tmp.find_first_not_of(_T(" ")) );
			result += _T("\n");
		}
		DWORD speed;
		if(key.QueryDWORDValue(_T("~MHz"), speed) == ERROR_SUCCESS){
			result += _T("Speed: ");
			result += Util::toStringW(speed);
			result += _T("MHz\n");
		}
		len = 255;
		if(key.QueryStringValue(_T("Identifier"), buf, &len) == ERROR_SUCCESS) {
			result += _T("Identifier: ");
			result += buf;
		}
		result += _T("\n\n");
	}

	OSVERSIONINFOEX ver;
	if(WinUtil::getVersionInfo(ver) ) {
		tstring platform;
		if(ver.dwPlatformId == VER_PLATFORM_WIN32_NT)
			platform = _T("Win32 NT");
		else if( ver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
			platform = _T("Win32 Windows");
		else
			platform = _T("Win32");
		
		tstring type;
		if( ver.wProductType == VER_NT_WORKSTATION )
			type = _T("Workstation");
		else if( ver.wProductType == VER_NT_DOMAIN_CONTROLLER )
			type = _T("Domain Controller");
		else
			type = _T("Server");

		_stprintf(buf, _T("%d.%d.%d SP: %d\nPlatform: %s Type: %s"),
			(DWORD)ver.dwMajorVersion, (DWORD)ver.dwMinorVersion, (DWORD)ver.dwBuildNumber,
			(DWORD)ver.wServicePackMajor, platform.c_str(), type.c_str());

		result += _T("OS\n");
		result += buf;
		result += _T("\n\n");
	}

	result += _T("Uptime\n");
	result += _T("System uptime: ");
	result += WinUtil::Uptime();
	result += _T('\n');
	result += TSTRING(FULDC_UPTIME) + _T(" ") + Util::formatTimeW(GET_TIME() - WinUtil::startTime, false);
	result += _T("\n\nDisk\n");
	result += _T("Disk space(free/total): ");
	result += DiskSpaceInfo(true);
	result += _T("\n\nTransfer\n");
	result += Text::toT("Upload: " + Util::formatBytes(SETTING(TOTAL_UPLOAD)) + ", Download: " + Util::formatBytes(SETTING(TOTAL_DOWNLOAD)));
	
	if(SETTING(TOTAL_DOWNLOAD) > 0) {
		_stprintf(buf, _T("Ratio (up/down): %.2f"), ((double)SETTING(TOTAL_UPLOAD)) / ((double)SETTING(TOTAL_DOWNLOAD)));
		result += _T('\n');
		result += buf;
	}

	return result;
}
//returns 1 if it's a supposed to activate the switch
//returns 0 if it's supposed to disable the switch
//returns -1 if it's not a correct param
int WinUtil::checkParam( const tstring& param ){
	if( Util::stricmp(param, _T("on")) == 0 ||
		Util::stricmp(param, _T("1")) == 0 ||
		Util::stricmp(param, _T("true")) == 0 ||
		Util::stricmp(param, _T("yes")) == 0 ) {
			return 1;
	}

	if( Util::stricmp(param, _T("off")) == 0 ||
		Util::stricmp(param, _T("0")) == 0 ||
		Util::stricmp(param, _T("false")) == 0 ||
		Util::stricmp(param, _T("no")) == 0 ) {
			return 0;
	}

	return -1;
}

bool WinUtil::flashWindow() {
	if( GetForegroundWindow() != WinUtil::mainWnd ) {
		DWORD flashCount;
		SystemParametersInfo(SPI_GETFOREGROUNDFLASHCOUNT, 0, &flashCount, 0);
		FLASHWINFO flash;
		flash.cbSize = sizeof(FLASHWINFO);
		flash.dwFlags = FLASHW_ALL;
		flash.uCount = flashCount;
		flash.hwnd = WinUtil::mainWnd;
		flash.dwTimeout = 0;

		FlashWindowEx(&flash);

		return true;
	}

	return false;
}

/**
 * @file
 * $Id: WinUtil.cpp,v 1.21 2004/02/21 15:43:54 trem Exp $
 */
