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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "WinUtil.h"
#include "PrivateFrame.h"
#include "SearchFrm.h"
#include "LineDlg.h"

#include "../client/Util.h"
#include "../client/StringTokenizer.h"
#include "../client/ShareManager.h"
#include "../client/ClientManager.h"
#include "../client/TimerManager.h"
#include "../client/HubManager.h"
#include "../client/ResourceManager.h"
#include "../client/QueueManager.h"
#include "../client/UploadManager.h"
#include "../client/WebShortcuts.h"
#include "../client/HashManager.h"
#include "../client/SimpleXML.h"

#include <direct.h>
#include <pdh.h>

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
StringList WinUtil::lastDirs;
HWND WinUtil::mainWnd = NULL;
HWND WinUtil::mdiClient = NULL;
FlatTabCtrl* WinUtil::tabCtrl = NULL;
HHOOK WinUtil::hook = NULL;
HWND WinUtil::findDialog = NULL;
const u_int32_t WinUtil::startTime = GET_TIME();


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

	file.AppendMenu(MF_STRING, IDC_OPEN_FILE_LIST, CSTRING(MENU_OPEN_FILE_LIST));
	file.AppendMenu(MF_STRING, IDC_OPEN_MY_LIST, CSTRING(MENU_OPEN_MY_LIST));
	file.AppendMenu(MF_STRING, IDC_REFRESH_FILE_LIST, CSTRING(MENU_REFRESH_FILE_LIST));
	file.AppendMenu(MF_STRING, IDC_OPEN_DOWNLOADS, CSTRING(MENU_OPEN_DOWNLOADS_DIR));
	file.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	file.AppendMenu(MF_STRING, IDC_FOLLOW, CSTRING(MENU_FOLLOW_REDIRECT));
	file.AppendMenu(MF_STRING, ID_FILE_RECONNECT, CSTRING(MENU_RECONNECT));
	file.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	file.AppendMenu(MF_STRING, IDC_IMPORT_QUEUE, CSTRING(MENU_IMPORT_QUEUE));
	file.AppendMenu(MF_STRING, ID_FILE_SETTINGS, CSTRING(MENU_SETTINGS));
	file.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	file.AppendMenu(MF_STRING, ID_APP_EXIT, CSTRING(MENU_EXIT));

	mainMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)file, CSTRING(MENU_FILE));

	CMenuHandle view;
	view.CreatePopupMenu();

	view.AppendMenu(MF_STRING, ID_FILE_CONNECT, CSTRING(MENU_PUBLIC_HUBS));
	view.AppendMenu(MF_STRING, IDC_QUEUE, CSTRING(MENU_DOWNLOAD_QUEUE));
	view.AppendMenu(MF_STRING, IDC_FINISHED, CSTRING(FINISHED_DOWNLOADS));
	view.AppendMenu(MF_STRING, IDC_FINISHED_UL, CSTRING(FINISHED_UPLOADS));
	view.AppendMenu(MF_STRING, IDC_FAVORITES, CSTRING(MENU_FAVORITE_HUBS));
	view.AppendMenu(MF_STRING, IDC_FAVUSERS, CSTRING(MENU_FAVORITE_USERS));
	view.AppendMenu(MF_STRING, ID_FILE_SEARCH, CSTRING(MENU_SEARCH));
	view.AppendMenu(MF_STRING, IDC_FILE_ADL_SEARCH, CSTRING(MENU_ADL_SEARCH));
	view.AppendMenu(MF_STRING, IDC_SEARCH_SPY, CSTRING(MENU_SEARCH_SPY));
	view.AppendMenu(MF_STRING, IDC_NET_STATS, CSTRING(MENU_NETWORK_STATISTICS));
	view.AppendMenu(MF_STRING, IDC_NOTEPAD, CSTRING(MENU_NOTEPAD));
	view.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	view.AppendMenu(MF_STRING, ID_VIEW_TOOLBAR, CSTRING(MENU_TOOLBAR));
	view.AppendMenu(MF_STRING, ID_VIEW_STATUS_BAR, CSTRING(MENU_STATUS_BAR));
	view.AppendMenu(MF_STRING, ID_VIEW_TRANSFER_VIEW, CSTRING(MENU_TRANSFER_VIEW));

	mainMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)view, CSTRING(MENU_VIEW));

	CMenuHandle window;
	window.CreatePopupMenu();

	window.AppendMenu(MF_STRING, ID_WINDOW_CASCADE, CSTRING(MENU_CASCADE));
	window.AppendMenu(MF_STRING, ID_WINDOW_TILE_HORZ, CSTRING(MENU_HORIZONTAL_TILE));
	window.AppendMenu(MF_STRING, ID_WINDOW_TILE_VERT, CSTRING(MENU_VERTICAL_TILE));
	window.AppendMenu(MF_STRING, ID_WINDOW_ARRANGE, CSTRING(MENU_ARRANGE));
	window.AppendMenu(MF_STRING, ID_WINDOW_MINIMIZE_ALL, CSTRING(MENU_MINIMIZE_ALL));
	window.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	window.AppendMenu(MF_STRING, IDC_CLOSE_DISCONNECTED, CSTRING(MENU_CLOSE_DISCONNECTED));
	window.AppendMenu(MF_STRING, IDC_CLOSE_ALL_PM, CSTRING(MENU_CLOSE_ALL_PM));
	window.AppendMenu(MF_STRING, IDC_CLOSE_ALL_DIR_LIST, CSTRING(MENU_CLOSE_ALL_DIR_LIST));
	
	mainMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)window, CSTRING(MENU_WINDOW));

	CMenuHandle help;
	help.CreatePopupMenu();

	help.AppendMenu(MF_STRING, IDC_HELP_README, CSTRING(MENU_README));
	help.AppendMenu(MF_STRING, IDC_HELP_CHANGELOG, CSTRING(MENU_CHANGELOG));
	help.AppendMenu(MF_STRING, ID_APP_ABOUT, CSTRING(MENU_ABOUT));
	help.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	help.AppendMenu(MF_STRING, IDC_HELP_FULPAGE, CSTRING(MENU_FULPAGE));
	help.AppendMenu(MF_STRING, IDC_HELP_HOMEPAGE, CSTRING(MENU_HOMEPAGE));
	help.AppendMenu(MF_STRING, IDC_HELP_DOWNLOADS, CSTRING(MENU_DOWNLOADS));
	help.AppendMenu(MF_STRING, IDC_HELP_FAQ, CSTRING(MENU_FAQ));
	help.AppendMenu(MF_STRING, IDC_HELP_HELP_FORUM, CSTRING(MENU_HELP_FORUM));
	help.AppendMenu(MF_STRING, IDC_HELP_DISCUSS, CSTRING(MENU_DISCUSS));
	help.AppendMenu(MF_STRING, IDC_HELP_REQUEST_FEATURE, CSTRING(MENU_REQUEST_FEATURE));
	help.AppendMenu(MF_STRING, IDC_HELP_REPORT_BUG, CSTRING(MENU_REPORT_BUG));
	help.AppendMenu(MF_STRING, IDC_HELP_DONATE, CSTRING(MENU_DONATE));

	mainMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)help, CSTRING(MENU_HELP));

	if(BOOLSETTING(USE_SYSTEM_ICONS)) {
		SHFILEINFO fi;
		fileImages.Create(16, 16, ILC_COLOR32 | ILC_MASK, 16, 16);
		::SHGetFileInfo(".", FILE_ATTRIBUTE_DIRECTORY, &fi, sizeof(fi), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
		fileImages.AddIcon(fi.hIcon);
		::DestroyIcon(fi.hIcon);
		dirIconIndex = fileImageCount++;	
	} else {
		fileImages.CreateFromImage("icons\\folders.bmp", 16, 3, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED | LR_LOADFROMFILE);
		dirIconIndex = 0;
	}
	userImages.CreateFromImage("icons\\users.bmp", 16, 8, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED | LR_LOADFROMFILE);

	LOGFONT lf, lf2;
	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);
	SettingsManager::getInstance()->setDefault(SettingsManager::TEXT_FONT, encodeFont(lf));
	decodeFont(SETTING(TEXT_FONT), lf);
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

	hook = SetWindowsHookEx(WH_KEYBOARD, &KeyboardProc, NULL, GetCurrentThreadId());
}

