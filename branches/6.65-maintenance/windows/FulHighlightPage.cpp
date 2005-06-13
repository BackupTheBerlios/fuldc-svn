/* 
* Copyright (C) 2003-2005 P�r Bj�rklund, per.bjorklund@gmail.com
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
#include "WinUtil.h"

PropPage::TextItem FulHighlightPage::texts[] = {
	{ IDC_SB_HIGHLIGHT,  ResourceManager::SETTINGS_SB_HIGHLIGHT		},
	{ IDC_SB_SETTINGS,   ResourceManager::SETTINGS_SB_SETTINGS		},
	{ IDC_ST_MATCH_TYPE, ResourceManager::SETTINGS_ST_MATCH_TYPE	},
	{ IDC_ADD,			 ResourceManager::ADD						},
	{ IDC_DELETE,		 ResourceManager::REMOVE					},
	{ IDC_UPDATE,		 ResourceManager::SETTINGS_BTN_UPDATE		},
	{ IDC_MOVEUP,		 ResourceManager::SETTINGS_BTN_MOVEUP		},
	{ IDC_MOVEDOWN,		 ResourceManager::SETTINGS_BTN_MOVEDOWN		},
	{ IDC_BGCOLOR,		 ResourceManager::SETTINGS_BTN_BGCOLOR		},
	{ IDC_FGCOLOR,		 ResourceManager::SETTINGS_BTN_TEXTCOLOR	},
	{ IDC_SELECT_SOUND,  ResourceManager::SETTINGS_SELECT_SOUND		},
	{ IDC_BOLD,			 ResourceManager::BOLD						},
	{ IDC_ITALIC,		 ResourceManager::ITALIC					},
	{ IDC_UNDERLINE,	 ResourceManager::UNDERLINE					},
	{ IDC_STRIKEOUT,	 ResourceManager::STRIKEOUT					},
	{ IDC_POPUP,		 ResourceManager::SETTINGS_POPUP			},
	{ IDC_SOUND,		 ResourceManager::SETTINGS_PLAY_SOUND		},
	{ IDC_FLASHWINDOW,	 ResourceManager::SETTINGS_FLASH_WINDOW		},
	{ IDC_INCLUDENICK,	 ResourceManager::SETTINGS_INCLUDE_NICK		},
	{ IDC_WHOLELINE,	 ResourceManager::SETTINGS_WHOLE_LINE		},
	{ IDC_CASESENSITIVE, ResourceManager::SETTINGS_CASE_SENSITIVE	},
	{ IDC_WHOLEWORD,	 ResourceManager::SETTINGS_ENTIRE_WORD		},
	{ IDC_TABCOLOR,		 ResourceManager::SETTINGS_TAB_COLOR		},
	{ IDC_LASTLOG,		 ResourceManager::SETTINGS_LASTLOG			}, 
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
	
	//initalize colors
	bgColor = WinUtil::bgColor;
	fgColor = WinUtil::textColor;

	//initalize ComboBox
	ctrlMatchType.Attach(GetDlgItem(IDC_MATCHTYPE));

	//add alternatives
	StringTokenizer<tstring> s(Text::toT(STRING(HIGHLIGHT_MATCH_TYPES)), _T(','));
	TStringList l = s.getTokens();
	for(TStringIter i = l.begin(); i != l.end(); ++i)
		ctrlMatchType.AddString((*i).c_str());

	ctrlMatchType.SetCurSel(1);

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
	ColorSettings cs;
	getValues(&cs);

	if(cs.getMatch().empty()){
		MessageBox(CTSTRING(ADD_EMPTY), _T(FULDC) _T(" ") _T(FULVERSIONSTRING), MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	if(cs.getMatch().find(_T("$Re:")) == 0) {
		PME reg(cs.getMatch().substr(4));
		if(! reg.IsValid()){
			MessageBox(CTSTRING(BAD_REGEXP), _T(FULDC) _T(" ") _T(FULVERSIONSTRING), MB_OK | MB_ICONEXCLAMATION);
			return TRUE;
		}
	}

	//add the string to the listview
	highlights.push_back(cs);
	ctrlStrings.insert( ctrlStrings.GetItemCount(), cs.getMatch());
	ctrlStrings.SelectItem(ctrlStrings.GetItemCount()-1);

	return TRUE;
}

LRESULT FulHighlightPage::onUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int sel = ctrlStrings.GetSelectedIndex();
	if(sel == -1)
		return true;

	ColorSettings *cs = &highlights[sel];
	tstring old = cs->getMatch();

	getValues(cs);
		
	if(old.compare(cs->getMatch()) != 0){
		ctrlStrings.DeleteItem(sel);
		ctrlStrings.insert(sel, cs->getMatch());
		ctrlStrings.SelectItem(sel);
		
	}
	return true;
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

LRESULT FulHighlightPage::onFgColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	CColorDialog dlg(fgColor, CC_FULLOPEN);
	if(dlg.DoModal() == IDOK) {
		fgColor = dlg.GetColor();
	}
	return TRUE;
}

LRESULT FulHighlightPage::onBgColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	CColorDialog dlg(bgColor, CC_FULLOPEN);
	if(dlg.DoModal() == IDOK) {
		bgColor = dlg.GetColor();
	}
	return TRUE;
}

LRESULT FulHighlightPage::onSelSound(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CFileDialog dlg(TRUE);
	dlg.m_ofn.lpstrFilter = _T("Wave Files\0*.wav\0\0");
	if(dlg.DoModal() == IDOK){
		soundFile = dlg.m_ofn.lpstrFile;
	}

	return TRUE;
}

void FulHighlightPage::clear() {

	SetDlgItemText(IDC_STRING, _T(""));

	CheckDlgButton(IDC_BOLD,			BST_UNCHECKED);
	CheckDlgButton(IDC_ITALIC,			BST_UNCHECKED);
	CheckDlgButton(IDC_UNDERLINE,		BST_UNCHECKED);
	CheckDlgButton(IDC_STRIKEOUT,		BST_UNCHECKED);
	CheckDlgButton(IDC_INCLUDENICK,		BST_UNCHECKED);
	CheckDlgButton(IDC_WHOLELINE,		BST_UNCHECKED);
	CheckDlgButton(IDC_CASESENSITIVE,	BST_UNCHECKED);
	CheckDlgButton(IDC_WHOLEWORD,		BST_UNCHECKED);
	CheckDlgButton(IDC_POPUP,			BST_UNCHECKED);
	CheckDlgButton(IDC_TABCOLOR,		BST_UNCHECKED);
	CheckDlgButton(IDC_SOUND,			BST_UNCHECKED);
	CheckDlgButton(IDC_LASTLOG,			BST_UNCHECKED);
	CheckDlgButton(IDC_FLASHWINDOW,		BST_UNCHECKED);
	CheckDlgButton(IDC_HAS_FG_COLOR,	BST_UNCHECKED);
	CheckDlgButton(IDC_HAS_BG_COLOR,	BST_UNCHECKED);

	ctrlMatchType.SetCurSel(1);

	bgColor = WinUtil::bgColor;
	fgColor = WinUtil::textColor;

	soundFile.clear();

	BOOL t;
	onClickedBox(0, IDC_HAS_BG_COLOR, NULL, t);
	onClickedBox(0, IDC_HAS_FG_COLOR, NULL, t);
	onClickedBox(0, IDC_SOUND, NULL, t);
}

FulHighlightPage::~FulHighlightPage() {
	ctrlStrings.Detach();
	ctrlMatchType.Detach();
}
void FulHighlightPage::getValues(ColorSettings* cs){
	TCHAR buf[1000];
	GetDlgItemText(IDC_STRING, buf, 1000);
	cs->setMatch(buf);

	cs->setBold(	  IsDlgButtonChecked(IDC_BOLD)		== BST_CHECKED );
	cs->setItalic(	  IsDlgButtonChecked(IDC_ITALIC)	== BST_CHECKED );
	cs->setUnderline( IsDlgButtonChecked(IDC_UNDERLINE)	== BST_CHECKED );
	cs->setStrikeout( IsDlgButtonChecked(IDC_STRIKEOUT) == BST_CHECKED );

	cs->setCaseSensitive( IsDlgButtonChecked(IDC_CASESENSITIVE) == BST_CHECKED );
	cs->setIncludeNick(	  IsDlgButtonChecked(IDC_INCLUDENICK)	== BST_CHECKED );
	cs->setWholeLine(	  IsDlgButtonChecked(IDC_WHOLELINE)		== BST_CHECKED );
	cs->setWholeWord(	  IsDlgButtonChecked(IDC_WHOLEWORD)		== BST_CHECKED );
	cs->setPopup(		  IsDlgButtonChecked(IDC_POPUP)			== BST_CHECKED );
	cs->setTab(			  IsDlgButtonChecked(IDC_TABCOLOR)		== BST_CHECKED );
	cs->setPlaySound(	  IsDlgButtonChecked(IDC_SOUND)			== BST_CHECKED );
	cs->setLog(			  IsDlgButtonChecked(IDC_LASTLOG)		== BST_CHECKED );
	cs->setFlashWindow(	  IsDlgButtonChecked(IDC_FLASHWINDOW)	== BST_CHECKED );
	cs->setHasBgColor(	  IsDlgButtonChecked(IDC_HAS_BG_COLOR)	== BST_CHECKED );
    cs->setHasFgColor(	  IsDlgButtonChecked(IDC_HAS_FG_COLOR)	== BST_CHECKED );

	
	cs->setBgColor( bgColor );
	cs->setFgColor( fgColor );

	cs->setMatchType( ctrlMatchType.GetCurSel() );

	cs->setSoundFile( soundFile );

}


LRESULT FulHighlightPage::onItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/) {
	if(ctrlStrings.GetSelectedCount() != 1) {
		return TRUE;
	}
	int sel = ctrlStrings.GetSelectedIndex();
	ColorSettings *cs = &highlights[sel];

	if(cs == NULL)
		return TRUE;

	SetDlgItemText(IDC_STRING, cs->getMatch().c_str());
	ctrlMatchType.SetCurSel(cs->getMatchType());

	CheckDlgButton(IDC_BOLD			, cs->getBold()			 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_ITALIC		, cs->getItalic()		 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_UNDERLINE	, cs->getUnderline()	 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_STRIKEOUT	, cs->getStrikeout()	 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_INCLUDENICK	, cs->getIncludeNick()	 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_WHOLELINE	, cs->getWholeLine()	 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_CASESENSITIVE, cs->getCaseSensitive() ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_WHOLEWORD	, cs->getWholeWord()	 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_POPUP		, cs->getPopup()		 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_TABCOLOR		, cs->getTab()			 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_SOUND		, cs->getPlaySound()	 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_LASTLOG		, cs->getLog()			 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_FLASHWINDOW  , cs->getFlashWindow()   ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_HAS_BG_COLOR , cs->getHasBgColor()	 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_HAS_FG_COLOR , cs->getHasFgColor()	 ? BST_CHECKED : BST_UNCHECKED);

	if(cs->getHasBgColor())
		bgColor = cs->getBgColor();
	if(cs->getHasFgColor())
		fgColor = cs->getFgColor();

	if(cs->getPlaySound())
		soundFile = cs->getSoundFile();

	BOOL t;
	onClickedBox(0, IDC_HAS_BG_COLOR, NULL, t);
	onClickedBox(0, IDC_HAS_FG_COLOR, NULL, t);
	onClickedBox(0, IDC_SOUND, NULL, t);
	
	return TRUE;
}

LRESULT FulHighlightPage::onClickedBox(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	int button = 0;
	switch(wID) {
	case IDC_SOUND:		   button = IDC_SELECT_SOUND; break;
	case IDC_HAS_BG_COLOR: button = IDC_BGCOLOR;	  break;
	case IDC_HAS_FG_COLOR: button = IDC_FGCOLOR;	  break;
	}

	bool enabled = IsDlgButtonChecked(wID) == BST_CHECKED;
	ctrlButton.Attach(GetDlgItem(button));
	ctrlButton.EnableWindow(enabled);
	ctrlButton.Detach();

	return TRUE;
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
			clear();
			addPreset(i);
			SendMessage(WM_COMMAND, IDC_ADD);
		}
	} else {
		clear();
		addPreset(wParam);
	}
	return 0;
}

void FulHighlightPage::addPreset(int preset) {
	switch(preset){
		case 2:
			SetDlgItemText(IDC_STRING, _T("$mynick$"));
			CheckDlgButton(IDC_BOLD, BST_CHECKED);
			CheckDlgButton(IDC_LASTLOG, BST_CHECKED);
			CheckDlgButton(IDC_POPUP, BST_CHECKED);
			CheckDlgButton(IDC_HAS_FG_COLOR, BST_CHECKED);
			fgColor = RGB(200, 0, 0);
			break;
		case 3:
			SetDlgItemText(IDC_STRING, _T("<$mynick$>"));
			CheckDlgButton(IDC_INCLUDENICK, BST_CHECKED);
			CheckDlgButton(IDC_WHOLELINE, BST_CHECKED);
			CheckDlgButton(IDC_HAS_FG_COLOR, BST_CHECKED);
			fgColor = RGB(115, 115, 118);
			break;
		case 4:
			SetDlgItemText(IDC_STRING, _T("$Re:\\s(http://\\S+|ftp://\\S+|https://\\S+|mms://\\S+|ftps://\\S+|www\\.\\S+)"));
			CheckDlgButton(IDC_UNDERLINE, BST_CHECKED);
			CheckDlgButton(IDC_LASTLOG, BST_CHECKED);
			CheckDlgButton(IDC_HAS_FG_COLOR, BST_CHECKED);
			fgColor = RGB(102, 153, 204);
			break;
		case 5:
			SetDlgItemText(IDC_STRING, _T("$Re:^\\[.*?\\] (\\*{2} .*)"));
			CheckDlgButton(IDC_HAS_FG_COLOR, BST_CHECKED);
			fgColor = RGB(255, 102, 0);
			break;
		case 6:
			SetDlgItemText(IDC_STRING, _T("$Re:^\\[.*?\\] (\\*{3} .*)")); //will do for now
			CheckDlgButton(IDC_HAS_FG_COLOR, BST_CHECKED);
			fgColor = RGB(255, 153, 204);
			break;
		case 7:
			SetDlgItemText(IDC_STRING, _T("$Re:^\\[.*?\\] (\\*{3} Joins: .*)"));
			CheckDlgButton(IDC_HAS_FG_COLOR, BST_CHECKED);
			CheckDlgButton(IDC_BOLD, BST_CHECKED);
			fgColor = RGB(153, 153, 51);
			break;
		case 8:
			SetDlgItemText(IDC_STRING, _T("$Re:^\\[.*?\\] (\\*{3} Parts: .*)"));
			CheckDlgButton(IDC_HAS_FG_COLOR, BST_CHECKED);
			CheckDlgButton(IDC_BOLD, BST_CHECKED);
			fgColor = RGB(51, 102, 154);
			break;
		case 9:
			SetDlgItemText(IDC_STRING, _T("$Re:(\\S+\\.\\S*(?:dvdrip|xvid|dvdr|svcd|vcd|rip|hdtv)\\S?-\\S+)"));
			CheckDlgButton(IDC_BOLD, BST_CHECKED);
			CheckDlgButton(IDC_HAS_FG_COLOR, BST_CHECKED);
			fgColor = RGB(153, 51, 153);
			break;
		case 10:
			SetDlgItemText(IDC_STRING, _T("$ts$"));
			CheckDlgButton(IDC_INCLUDENICK, BST_CHECKED);
			CheckDlgButton(IDC_BOLD, BST_CHECKED);
			CheckDlgButton(IDC_HAS_FG_COLOR, BST_CHECKED);
			fgColor = RGB(115, 115, 118);
			break;
		case 11:
			SetDlgItemText(IDC_STRING, _T("$users$"));
			CheckDlgButton(IDC_INCLUDENICK, BST_CHECKED);
			CheckDlgButton(IDC_BOLD, BST_CHECKED);
			CheckDlgButton(IDC_HAS_FG_COLOR, BST_CHECKED);
			fgColor = RGB(115, 115, 118);
			break;
		default:
			break;
	}

	BOOL t;
	onClickedBox(0, IDC_HAS_BG_COLOR, NULL, t);
	onClickedBox(0, IDC_HAS_FG_COLOR, NULL, t);
	onClickedBox(0, IDC_SOUND, NULL, t);
}