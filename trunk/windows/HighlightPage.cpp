#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "../client/SettingsManager.h"
#include "../client/ColorSettings.h"
#include "../client/HighlightManager.h"
#include "../client/StringTokenizer.h"

#include "../client/pme.h"

#include "HighlightPage.h"
#include "WinUtil.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::TextItem HighlightPage::texts[] = {
	{ IDC_SB_HIGHLIGHT,  ResourceManager::SETTINGS_SB_HIGHLIGHT		},
	{ IDC_SB_SETTINGS,   ResourceManager::SETTINGS_SB_SETTINGS		},
	{ IDC_ST_MATCH_TYPE, ResourceManager::SETTINGS_ST_MATCH_TYPE	},
	{ IDC_ADD,			 ResourceManager::ADD						},
	{ IDC_DELETE,		 ResourceManager::REMOVE					},
	{ IDC_UPDATE,		 ResourceManager::SETTINGS_BTN_UPDATE		},
	{ IDC_MOVE_UP,		 ResourceManager::SETTINGS_BTN_MOVEUP		},
	{ IDC_MOVE_DOWN,	 ResourceManager::SETTINGS_BTN_MOVEDOWN		},
	{ IDC_BGCOLOR,		 ResourceManager::SETTINGS_BTN_BGCOLOR		},
	{ IDC_FGCOLOR,		 ResourceManager::SETTINGS_BTN_TEXTCOLOR	},
	{ IDC_SELECT_SOUND,  ResourceManager::SETTINGS_SELECT_SOUND		},
	{ IDC_BOLD,			 ResourceManager::BOLD						},
	{ IDC_ITALIC,		 ResourceManager::ITALIC					},
	{ IDC_UNDERLINE,	 ResourceManager::UNDERLINE					},
	{ IDC_STRIKEOUT,	 ResourceManager::STRIKEOUT					},
	{ IDC_POPUP,		 ResourceManager::SETTINGS_POPUP			},
	{ IDC_SOUND,		 ResourceManager::SETTINGS_PLAY_SOUND		},
	{ IDC_INCLUDENICK,	 ResourceManager::SETTINGS_INCLUDE_NICK		},
	{ IDC_WHOLELINE,	 ResourceManager::SETTINGS_WHOLE_LINE		},
	{ IDC_CASESENSITIVE, ResourceManager::SETTINGS_CASE_SENSITIVE	},
	{ IDC_WHOLEWORD,	 ResourceManager::SETTINGS_ENTIRE_WORD		},
	{ IDC_TABCOLOR,		 ResourceManager::SETTINGS_TAB_COLOR		},
	{ IDC_LASTLOG,		 ResourceManager::SETTINGS_LASTLOG			}, 
	{ 0,				 ResourceManager::SETTINGS_AUTO_AWAY		}
};

LRESULT HighlightPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::translate((HWND)(*this), texts);

	CRect rc;
	
	//Initalize listview
	ctrlStrings.Attach(GetDlgItem(IDC_ITEMS));
	ctrlStrings.GetClientRect(rc);
	ctrlStrings.InsertColumn(0, CTSTRING(HIGHLIGHTLIST_HEADER), LVCFMT_LEFT, rc.Width(), 0);

	ColorList* cList = HighlightManager::getInstance()->rLock();
		
	//populate listview with current strings
	for(ColorIter i = cList->begin();i != cList->end(); ++i)
		ctrlStrings.insert( ctrlStrings.GetItemCount(), (*i)->getMatch(), 0, (LPARAM)(*i) );
	
	HighlightManager::getInstance()->rUnlock();
	
	//initalize colors
	bgColor = WinUtil::bgColor;
	fgColor = WinUtil::textColor;

	//initalize ComboBox
	ctrlMatchType.Attach(GetDlgItem(IDC_MATCHTYPE));

	//add alternatives
	StringTokenizer<tstring> s(WinUtil::toT(STRING(HIGHLIGHT_MATCH_TYPES)), _T(","));
	TStringList l = s.getTokens();
	for(TStringIter i = l.begin(); i != l.end(); ++i)
		ctrlMatchType.AddString((*i).c_str());

	ctrlMatchType.SetCurSel(1);

	return TRUE;
}

void HighlightPage::write(){
	int i = 0;
	int end = ctrlStrings.GetItemCount();
	ColorSettings *cs;
	ColorList* cList = HighlightManager::getInstance()->wLock();
    cList->clear();
	for(; i < end; ++i){
		cs = (ColorSettings*)ctrlStrings.GetItemData(i);
		cList->push_back(cs);
	}

	HighlightManager::getInstance()->wUnlock();
}

