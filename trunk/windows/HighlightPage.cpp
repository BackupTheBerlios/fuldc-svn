#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "../client/SettingsManager.h"
#include "../client/ColorSettings.h"

#include "../greta/regexpr2.h"

#include "WinUtil.h"
#include "HighlightPage.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

LRESULT HighlightPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CRect rc;
	
	//Initalize listview
	ctrlStrings.Attach(GetDlgItem(IDC_ITEMS));
	ctrlStrings.GetClientRect(rc);
	ctrlStrings.InsertColumn(0, "string to match", LVCFMT_LEFT, rc.Width(), 0);

	colorVector::iterator j = SettingsManager::getInstance()->colorSettings.end();
	colorVector::iterator i = SettingsManager::getInstance()->colorSettings.begin();
		
	//populate listview with current strings
	for(;i != j; ++i)
		ctrlStrings.insert( ctrlStrings.GetItemCount(), (*i)->getMatch(), 0, (LPARAM)(*i) );
		
	
	//initalize colors
	bgColor = WinUtil::bgColor;
	fgColor = WinUtil::textColor;

	//initalize ComboBox
	ctrlMatchType.Attach(GetDlgItem(IDC_MATCHTYPE));

	//add alternatives
	ctrlMatchType.AddString("Begins");
	ctrlMatchType.AddString("Contains");
	ctrlMatchType.AddString("Ends");
	ctrlMatchType.AddString("Equals");

	ctrlMatchType.SetCurSel(1);

	return TRUE;
}

void HighlightPage::write(){
	int i = 0;
	int end = ctrlStrings.GetItemCount();
	ColorSettings *cs;
	settings->colorSettings.clear();
	for(; i < end; ++i){
		cs = (ColorSettings*)ctrlStrings.GetItemData(i);
		settings->colorSettings.push_back(cs);
	}
}

LRESULT HighlightPage::onAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	ColorSettings *cs = new ColorSettings();
	getValues(cs);

	if(cs->getMatch().empty()){
		MessageBox(CSTRING(ADD_EMPTY), "", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	if(cs->getMatch().find("$Re:") == 0) {
		try{
			regex::rpattern reg(cs->getMatch().substr(4));
		}catch(regex::bad_regexpr){
			MessageBox(CSTRING(BAD_REGEXP), "", MB_OK | MB_ICONEXCLAMATION);
			return TRUE;
		}
	}

	//add the string to the listview
	ctrlStrings.insert( ctrlStrings.GetItemCount(), cs->getMatch(), 0, (LPARAM)cs );
	ctrlStrings.SelectItem(ctrlStrings.GetItemCount()-1);

	clear();
		
	return TRUE;
}

LRESULT HighlightPage::onUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int sel = ctrlStrings.GetSelectedIndex();
	if(sel == -1)
		return true;

	ColorSettings *cs = (ColorSettings*)ctrlStrings.GetItemData(sel);
	string old = cs->getMatch();
	getValues(cs);
	
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
		delete cs;
		cs = NULL;
	}
	
	return TRUE;
}

LRESULT HighlightPage::onFgColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	CColorDialog dlg(WinUtil::textColor, CC_FULLOPEN);
	if(dlg.DoModal() == IDOK) {
		fgColor = dlg.GetColor();
	}
	return TRUE;
}

LRESULT HighlightPage::onBgColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	CColorDialog dlg(WinUtil::bgColor, CC_FULLOPEN);
	if(dlg.DoModal() == IDOK) {
		bgColor = dlg.GetColor();
	}
	return TRUE;
}

LRESULT HighlightPage::onSelSound(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CFileDialog dlg(TRUE);
	dlg.m_ofn.lpstrFilter = "Wave Files\0*.wav\0\0";
	if(dlg.DoModal() == IDOK){
		soundFile = dlg.m_ofn.lpstrFile;
	}

	return TRUE;
}

void HighlightPage::clear() {

	SetDlgItemText(IDC_STRING, "");

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
	char buf[1000];
	GetDlgItemText(IDC_STRING, buf, 1000);
	cs->setMatch(buf);

	cs->setBold(	  IsDlgButtonChecked(IDC_BOLD)	    );
	cs->setItalic(	  IsDlgButtonChecked(IDC_ITALIC)    );
	cs->setUnderline( IsDlgButtonChecked(IDC_UNDERLINE) );
	cs->setStrikeout( IsDlgButtonChecked(IDC_STRIKEOUT) );

	cs->setCaseSensitive( IsDlgButtonChecked(IDC_CASESENSITIVE)	 );
	cs->setIncludeNick(	  IsDlgButtonChecked(IDC_INCLUDENICK)	 );
	cs->setWholeLine(	  IsDlgButtonChecked(IDC_WHOLELINE)		 );
	cs->setWholeWord(	  IsDlgButtonChecked(IDC_WHOLEWORD)		 );
	cs->setPopup(		  IsDlgButtonChecked(IDC_POPUP)			 );
	cs->setTab(			  IsDlgButtonChecked(IDC_TABCOLOR)		 );
	cs->setPlaySound(	  IsDlgButtonChecked(IDC_SOUND)			 );
	cs->setHasBgColor(	  IsDlgButtonChecked(IDC_HAS_BG_COLOR)	 );
    cs->setHasFgColor(	  IsDlgButtonChecked(IDC_HAS_FG_COLOR)	 );
	
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

	bool enabled = IsDlgButtonChecked(wID);
	ctrlButton.Attach(GetDlgItem(button));
	ctrlButton.EnableWindow(enabled);
	ctrlButton.Detach();

	return TRUE;
}