void WinUtil::uninit() {
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

void WinUtil::decodeFont(const string& setting, LOGFONT &dest) {
	StringTokenizer st(setting, ',');
	StringList &sl = st.getTokens();
	
	::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(dest), &dest);
	string face;
	if(sl.size() == 4)
	{
		face = sl[0];
		dest.lfHeight = Util::toInt(sl[1]);
		dest.lfWeight = Util::toInt(sl[2]);
		dest.lfItalic = (BYTE)Util::toInt(sl[3]);
	}
	
	if(!face.empty()) {
		::ZeroMemory(dest.lfFaceName, LF_FACESIZE);
		strcpy(dest.lfFaceName, face.c_str());
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

bool WinUtil::browseDirectory(string& target, HWND owner /* = NULL */) {
	char buf[MAX_PATH];
	BROWSEINFO bi;
	LPMALLOC ma;
	
	ZeroMemory(&bi, sizeof(bi));
	
	bi.hwndOwner = owner;
	bi.pszDisplayName = buf;
	bi.lpszTitle = CSTRING(CHOOSE_FOLDER);
	bi.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lParam = (LPARAM)target.c_str();
	bi.lpfn = &browseCallbackProc;
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if(pidl != NULL) {
		SHGetPathFromIDList(pidl, buf);
		target = buf;
		
		if(target.size() > 0 && target[target.size()-1] != '\\')
			target+='\\';
		
		if(SHGetMalloc(&ma) != E_FAIL) {
			ma->Free(pidl);
			ma->Release();
		}
		return true;
	}
	return false;
}

bool WinUtil::browseFile(string& target, HWND owner /* = NULL */, bool save /* = true */, const string& initialDir /* = Util::emptyString */, const char* types /* = NULL */, const char* defExt /* = NULL */) {
	char buf[MAX_PATH];
	OPENFILENAME ofn;       // common dialog box structure
	target = Util::validateFileName(target);
	memcpy(buf, target.c_str(), target.length() + 1);
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
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


void WinUtil::splitTokens(int* array, const string& tokens, int maxItems /* = -1 */) throw() {
	StringTokenizer t(tokens, ',');
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
			dlg.title = uc.getName();
			dlg.description = name;
			dlg.line = sm["line:" + name];
			if(dlg.DoModal(parent) == IDOK) {
				sm["line:" + name] = dlg.line;
				done[name] = dlg.line;
			} else {
				return false;
			}
		}
		i = j + 1;
	}
	return true;
}

#define LINE2 "-- http://dcplusplus.sourceforge.net  <DC++ " VERSIONSTRING ">"
char *msgs[] = { "\r\n-- I'm a happy dc++ user. You could be happy too.\r\n" LINE2,
"\r\n-- Neo-...what? Nope...never heard of it...\r\n" LINE2,
"\r\n-- Evolution of species: Ape --> Man\r\n-- Evolution of science: \"The Earth is Flat\" --> \"The Earth is Round\"\r\n-- Evolution of sharing: NMDC --> DC++\r\n" LINE2,
"\r\n-- I share, therefore I am.\r\n" LINE2,
"\r\n-- I came, I searched, I found...\r\n" LINE2,
"\r\n-- I came, I shared, I sent...\r\n" LINE2,
"\r\n-- I can set away mode, can't you?\r\n" LINE2,
"\r\n-- I don't have to see any ads, do you?\r\n" LINE2,
"\r\n-- I don't have to see those annoying kick messages, do you?\r\n" LINE2,
"\r\n-- I can resume my files to a different filename, can you?\r\n" LINE2,
"\r\n-- I can share huge amounts of files, can you?\r\n" LINE2,
"\r\n-- My client doesn't spam the chat with useless debug messages, does yours?\r\n" LINE2,
"\r\n-- I can add multiple users to the same download and have the client connect to another automatically when one goes offline, can you?\r\n" LINE2,
"\r\n-- These addies are pretty annoying, aren't they? Get revenge by sending them yourself!\r\n" LINE2,
"\r\n-- My client supports TTH hashes, does yours?\r\n" LINE2,
"\r\n-- My client supports XML file lists, does yours?\r\n" LINE2
};

#define MSGS 16

string WinUtil::commands = "/refresh, /slots #, /search <string>, /dc++, /away <msg>, /back, /fuldc, /share <dir>, /unshare <dir>, /lastseen <nick>, /lastlog #, /rebuild";

bool WinUtil::checkCommand(string& cmd, string& param, string& message, string& status) {
	string::size_type i = cmd.find(' ');
	if(i != string::npos) {
		param = cmd.substr(i+1);
		cmd = cmd.substr(1, i - 1);
	} else {
		cmd = cmd.substr(1);
	}

	if(Util::stricmp(cmd.c_str(), "refresh")==0) {
		try {
			ShareManager::getInstance()->setDirty();
			if(!param.empty()) {
				if(!ShareManager::getInstance()->refresh(param))
					status = STRING(DIRECTORY_NOT_FOUND);
			} else {
				ShareManager::getInstance()->refresh(true);
			}
		} catch(const ShareException& e) {
			status = e.getError();
		}
	} else if(Util::stricmp(cmd.c_str(), "refreshi") == 0) {
		try {
			ShareManager::getInstance()->setDirty();
			ShareManager::getInstance()->refresh(false, true, false, true);
		} catch( const ShareException& e) {
			status = e.getError();
		}
	} else if(Util::stricmp(cmd.c_str(), "share") == 0){
		if(!param.empty()){
			ShareManager::getInstance()->addDirectory(param);
			status = STRING(ADDED) + " " + param;
		}
	} else if(Util::stricmp(cmd.c_str(), "unshare") == 0) {
		if(!param.empty()){
			ShareManager::getInstance()->removeDirectory(param);
			status = STRING(REMOVED) + " " + param;
		}
	} else if(Util::stricmp(cmd.c_str(), "slots")==0) {
		if(param.empty()) {
			int slots = SettingsManager::getInstance()->get(SettingsManager::SLOTS);
			status = "Current number of slots: " + Util::toString(slots);
		}else {
			int j = Util::toInt(param);
			if(j > 0) {
				SettingsManager::getInstance()->set(SettingsManager::SLOTS, j);
				status = STRING(SLOTS_SET);
				ClientManager::getInstance()->infoUpdated();
			} else {
				status = STRING(INVALID_NUMBER_OF_SLOTS);
			}
		}
	} else if(Util::stricmp(cmd.c_str(), "search") == 0) {
		if(!param.empty()) {
			SearchFrame::openWindow(param);
		} else {
			status = STRING(SPECIFY_SEARCH_STRING);
		}
	} else if(Util::stricmp(cmd.c_str(), "dc++") == 0) {
		message = msgs[GET_TICK() % MSGS];
	} else if(Util::stricmp(cmd.c_str(), "away") == 0) {
		if(Util::getAway() && param.empty()) {
			Util::setAway(false);
			status = STRING(AWAY_MODE_OFF);
		} else {
			Util::setAway(true);
			Util::setAwayMessage(param);
			status = STRING(AWAY_MODE_ON) + Util::getAwayMessage();
		}
	} else if(Util::stricmp(cmd.c_str(), "back") == 0) {
		Util::setAway(false);
		status = STRING(AWAY_MODE_OFF);
	} else if(Util::stricmp(cmd.c_str(), "dslots") == 0) {
		if(param.empty()) {
			int slots = SettingsManager::getInstance()->get(SettingsManager::DOWNLOAD_SLOTS);
			status = "Current number of download slots: " + Util::toString(slots);
		} else {
			int nr = Util::toInt(param);
			if( nr >= 0 ){
				SettingsManager::getInstance()->set(SettingsManager::DOWNLOAD_SLOTS, nr);
				status = "Download slots set";
			} else {
				status = "Invalid number of slots";
			}
		} 
	}else if(Util::stricmp(cmd.c_str(), "fuldc") == 0) {
		message = "http://paxi.myftp.org <fulDC " + string(FULVERSIONSTRING) + ">";
	} else if(Util::stricmp(cmd.c_str(), "fuptime") == 0) {
		message = "fulDC uptime: " + Util::formatTime(GET_TIME() - WinUtil::startTime, false);
	} else if(Util::stricmp(cmd.c_str(), "uptime") == 0) {
		message = "System uptime: " + WinUtil::Uptime();
	} else if(WebShortcuts::getInstance()->getShortcutByKey(cmd) != NULL) {
		WinUtil::SearchSite(WebShortcuts::getInstance()->getShortcutByKey(cmd), param);
	} else if(Util::stricmp(cmd.c_str(), "rebuild") == 0) {
		HashManager::getInstance()->rebuild();
		status = STRING(HASH_REBUILT);
	} else {
		return false;
	}

	return true;
}

void WinUtil::openLink(const string& url) {
	CRegKey key;
	char regbuf[MAX_PATH];
	ULONG len = MAX_PATH;
	if(key.Open(HKEY_CLASSES_ROOT, "http\\shell\\open\\command", KEY_READ) == ERROR_SUCCESS) {
		if(key.QueryStringValue(NULL, regbuf, &len) == ERROR_SUCCESS) {
			/*
			 * Various values:
			 *  C:\PROGRA~1\MOZILL~1\FIREFOX.EXE -url "%1"
			 *  "C:\Program Files\Internet Explorer\iexplore.exe" -nohome
			 *  "C:\Apps\Opera7\opera.exe"
			 *  C:\PROGRAMY\MOZILLA\MOZILLA.EXE -url "%1"
			 */
			string cmd(regbuf); // otherwise you consistently get two trailing nulls
			
			if(!cmd.empty() && cmd.length() > 1) {
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

				string cmdLine(cmd);
				cmd = cmd.substr(start, end-start);
				size_t arg_pos;
				if((arg_pos = cmdLine.find("%1")) != string::npos) {
					cmdLine.replace(arg_pos, 2, url);
				} else {
					cmdLine.append(" \"" + url + '\"');
				}

				STARTUPINFO si = { sizeof(si), 0 };
				PROCESS_INFORMATION pi = { 0 };
				AutoArray<char> buf(cmdLine.length() + 1);
				strcpy(buf, cmdLine.c_str());
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

int WinUtil::getIconIndex(const string& aFileName) {
	if(BOOLSETTING(USE_SYSTEM_ICONS)) {
		SHFILEINFO fi;
		string x = Util::getFileName(aFileName);
		string::size_type i = x.rfind('.');
		if(i != string::npos) {
			x = x.substr(i);
			ImageIter j = fileIndexes.find(x);
			if(j != fileIndexes.end())
				return j->second;
		}

		::SHGetFileInfo(x.c_str(), FILE_ATTRIBUTE_NORMAL, &fi, sizeof(fi), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
		fileImages.AddIcon(fi.hIcon);
		::DestroyIcon(fi.hIcon);

		fileIndexes[x] = fileImageCount++;
		return fileImageCount - 1;
	} else {
		return 2;
	}
}

void WinUtil::copyToClipboard(const string &aStr) {
	if(!OpenClipboard(0)) {
		return;
	}

	EmptyClipboard();
			
	// Allocate a global memory object for the text. 
	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (aStr.size() + 1)); 
	if (hglbCopy == NULL) { 
		CloseClipboard(); 
		return; 
	} 
			
	// Lock the handle and copy the text to the buffer. 
	char* lptstrCopy = (char*)GlobalLock(hglbCopy); 
	memcpy(lptstrCopy, aStr.c_str(), aStr.length() + 1);
	GlobalUnlock(hglbCopy); 
			
	// Place the handle on the clipboard. 
	SetClipboardData(CF_TEXT, hglbCopy); 
	CloseClipboard();
}

void WinUtil::SearchSite(WebShortcut* ws, string strSearchString) {
	if(ws == NULL)
		return;

	regex::rpattern regexp;
	regex::match_results result;
	regex::rpattern::backref_type br;

	string strSearch = strSearchString;
	string strStoplistText = "xvid|divx|dvdrip|dvdr|dvd-r|pal|ntsc|screener|dvdscr|complete|proper|.*|.ws.|ac3|internal|directoryfix|pdtv|hdtv|rerip|tvrip|swedish";
	int intPos = 0;

	// Convert the stoplist string to a vector
	StringTokenizer t(strStoplistText, '|');
	StringList strStoplist = t.getTokens();
	
	// To lower case
	strSearch = Util::toLower(strSearch);
	// Loop all words and remove those that exists in the search string
	for (unsigned int i = 0; i < strStoplist.size(); i++) {
		int intPos = strSearch.find(strStoplist[i]);
		while (intPos > 0) {
			strSearch = strSearch.substr(0, intPos) + strSearch.substr(intPos + strStoplist[i].length());
			intPos = strSearch.find(strStoplist[i]);
		}
	}
	// Just include the text until the first "-"
	intPos = strSearch.find("-");
	if (intPos > 0) {
		strSearch = strSearch.substr(0, intPos);
	}
	// Exchange all "." with " "
	intPos = 0;
	while ( (intPos = strSearch.find_first_of("._", intPos)) != string::npos) {
		strSearch.replace(intPos, 1, " ");
	}
	// Exchange all "_" with " "
	//intPos = strSearch.find("_");
	//while (intPos > 0) {
	//	strSearch.replace(intPos, 1, " ");
	//	intPos = strSearch.find("_");
	//}
	// Remove 4 digits (year)
	regexp.init("\\d{4}");
	br = regexp.match(strSearch, result);
	if(br.matched) {
		strSearch.replace(result.rstart(), result.rlength(), "");
	}
	// search for "s01e01" and remove
	regexp.init("s\\d{2}(e\\d{2})?", regex::NOCASE);
	br = regexp.match(strSearch, result);
	if (br.matched) {
		strSearch.replace(result.rstart(), result.rlength(), "");
	}
	// search for "1x01" and remove
	//regExp.Parse("{ [0-9]x[0-9][0-9]}", false);
	regexp.init("\\dx\\d{2}", regex::NOCASE);
	br = regexp.match(strSearch, result);
	if (br.matched) {
		strSearch.replace(result.rstart(), result.rlength(), "");
	}
	// Remove trailing spaces
	intPos = strSearch.length() - 1;
	while (intPos > 0) {
		if (strSearch[intPos] != ' ') {
			break;
		}
		intPos--;
	}
	strSearch = strSearch.substr(0, intPos + 1);
	// Do the search
	char *buf = new char[ws->url.length() + strSearch.length()];
	
	if(ws->url.compare("%s") != 0)
		sprintf(buf, ws->url.c_str(), strSearch.c_str());
	else
		sprintf(buf, ws->url.c_str(), strSearchString.c_str());
	WinUtil::openLink(buf);
}

void WinUtil::search(string searchTerm, int searchMode) {
	if(!searchTerm.empty()) {
		//skapa listan över icke tillåtna karaktärer
		char chars[33] = {'<', '>', ',', ';', '.', ':', '-', '_', '!', '\"', '@', '#', '£',
						'$', '%', '&', '/', '{', '(', '[', ')', ']', '=', '}', '?', '+', '´',
						'`', '*', '^', '\\', '\r', '\n'};
		
		int length = searchTerm.length()-1;
		try{
			//gå igenom hela listan och leta efter tecknen
			for(int i = 0; i < 31; ++i) {
				//om vi hittar ett tecken, ta bort det och börja om från början
				if(searchTerm[0] == chars[i]) {
					searchTerm.erase((string::size_type)0, (string::size_type)1);
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
				SearchFrame* pChild = new SearchFrame();
				pChild->setInitial(searchTerm, 0, SearchManager::SIZE_ATLEAST, SearchManager::TYPE_ANY);
				pChild->CreateEx(WinUtil::mdiClient);
				searchTerm = string();
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
	for(int j = 1; i != WebShortcuts::getInstance()->list.end(); ++i, ++j) {
		menu.AppendMenu(MF_STRING, IDC_SEARCH+j, (*i)->name.c_str());
	}
}

void WinUtil::SetIcon(HWND hWnd, string file, bool big) {
	string path = "icons\\" + file;
	HICON hIconSm = (HICON)::LoadImage(NULL, path.c_str(), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_LOADFROMFILE);
	::SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSm);

	if(big){
		HICON hIcon   = (HICON)::LoadImage(NULL, path.c_str(), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR | LR_LOADFROMFILE);
		::SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	}
	
}

string WinUtil::encodeFont(LOGFONT const& font){
	string res(font.lfFaceName);
	res += ',';
	res += Util::toString(font.lfHeight);
	res += ',';
	res += Util::toString(font.lfWeight);
	res += ',';
	res += Util::toString(font.lfItalic);
	return res;
}

void WinUtil::addLastDir(const string& dir) {
	if(find(lastDirs.begin(), lastDirs.end(), dir) != lastDirs.end()) {
		return;
	}
	if(lastDirs.size() == 10) {
		lastDirs.erase(lastDirs.begin());
	}
	lastDirs.push_back(dir);
}

int WinUtil::getTextWidth(const string& str, HWND hWnd) {
	HDC dc = ::GetDC(hWnd);
	int sz = getTextWidth(str, dc);
	::ReleaseDC(mainWnd, dc);
	return sz;
}
int WinUtil::getTextWidth(const string& str, HDC dc) {
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

string WinUtil::DiskSpaceInfo() {
	ULONG drives = _getdrives();
	char drive[3] = { 'C', ':', '\0' };
	string ret = Util::emptyString;
	int64_t free = 0, totalFree = 0, size = 0, totalSize = 0;

	drives = ( drives >> 2);
	
	while(drives != 0) {
		if(drives & 1 && GetDriveType(drive) != DRIVE_CDROM){
			if(GetDiskFreeSpaceEx(drive, NULL, (PULARGE_INTEGER)&size, (PULARGE_INTEGER)&free)){
				totalFree += free;
				totalSize += size;
				
				ret.append(drive);
				ret += "=" + Util::formatBytes(size) + " ";
			}
		}

		++drive[0];
		drives = (drives >> 1);
	}

	if(totalSize != 0)
		ret += "total=" + Util::formatBytes(totalFree) + "/" + Util::formatBytes(totalSize);

	return ret;
}

string WinUtil::Help(const string& command) {
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
	}catch (Exception&) { 
		//if we for some reason failed, return false to indicate that a refresh is needed
		return Util::emptyString;
	}

	string ret = Util::emptyString;
	try{
		xml.fromXML(xmlString);

		xml.stepIn();

		bool found = false;
		if(Util::toLower(command).find("dc++") != string::npos)
			found = xml.findChild("dc");
		else
			found = xml.findChild(Util::toLower(command));

		if(found)
			ret = xml.getChildData();
	} catch(const SimpleXMLException &e) {
		return e.getError();
	}

	return ret;
}

string WinUtil::Uptime() {
	HQUERY    hQuery	= NULL;
	HCOUNTER  hCounter	= NULL;
	string ret = Util::emptyString;
	
	if ( PdhOpenQuery( NULL, 0, &hQuery ) == ERROR_SUCCESS ) {
		if(PdhAddCounter( hQuery, "\\System\\System Up Time", 0, &hCounter ) == ERROR_SUCCESS) {
			PDH_FMT_COUNTERVALUE  pdhCounterValue;

			if ( PdhCollectQueryData( hQuery ) == ERROR_SUCCESS )	{
				if ( PdhGetFormattedCounterValue( hCounter, PDH_FMT_LARGE, NULL, &pdhCounterValue ) == ERROR_SUCCESS )
					ret = Util::formatTime(pdhCounterValue.largeValue, false);
			}
		}
	}
	
	
	if ( hCounter )   
		PdhRemoveCounter( hCounter );
	if ( hQuery )
		PdhCloseQuery   ( hQuery );

	return ret;
}
/**
 * @file
 * $Id: WinUtil.cpp,v 1.21 2004/02/21 15:43:54 trem Exp $
 */
