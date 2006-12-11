/* 
* Copyright (C) 2003-2005 Pär Björklund, per.bjorklund@gmail.com
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

#include "../client/SettingsManager.h"
#include "../client/ColorSettings.h"
#include "../client/HighlightManager.h"
#include "../client/StringTokenizer.h"
#include "../client/version.h"
#include "../client/pme.h"

#include "FulHighlightPage.h"
#include "FulHighlightDialog.h"
#include "WinUtil.h"

PropPage::TextItem FulHighlightPage::texts[] = {
	{ IDC_ADD,			 ResourceManager::HIGHLIGHT_ADD				},
	{ IDC_DELETE,		 ResourceManager::REMOVE					},
	{ IDC_UPDATE,		 ResourceManager::HIGHLIGHT_CHANGE			},
	{ IDC_MOVEUP,		 ResourceManager::SETTINGS_BTN_MOVEUP		},
	{ IDC_MOVEDOWN,		 ResourceManager::SETTINGS_BTN_MOVEDOWN		},
	{ 0,				 ResourceManager::SETTINGS_AUTO_AWAY		}
};

LRESULT FulHighlightPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::translate((HWND)(*this), texts);

	CRect rc;
	
	//Initalize listview
	ctrlStrings.Attach(GetDlgItem(IDC_ITEMS));
	ctrlStrings.GetClientRect(rc);
	ctrlStrings.InsertColumn(0, CTSTRING(HIGHLIGHTLIST_HEADER), LVCFMT_LEFT, rc.Width(), 0);
	ctrlStrings.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	ColorList* cList = HighlightManager::getInstance()->getList();
		
	//populate listview with current strings
	highlights.reserve(cList->size());
	for(ColorIter i = cList->begin();i != cList->end(); ++i) {
		highlights.push_back((*i));
		ctrlStrings.insert( ctrlStrings.GetItemCount(), (*i).getMatch());
	}
	
	presets.CreatePopupMenu();
	presets.AppendMenu(MF_STRING, IDC_PRESETMENU, CTSTRING(PRESET_ADD_ALL));
	presets.AppendMenu(MF_SEPARATOR);
	presets.AppendMenu(MF_STRING, IDC_PRESETMENU, CTSTRING(PRESET_MY_NICK));
	presets.AppendMenu(MF_STRING, IDC_PRESETMENU, CTSTRING(PRESET_MY_MESSAGES));
	presets.AppendMenu(MF_STRING, IDC_PRESETMENU, CTSTRING(PRESET_URLS));
	presets.AppendMenu(MF_STRING, IDC_PRESETMENU, CTSTRING(PRESET_ME));
	presets.AppendMenu(MF_STRING, IDC_PRESETMENU, CTSTRING(PRESET_STATUS));
	presets.AppendMenu(MF_STRING, IDC_PRESETMENU, CTSTRING(PRESET_JOINS));
	presets.AppendMenu(MF_STRING, IDC_PRESETMENU, CTSTRING(PRESET_PARTS));
	presets.AppendMenu(MF_STRING, IDC_PRESETMENU, CTSTRING(PRESET_RELEASES));
	presets.AppendMenu(MF_STRING, IDC_PRESETMENU, CTSTRING(PRESET_TIMESTAMP));
	presets.AppendMenu(MF_STRING, IDC_PRESETMENU, CTSTRING(PRESET_USERS));

	MENUINFO inf;
	inf.cbSize = sizeof(MENUINFO);
	inf.fMask = MIM_STYLE;
	inf.dwStyle = MNS_NOTIFYBYPOS;
	presets.SetMenuInfo(&inf);

	return TRUE;
}

void FulHighlightPage::write(){
	HighlightManager::getInstance()->replaceList(highlights);
}

LRESULT FulHighlightPage::onAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	FulHighlightDialog dlg;
	if(dlg.DoModal(WinUtil::mainWnd) == IDOK) {
		//add the string to the listview
		highlights.push_back(dlg.getColorSetting());
		ctrlStrings.insert( ctrlStrings.GetItemCount(), highlights.back().getMatch());
		ctrlStrings.SelectItem(ctrlStrings.GetItemCount()-1);
	}

	return TRUE;
}

LRESULT FulHighlightPage::onUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int sel = ctrlStrings.GetSelectedIndex();
	if(sel == -1)
		return TRUE;
	
	FulHighlightDialog dlg(highlights[sel]);
	if(dlg.DoModal(WinUtil::mainWnd) == IDOK) {
		ColorSettings old, cur;
		old = highlights[sel];
		cur = dlg.getColorSetting();

		if(old.getMatch().compare(cur.getMatch()) != 0){
				ctrlStrings.DeleteItem(sel);
			ctrlStrings.insert(sel, cur.getMatch());
			ctrlStrings.SelectItem(sel);
		} 
		highlights[sel] = cur;
	}
	return TRUE;
}

LRESULT FulHighlightPage::onMove(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int sel = ctrlStrings.GetSelectedIndex();
	if(wID == IDC_MOVEUP && sel > 0){
		ColorSettings cs = highlights[sel];
		highlights[sel] = highlights[sel-1];
		highlights[sel-1] = cs;
		ctrlStrings.DeleteItem(sel);
		ctrlStrings.insert(sel-1, cs.getMatch());
		ctrlStrings.SelectItem(sel-1);
	} else if(wID == IDC_MOVEDOWN && sel < ctrlStrings.GetItemCount()-1){
		//hmm odd, moveItem handles the move but the list doesn't get updated
		//so well this works instead =)
		ColorSettings cs = highlights[sel];
		highlights[sel] = highlights[sel+1];
		highlights[sel+1] = cs;
		ctrlStrings.DeleteItem(sel);
		ctrlStrings.insert(sel+1, cs.getMatch());
		ctrlStrings.SelectItem(sel+1);
	}

	return 0;
}

LRESULT FulHighlightPage::onDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	if(ctrlStrings.GetSelectedCount() == 1) {
		int sel = ctrlStrings.GetSelectedIndex();
		ctrlStrings.DeleteItem(sel);
		
		if(sel > 0 && ctrlStrings.GetItemCount() > 0) {
			ctrlStrings.SelectItem(sel-1);
		} else if(ctrlStrings.GetItemCount() > 0) {
			ctrlStrings.SelectItem(0);
		}

		int j = 0;
		ColorIter i = highlights.begin();
		for(; j < sel; ++i, ++j);

		if(i != highlights.end())
			highlights.erase(i);
	}
	
	return TRUE;
}

FulHighlightPage::~FulHighlightPage() {
	ctrlStrings.Detach();
}


LRESULT FulHighlightPage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_HIGHLIGHTPAGE);
	return 0;
}

LRESULT FulHighlightPage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_HIGHLIGHTPAGE);
	return 0;
}

LRESULT FulHighlightPage::onPreset(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	POINT pt;
	if(GetCursorPos(&pt) != 0) {
		presets.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_HORPOSANIMATION | TPM_VERPOSANIMATION,
			pt.x, pt.y, m_hWnd);
	}
	return 0;
}

LRESULT FulHighlightPage::onMenuCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if(wParam == 0) {
		for(int i = 2; i < 12; ++i) {
			addPreset(i);
		}
	} else {
		addPreset(wParam);
	}
	return 0;
}

void FulHighlightPage::addPreset(int preset) {
	ColorSettings cs;
	switch(preset){
		case 2:
			cs.setMatch(_T("$mynick$"));
			cs.setBold(true);
			cs.setLog(true);
			cs.setPopup(true);
			cs.setHasFgColor(true);
			cs.setFgColor(RGB(200, 0, 0));
			break;
		case 3:
			cs.setMatch(_T("<$mynick$>)"));
			cs.setIncludeNick(true);
			cs.setWholeLine(true);
			cs.setHasFgColor(true);
			cs.setFgColor(RGB(115,115,118));
			break;
		case 4:
			cs.setMatch(_T("$Re:\\s(http://\\S+|ftp://\\S+|https://\\S+|mms://\\S+|ftps://\\S+|www\\.\\S+)"));
			cs.setUnderline(true);
			cs.setLog(true);
			cs.setHasFgColor(true);
			cs.setFgColor(RGB(102, 153, 204));
			break;
		case 5:
			cs.setMatch(_T("$Re:^\\[.*?\\] (\\*{2} .*)"));
			cs.setHasFgColor(true);
			cs.setFgColor(RGB(255, 102, 0));
			break;
		case 6:
			cs.setMatch(_T("$Re:^\\[.*?\\] (\\*{3} .*)")); //will do for now
			cs.setHasFgColor(true);
			cs.setFgColor(RGB(255, 153, 204));
			break;
		case 7:
			cs.setMatch(_T("$Re:^\\[.*?\\] (\\*{3} Joins: .*)"));
			cs.setHasFgColor(true);
			cs.setBold(true);
			cs.setFgColor(RGB(153,153,51));
			break;
		case 8:
			cs.setMatch(_T("$Re:^\\[.*?\\] (\\*{3} Parts: .*)"));
			cs.setHasFgColor(true);
			cs.setBold(true);
			cs.setFgColor(RGB(51, 102, 154));
			break;
		case 9:
			cs.setMatch(_T("$Re:(\\S+\\.\\S*(?:dvdrip|xvid|dvdr|svcd|vcd|rip|hdtv)\\S?-\\S+)"));
			cs.setBold(true);
			cs.setHasFgColor(true);
			cs.setFgColor(RGB(153, 51, 153));
			break;
		case 10:
			cs.setMatch(_T("$ts$"));
			cs.setIncludeNick(true);
			cs.setBold(true);
			cs.setHasFgColor(true);
			cs.setFgColor(RGB(115, 115, 118));
			break;
		case 11:
			cs.setMatch(_T("$users$"));
			cs.setIncludeNick(true);
			cs.setBold(true);
			cs.setHasFgColor(true);
			cs.setFgColor(RGB(115, 115, 118));
			break;
		default:
			break;
	}

	highlights.push_back(cs);
	ctrlStrings.insert( ctrlStrings.GetItemCount(), highlights.back().getMatch());
	ctrlStrings.SelectItem(ctrlStrings.GetItemCount()-1);
}

LRESULT FulHighlightPage::onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled) {
	NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
	switch(kd->wVKey) {
	case VK_INSERT:
		PostMessage(WM_COMMAND, IDC_ADD, 0);
		break;
	case VK_DELETE:
		PostMessage(WM_COMMAND, IDC_REMOVE, 0);
		break;
	default:
		bHandled = FALSE;
	}
	return 0;
}

LRESULT FulHighlightPage::onDoubleClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;

	if(item->iItem >= 0) {
		PostMessage(WM_COMMAND, IDC_UPDATE, 0);
	} else if(item->iItem == -1) {
		PostMessage(WM_COMMAND, IDC_ADD, 0);
	}

	return 0;
}