LRESULT HighlightPage::onAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	ColorSettings *cs = new ColorSettings();
	getValues(cs);

	if(cs->getMatch().empty()){
		MessageBox(CTSTRING(ADD_EMPTY), _T(""), MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	if(cs->getMatch().find(_T("$Re:")) == 0) {
		PME reg(cs->getMatch().substr(4));
		if(! reg.IsValid()){
			MessageBox(CTSTRING(BAD_REGEXP), _T(""), MB_OK | MB_ICONEXCLAMATION);
			return TRUE;
		}
	}

	//add the string to the listview
	ctrlStrings.insert( ctrlStrings.GetItemCount(), cs->getMatch(), 0, (LPARAM)cs );
	ctrlStrings.SelectItem(ctrlStrings.GetItemCount()-1);

	return TRUE;
}

LRESULT HighlightPage::onUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int sel = ctrlStrings.GetSelectedIndex();
	if(sel == -1)
		return true;

	ColorSettings *cs = (ColorSettings*)ctrlStrings.GetItemData(sel);
	tstring old = cs->getMatch();

	HighlightManager::getInstance()->wLock();
	getValues(cs);
	HighlightManager::getInstance()->wUnlock();
	
	if(old.compare(cs->getMatch()) != 0){
		ctrlStrings.DeleteItem(sel);
		ctrlStrings.insert(sel, cs->getMatch(), 0, (LPARAM)cs);
		ctrlStrings.SelectItem(sel);
		
	}
	return true;
}

LRESULT HighlightPage::onMove(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int sel = ctrlStrings.GetSelectedIndex();
	if(wID == IDC_MOVEUP && sel > 0){
		ctrlStrings.moveItem(sel, sel-1);
		ctrlStrings.SelectItem(sel-1);
	} else if(wID == IDC_MOVEDOWN && sel < ctrlStrings.GetItemCount()-1){
		//hmm odd, moveItem handles the move but the list doesn't get updated
		//so well this works instead =)
		ColorSettings *cs = (ColorSettings*)ctrlStrings.GetItemData(sel);
		ctrlStrings.DeleteItem(sel);
		ctrlStrings.insert(sel+1, cs->getMatch(), 0, (LPARAM)cs);
		ctrlStrings.SelectItem(sel+1);
	}

	return 0;
}

LRESULT HighlightPage::onDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	if(ctrlStrings.GetSelectedCount() == 1) {
		int sel = ctrlStrings.GetSelectedIndex();
		ColorSettings* cs = (ColorSettings*)ctrlStrings.GetItemData(sel);
		ctrlStrings.DeleteItem(sel);
		HighlightManager::getInstance()->wLock();
		delete cs;
		cs = NULL;
		HighlightManager::getInstance()->wUnlock();
	}
	
	return TRUE;
}

LRESULT HighlightPage::onFgColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	CColorDialog dlg(fgColor, CC_FULLOPEN);
	if(dlg.DoModal() == IDOK) {
		fgColor = dlg.GetColor();
	}
	return TRUE;
}

LRESULT HighlightPage::onBgColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	CColorDialog dlg(bgColor, CC_FULLOPEN);
	if(dlg.DoModal() == IDOK) {
		bgColor = dlg.GetColor();
	}
	return TRUE;
}

LRESULT HighlightPage::onSelSound(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CFileDialog dlg(TRUE);
	dlg.m_ofn.lpstrFilter = _T("Wave Files\0*.wav\0\0");
	if(dlg.DoModal() == IDOK){
		soundFile = dlg.m_ofn.lpstrFile;
	}

	return TRUE;
}

void HighlightPage::clear() {

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

	ctrlMatchType.SetCurSel(1);

	bgColor = WinUtil::bgColor;
	fgColor = WinUtil::textColor;

	soundFile.clear();

	BOOL t;
	onClickedBox(0, IDC_HAS_BG_COLOR, NULL, t);
	onClickedBox(0, IDC_HAS_FG_COLOR, NULL, t);
	onClickedBox(0, IDC_SELECT_SOUND, NULL, t);
}

HighlightPage::~HighlightPage() {
	ctrlStrings.Detach();
	ctrlMatchType.Detach();
}
void HighlightPage::getValues(ColorSettings* cs){
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
	cs->setHasBgColor(	  IsDlgButtonChecked(IDC_HAS_BG_COLOR)	== BST_CHECKED );
    cs->setHasFgColor(	  IsDlgButtonChecked(IDC_HAS_FG_COLOR)	== BST_CHECKED );

	
	cs->setBgColor( bgColor );
	cs->setFgColor( fgColor );

	cs->setMatchType( ctrlMatchType.GetCurSel() );

	cs->setSoundFile( soundFile );

}


LRESULT HighlightPage::onItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/) {
	if(ctrlStrings.GetSelectedCount() != 1) {
		return TRUE;
	}
	int sel = ctrlStrings.GetSelectedIndex();
	ColorSettings *cs = (ColorSettings*)ctrlStrings.GetItemData(sel);

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

LRESULT HighlightPage::onClickedBox(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
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