#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "../client/SettingsManager.h"

#include "FulPage.h"
#include "WinUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::TextItem FulPage::texts[] = {
	{ IDC_REFRESH_INCOMING,		ResourceManager::SETTINGS_AUTO_UPDATE_INCOMING	},
	{ IDC_REFRESH_SHARE,		ResourceManager::SETTINGS_AUTO_UPDATE_LIST		},
	{ IDC_SB_SKIPLIST_DOWNLOAD, ResourceManager::SETTINGS_SB_SKIPLIST_DOWNLOAD	},
	{ IDC_SB_SKIPLIST_SHARE,	ResourceManager::SETTINGS_SB_SKIPLIST_SHARE		},
	{ IDC_SB_REFRESH,			ResourceManager::SETTINGS_SB_REFRESH			},
	{ IDC_ST_SKIPLIST_DOWNLOAD, ResourceManager::SETTINGS_ST_SKIPLIST_DOWNLOAD	},
	{ IDC_ST_SKIPLIST_SHARE,	ResourceManager::SETTINGS_ST_SKIPLIST_SHARE		},
	{ IDC_ST_REFRESH_INCOMING,	ResourceManager::SETTINGS_ST_REFRESH_INCOMING	},
	{ IDC_ST_REFRESH_SHARE,		ResourceManager::SETTINGS_ST_REFRESH_SHARE		}, 
	{ IDC_ST_REFRESH_HELP,		ResourceManager::SETTINGS_ST_REFRESH_HELP		},
	{ 0,						ResourceManager::SETTINGS_AUTO_AWAY				},
};
PropPage::Item FulPage::items[] = {
	{ IDC_SKIPLIST_DOWNLOAD,	 SettingsManager::SKIPLIST_DOWNLOAD,	 PropPage::T_STR  },
	{ IDC_SKIPLIST_SHARE,		 SettingsManager::SKIPLIST_SHARE,		 PropPage::T_STR  },
	{ IDC_INCOMING_REFRESH_TIME, SettingsManager::INCOMING_REFRESH_TIME, PropPage::T_INT  },
	{ IDC_SHARE_REFRESH_TIME,	 SettingsManager::SHARE_REFRESH_TIME,	 PropPage::T_INT  },
	{ IDC_REFRESH_SHARE,		 SettingsManager::AUTO_UPDATE_LIST,		 PropPage::T_BOOL }, 
	{ IDC_REFRESH_INCOMING,		 SettingsManager::AUTO_UPDATE_INCOMING,  PropPage::T_BOOL },
	{ 0,						 0,										 PropPage::T_END  }
};

LRESULT FulPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::read((HWND)*this, items);
	PropPage::translate((HWND)(*this), texts);
	
	BOOL b;
	onSetCheck(0, IDC_REFRESH_INCOMING, 0, b);
	onSetCheck(0, IDC_REFRESH_SHARE, 0, b);

	return TRUE;
}

LRESULT FulPage::onSetCheck(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	bool show = (IsDlgButtonChecked(wID) == BST_CHECKED);
	
	int id = (wID == IDC_REFRESH_SHARE ? IDC_SHARE_REFRESH_TIME : IDC_INCOMING_REFRESH_TIME);

	CButton btn;
	btn.Attach(GetDlgItem(id));
	btn.EnableWindow(show);
	btn.Detach();
			
	return TRUE;
}
void FulPage::write() {
	PropPage::write((HWND)*this, items);
}
