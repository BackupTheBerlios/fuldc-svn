#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "../client/SettingsManager.h"
#include "../client/StringTokenizer.h"
#include "WinUtil.h"
#include "LineDlg.h"

#include "FulPage2.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::TextItem FulPage2::texts[] =  {
	{ IDC_SB_MINISLOTS,		 ResourceManager::SETTINGS_SB_MINISLOTS		 },
	{ IDC_SB_PATHS,			 ResourceManager::SETTINGS_SB_PATHS			 },
	{ IDC_SB_MAX_SOURCES,	 ResourceManager::SETTINGS_SB_MAX_SOURCES	 },
	{ IDC_ST_MINISLOTS_EXT,  ResourceManager::SETTINGS_ST_MINISLOTS_EXT	 },
	{ IDC_ST_MINISLOTS_SIZE, ResourceManager::SETTINGS_ST_MINISLOTS_SIZE },
	{ IDC_ST_PATHS,			 ResourceManager::SETTINGS_ST_PATHS			 },
	{ IDC_ADD,				 ResourceManager::ADD						 },
	{ IDC_REMOVE,			 ResourceManager::REMOVE					 },
	{ 0,					 ResourceManager::SETTINGS_AUTO_AWAY		 }

};

PropPage::Item FulPage2::items[] = {
	{ IDC_FREE_SLOTS_EXTENSIONS, SettingsManager::FREE_SLOTS_EXTENSIONS, PropPage::T_STR }, 
	{ IDC_FREE_SLOTS_SIZE, SettingsManager::FREE_SLOTS_SIZE, PropPage::T_INT }, 
	{ IDC_MAX_SOURCES, SettingsManager::MAX_AUTO_MATCH_SOURCES, PropPage::T_INT }, 
	{ 0, 0, PropPage::T_END }
};

LRESULT FulPage2::onInitDialog(UINT, WPARAM, LPARAM, BOOL&){
	PropPage::read((HWND)*this, items);

	download = SettingsManager::getInstance()->getDownloadPaths();

	ctrlDownload.Attach( GetDlgItem(IDC_DOWNLOAD_LIST) );

	CRect rc;
	ctrlDownload.GetClientRect(rc);
	ctrlDownload.InsertColumn( 0, CTSTRING(SETTINGS_NAME), LVCFMT_LEFT, rc.Width() / 3, 0 );
	ctrlDownload.InsertColumn( 1, CTSTRING(PATH), LVCFMT_LEFT, (rc. Width() / 3) * 2, 1);
	ctrlDownload.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	TStringList t;
	for(StringMapIter i = download.begin(); i != download.end(); ++i){
		t.push_back( Text::toT(i->first) );
		t.push_back( Text::toT(i->second) );
		ctrlDownload.insert( t );
		t.clear();
	}

	return 0;
}

LRESULT FulPage2::onAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	tstring path;
	if(WinUtil::browseDirectory(path)){
		if( path[ path.length() -1 ] != _T('\\') )
			path += _T('\\');

		bool accepted = false;
		
		//loop until the user enters an accepted name instead of aborting
		//and having the user go through the browse dialog again
		while( !accepted ){
			LineDlg dlg;
			dlg.line = Util::getLastDir( path );
			dlg.description = TSTRING(DISPLAY_NAME);
			dlg.title = TSTRING(SETTINGS_NAME);
			if( dlg.DoModal() == IDOK ){
				if( download.find( Text::fromT(dlg.line) ) != download.end() ){
					MessageBox(CTSTRING(NAME_EXISTS), _T(FULDC) _T(" ") _T(FULVERSIONSTRING), MB_OK);
				} else {
					//download[ Text::fromT(dlg.line) ] = Text::fromT(path);
					download.insert( StringPair( Text::fromT(dlg.line), Text::fromT(path) ) );
					TStringList l;
					l.push_back( dlg.line );
					l.push_back( path );
					ctrlDownload.insert( l );
					accepted = true;
				}
			} else {
				accepted = true;
			}
		}
	}

	return 0;
}

LRESULT FulPage2::onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int sel = ctrlDownload.GetSelectedIndex();

	TCHAR buf[1000];
	ctrlDownload.GetItemText(sel, 0, buf, 1000);
	string name = Text::fromT(buf);

	StringMapIter i = download.find ( name );
	if( i != download.end() )
		download.erase( i );

	ctrlDownload.DeleteItem( sel );
	
	return 0;
}

void FulPage2::write() {
	SettingsManager::getInstance()->setDownloadPaths( download );
	
	PropPage::write((HWND)*this, items);
}