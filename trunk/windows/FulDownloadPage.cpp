#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "../client/SettingsManager.h"
#include "../client/StringTokenizer.h"
#include "../client/version.h"
#include "WinUtil.h"
#include "LineDlg.h"

#include "FulDownloadPage.h"

PropPage::TextItem FulDownloadPage::texts[] =  {
	{ IDC_SB_SKIPLIST_DOWNLOAD,	ResourceManager::SETTINGS_SB_SKIPLIST_DOWNLOAD	},
	{ IDC_ST_SKIPLIST_DOWNLOAD,	ResourceManager::SETTINGS_ST_SKIPLIST_DOWNLOAD	},
	{ IDC_SB_PATHS,				ResourceManager::SETTINGS_SB_PATHS				},
	{ IDC_SB_MAX_SOURCES,		ResourceManager::SETTINGS_SB_MAX_SOURCES		},
	{ IDC_ST_PATHS,				ResourceManager::SETTINGS_ST_PATHS				},
	{ IDC_ADD,					ResourceManager::ADD							},
	{ IDC_REMOVE,				ResourceManager::REMOVE							},
	{ 0,						ResourceManager::SETTINGS_AUTO_AWAY				}

};

PropPage::Item FulDownloadPage::items[] = {
	{ IDC_MAX_SOURCES,			SettingsManager::MAX_AUTO_MATCH_SOURCES,	PropPage::T_INT }, 
	{ IDC_SKIPLIST_DOWNLOAD,	SettingsManager::SKIPLIST_DOWNLOAD,			PropPage::T_STR },
	{ 0,						0,											PropPage::T_END }
};

LRESULT FulDownloadPage::onInitDialog(UINT, WPARAM, LPARAM, BOOL&){
	PropPage::read((HWND)*this, items);
	PropPage::translate((HWND)(*this), texts);
	download = SettingsManager::getInstance()->getDownloadPaths();

	ctrlDownload.Attach( GetDlgItem(IDC_DOWNLOAD_LIST) );

	CRect rc;
	ctrlDownload.GetClientRect(rc);
	ctrlDownload.InsertColumn( 0, CTSTRING(SETTINGS_NAME), LVCFMT_LEFT, rc.Width() / 3, 0 );
	ctrlDownload.InsertColumn( 1, CTSTRING(PATH), LVCFMT_LEFT, (rc. Width() / 3) * 2, 1);
	ctrlDownload.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	TStringList t;
	for(StringPairIter i = download.begin(); i != download.end(); ++i){
		t.push_back( Text::toT(i->first) );
		t.push_back( Text::toT(i->second) );
		ctrlDownload.insert( t );
		t.clear();
	}

	return 0;
}

LRESULT FulDownloadPage::onAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
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
				if( find_if(download.begin(), download.end(), CompareFirst<string, string>(Text::fromT(dlg.line)) ) != download.end() ){
					MessageBox(CTSTRING(NAME_EXISTS), _T(FULDC) _T(" ") _T(FULVERSIONSTRING), MB_OK);
				} else {
					download.push_back(StringPair( Text::fromT(dlg.line), Text::fromT(path)));
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

LRESULT FulDownloadPage::onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int sel = ctrlDownload.GetSelectedIndex();

	TCHAR buf[1000];
	ctrlDownload.GetItemText(sel, 0, buf, 1000);
	string name = Text::fromT(buf);

	StringPairIter i = find_if(download.begin(), download.end(), CompareFirst<string, string>(name));
	if( i != download.end() )
		download.erase( i );

	ctrlDownload.DeleteItem( sel );
	
	return 0;
}

void FulDownloadPage::write() {
	SettingsManager::getInstance()->setDownloadPaths( download );
	
	PropPage::write((HWND)*this, items);
}

LRESULT FulDownloadPage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_FULDOWNLOADPAGE);
	return 0;
}

LRESULT FulDownloadPage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_FULDOWNLOADPAGE);
	return 0;
}