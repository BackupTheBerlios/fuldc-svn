#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "../client/SettingsManager.h"

#include "FulPopupsPage.h"

PropPage::TextItem FulPopupsPage::texts[] = {
	{ IDC_POPUP_AWAY,			ResourceManager::POPUP_AWAY					},
	{ IDC_POPUP_MINIMIZED,		ResourceManager::POPUP_MINIMIZED			},
	{ IDC_POPUP_ON_PM,			ResourceManager::POPUP_ON_PM				},
	{ IDC_POPUP_ON_NEW_PM,		ResourceManager::POPUP_ON_NEW_PM			},
	{ IDC_POPUP_ON_HUBSTATUS,	ResourceManager::POPUP_ON_HUBSTATUS			},
	{ IDC_POPUP_REMOVE,			ResourceManager::REMOVE_POPUPS				},
	{ IDC_SB_POPUP,				ResourceManager::SETTINGS_SB_POPUP			},
	{ IDC_ST_DISPLAYTIME,		ResourceManager::SETTINGS_ST_DISPLAYTIME	},
	{ IDC_ST_MESSAGE_LENGTH,	ResourceManager::SETTINGS_ST_MESSAGE_LENGTH	},
	{ 0,						ResourceManager::SETTINGS_AUTO_AWAY			}
};

PropPage::Item FulPopupsPage::items[] = {
	{ IDC_POPUP_AWAY,			SettingsManager::POPUP_AWAY,			PropPage::T_BOOL },
	{ IDC_POPUP_MINIMIZED,		SettingsManager::POPUP_MINIMIZED,		PropPage::T_BOOL },
	{ IDC_POPUP_ON_PM,			SettingsManager::POPUP_ON_PM,			PropPage::T_BOOL },
	{ IDC_POPUP_ON_NEW_PM,		SettingsManager::POPUP_ON_NEW_PM,		PropPage::T_BOOL },
	{ IDC_POPUP_ON_HUBSTATUS,	SettingsManager::POPUP_ON_HUBSTATUS,	PropPage::T_BOOL },
	{ IDC_POPUP_REMOVE,			SettingsManager::REMOVE_POPUPS,			PropPage::T_BOOL },
	{ IDC_DISPLAYTIME,			SettingsManager::POPUP_TIMEOUT,			PropPage::T_INT  },
	{ IDC_MESSAGE_LENGTH,		SettingsManager::MAX_MSG_LENGTH,		PropPage::T_INT  },
	{ 0,						0,										PropPage::T_END	 }
};


LRESULT FulPopupsPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::read((HWND)*this, items);
	PropPage::translate((HWND)(*this), texts);

	return TRUE;
}

void FulPopupsPage::write() {
	PropPage::write((HWND)*this, items);
}