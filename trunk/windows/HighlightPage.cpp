#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "../client/SettingsManager.h"

#include "../greta/regexpr2.h"

#include "WinUtil.h"
#include "HighlightPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
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
	StringList cols;
	
	//populate listview with current strings
	for(;i != j; ++i)
	{
		cols.push_back(i->getMatch());
		ctrlStrings.insert( cols );
		cols.clear();
	}
	
	
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
	//ctrlStrings.Detach();
	//ctrlMatchType.Detach();
}

LRESULT HighlightPage::onAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	ColorSettings cs;

	getValues(cs);

	if(cs.getMatch().empty()){
		MessageBox(CSTRING(ADD_EMPTY), "", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	if(cs.getMatch().find("$Re:") == 0) {
		try{
			regex::rpattern reg(cs.getMatch().substr(4));
		}catch(regex::bad_regexpr){
			MessageBox(CSTRING(BAD_REGEXP), "", MB_OK | MB_ICONEXCLAMATION);
			return TRUE;
		}
	}

	colorVector::iterator i = SettingsManager::getInstance()->colorSettings.begin();
	colorVector::iterator j = SettingsManager::getInstance()->colorSettings.end();
	bool update = false;
	for(;i != j; ++i) {
		if(i->getMatch().compare(cs.getMatch()) == 0){
			SettingsManager::getInstance()->colorSettings.erase(i);
			update = true;
			break;
		}
	}

	if(!update) {
		//add the string to the listview
		StringList cols;
		cols.push_back(cs.getMatch());
		ctrlStrings.insert(cols);
		clear();
	}
	
	//add highlight settings to the vector
	SettingsManager::getInstance()->colorSettings.push_back(cs);

	return TRUE;
}

LRESULT HighlightPage::onDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	if(ctrlStrings.GetSelectedCount() == 1) {
		int sel = ctrlStrings.GetSelectedIndex();
		char buf[100];
		ctrlStrings.GetItemText(sel, 0, buf, 100);
		ctrlStrings.deleteItem(buf);

		colorVector::iterator i = SettingsManager::getInstance()->colorSettings.begin();
		colorVector::iterator j = SettingsManager::getInstance()->colorSettings.end();

		for(; i != j; ++i) {
			if( i->getMatch().compare(buf) == 0 ) {
				SettingsManager::getInstance()->colorSettings.erase(i);
			}
		}
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

	CheckDlgButton(IDC_BOLD, BST_UNCHECKED);
	CheckDlgButton(IDC_ITALIC, BST_UNCHECKED);
	CheckDlgButton(IDC_UNDERLINE, BST_UNCHECKED);
	CheckDlgButton(IDC_STRIKEOUT, BST_UNCHECKED);
	
	CheckDlgButton(IDC_INCLUDENICK, BST_UNCHECKED);
	CheckDlgButton(IDC_WHOLELINE, BST_UNCHECKED);
	CheckDlgButton(IDC_CASESENSITIVE, BST_UNCHECKED);
	CheckDlgButton(IDC_WHOLEWORD, BST_UNCHECKED);
	CheckDlgButton(IDC_POPUP, BST_UNCHECKED);
	CheckDlgButton(IDC_TABCOLOR, BST_UNCHECKED);

	ctrlMatchType.SetCurSel(1);

	bgColor = WinUtil::bgColor;
	fgColor = WinUtil::textColor;

	soundFile.clear();
}

HighlightPage::~HighlightPage() {
	ctrlStrings.Detach();
	ctrlMatchType.Detach();
}

LRESULT HighlightPage::onItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/) {
	if(ctrlStrings.GetSelectedCount() == 1) {
		int sel = ctrlStrings.GetSelectedIndex();
		char buf[100];
		ctrlStrings.GetItemText(sel, 0, buf, 100);
		
		colorVector::iterator i = SettingsManager::getInstance()->colorSettings.begin();
		colorVector::iterator j = SettingsManager::getInstance()->colorSettings.end();

		for(; i != j; ++i) {
			if( i->getMatch().compare(buf) == 0 ) {
				break;
			}
		}

		if(i != j) {
			SetDlgItemText(IDC_STRING, i->getMatch().c_str());
			ctrlMatchType.SetCurSel(i->getMatchType());

			CheckDlgButton(IDC_BOLD, i->getBold() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(IDC_ITALIC, i->getItalic() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(IDC_UNDERLINE, i->getUnderline() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(IDC_STRIKEOUT, i->getStrikeout() ? BST_CHECKED : BST_UNCHECKED);
			
			CheckDlgButton(IDC_INCLUDENICK, i->getIncludeNick() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(IDC_WHOLELINE, i->getWholeLine() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(IDC_CASESENSITIVE, i->getCaseSensitive() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(IDC_WHOLEWORD, i->getWholeWord() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(IDC_POPUP, i->getPopup() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(IDC_TABCOLOR, i->getTab() ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(IDC_SOUND, i->getPlaySound() ? BST_CHECKED : BST_UNCHECKED);

			bgColor = i->getBgColor();
			fgColor = i->getFgColor();

			soundFile = i->getSoundFile();
		}
	}
	
	return TRUE;
}

void HighlightPage::getValues(ColorSettings &cs) {
	char buf[1000];
	GetDlgItemText(IDC_STRING, buf, 1000);
	cs.setMatch(buf);

	cs.setBold( IsDlgButtonChecked(IDC_BOLD) );
	cs.setItalic( IsDlgButtonChecked(IDC_ITALIC) );
	cs.setUnderline( IsDlgButtonChecked(IDC_UNDERLINE) );
	cs.setStrikeout( IsDlgButtonChecked(IDC_STRIKEOUT) );

	cs.setCaseSensitive( IsDlgButtonChecked(IDC_CASESENSITIVE) );
	cs.setIncludeNick( IsDlgButtonChecked(IDC_INCLUDENICK) );
	cs.setWholeLine( IsDlgButtonChecked(IDC_WHOLELINE) );
	cs.setWholeWord( IsDlgButtonChecked(IDC_WHOLEWORD) );
	cs.setPopup( IsDlgButtonChecked(IDC_POPUP) );
	cs.setTab( IsDlgButtonChecked(IDC_TABCOLOR));

	cs.setBgColor( bgColor, bgColor != WinUtil::bgColor );
	cs.setFgColor( fgColor, fgColor != WinUtil::textColor );

	cs.setMatchType( ctrlMatchType.GetCurSel() );

	cs.setSoundFile( soundFile );
}