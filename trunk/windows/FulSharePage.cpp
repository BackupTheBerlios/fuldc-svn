#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "../client/SettingsManager.h"

#include "FulSharePage.h"
#include "WinUtil.h"

PropPage::TextItem FulSharePage::texts[] = {
	{ IDC_REFRESH_INCOMING,		ResourceManager::SETTINGS_AUTO_UPDATE_INCOMING	},
	{ IDC_REFRESH_SHARE,		ResourceManager::SETTINGS_AUTO_UPDATE_LIST		},
	{ IDC_SB_SKIPLIST_SHARE,	ResourceManager::SETTINGS_SB_SKIPLIST_SHARE		},
	{ IDC_SB_REFRESH,			ResourceManager::SETTINGS_SB_REFRESH			},
	{ IDC_ST_SKIPLIST_SHARE,	ResourceManager::SETTINGS_ST_SKIPLIST_SHARE		},
	{ IDC_ST_REFRESH_INCOMING,	ResourceManager::SETTINGS_ST_REFRESH_INCOMING	},
	{ IDC_ST_REFRESH_SHARE,		ResourceManager::SETTINGS_ST_REFRESH_SHARE		}, 
	{ IDC_ST_REFRESH_HELP,		ResourceManager::SETTINGS_ST_REFRESH_HELP		},
	{ IDC_SB_MINISLOTS,			ResourceManager::SETTINGS_SB_MINISLOTS			},
	{ IDC_ST_MINISLOTS_EXT,		ResourceManager::SETTINGS_ST_MINISLOTS_EXT		},
	{ IDC_ST_MINISLOTS_SIZE,	ResourceManager::SETTINGS_ST_MINISLOTS_SIZE		},
	{ 0,						ResourceManager::SETTINGS_AUTO_AWAY				},
};
PropPage::Item FulSharePage::items[] = {
	{ IDC_SKIPLIST_SHARE,		 SettingsManager::SKIPLIST_SHARE,		 PropPage::T_STR  },
	{ IDC_INCOMING_REFRESH_TIME, SettingsManager::INCOMING_REFRESH_TIME, PropPage::T_INT  },
	{ IDC_SHARE_REFRESH_TIME,	 SettingsManager::SHARE_REFRESH_TIME,	 PropPage::T_INT  },
	{ IDC_REFRESH_SHARE,		 SettingsManager::AUTO_UPDATE_LIST,		 PropPage::T_BOOL }, 
	{ IDC_REFRESH_INCOMING,		 SettingsManager::AUTO_UPDATE_INCOMING,  PropPage::T_BOOL },
	{ IDC_FREE_SLOTS_EXTENSIONS, SettingsManager::FREE_SLOTS_EXTENSIONS, PropPage::T_STR  }, 
	{ IDC_FREE_SLOTS_SIZE,		 SettingsManager::FREE_SLOTS_SIZE,		 PropPage::T_INT  }, 
	{ 0,						 0,										 PropPage::T_END  }
};

LRESULT FulSharePage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::read((HWND)*this, items);
	PropPage::translate((HWND)(*this), texts);
	
	BOOL b;
	onSetCheck(0, IDC_REFRESH_INCOMING, 0, b);
	onSetCheck(0, IDC_REFRESH_SHARE, 0, b);

	return TRUE;
}

LRESULT FulSharePage::onSetCheck(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	bool show = (IsDlgButtonChecked(wID) == BST_CHECKED);
	
	int id = (wID == IDC_REFRESH_SHARE ? IDC_SHARE_REFRESH_TIME : IDC_INCOMING_REFRESH_TIME);

	CButton btn;
	btn.Attach(GetDlgItem(id));
	btn.EnableWindow(show);
	btn.Detach();
			
	return TRUE;
}
void FulSharePage::write() {
	PropPage::write((HWND)*this, items);
}

LRESULT FulSharePage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_FULSHAREPAGE);
	return 0;
}

LRESULT FulSharePage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_FULSHAREPAGE);
	return 0;
}