#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "../client/SettingsManager.h"

#include "FulPopupsPage.h"
#include "WinUtil.h"

PropPage::TextItem FulPopupsPage::texts[] = {
	{ IDC_POPUP_AWAY,				ResourceManager::POPUP_AWAY					},
	{ IDC_POPUP_MINIMIZED,			ResourceManager::POPUP_MINIMIZED			},
	{ IDC_POPUP_ON_PM,				ResourceManager::POPUP_ON_PM				},
	{ IDC_POPUP_ON_NEW_PM,			ResourceManager::POPUP_ON_NEW_PM			},
	{ IDC_POPUP_ON_HUBSTATUS,		ResourceManager::POPUP_ON_HUBSTATUS			},
	{ IDC_POPUP_REMOVE,				ResourceManager::REMOVE_POPUPS				},
	{ IDC_POPUP_APPEARANCE,			ResourceManager::POPUP_APPEARANCE			},
	{ IDC_ST_DISPLAYTIME,			ResourceManager::SETTINGS_ST_DISPLAYTIME	},
	{ IDC_ST_MESSAGE_LENGTH,		ResourceManager::SETTINGS_ST_MESSAGE_LENGTH	},
	{ IDC_POPUP_ACTIVATE_ON_CLICK,	ResourceManager::POPUP_ACTIVATE_ON_CLICK	},
	{ IDC_POPUP_DONT_SHOW_ACTIVE,	ResourceManager::POPUP_DONT_SHOW_ON_ACTIVE	},
	{ IDC_BTN_FONT,					ResourceManager::SETTINGS_BTN_FONT			},
	{ IDC_BTN_TEXTCOLOR,			ResourceManager::SETTINGS_BTN_TEXTCOLOR		},
	{ 0,							ResourceManager::SETTINGS_AUTO_AWAY			}
};

PropPage::Item FulPopupsPage::items[] = {
	{ IDC_POPUP_AWAY,				SettingsManager::POPUP_AWAY,				PropPage::T_BOOL },
	{ IDC_POPUP_MINIMIZED,			SettingsManager::POPUP_MINIMIZED,			PropPage::T_BOOL },
	{ IDC_POPUP_ON_PM,				SettingsManager::POPUP_ON_PM,				PropPage::T_BOOL },
	{ IDC_POPUP_ON_NEW_PM,			SettingsManager::POPUP_ON_NEW_PM,			PropPage::T_BOOL },
	{ IDC_POPUP_ON_HUBSTATUS,		SettingsManager::POPUP_ON_HUBSTATUS,		PropPage::T_BOOL },
	{ IDC_POPUP_REMOVE,				SettingsManager::REMOVE_POPUPS,				PropPage::T_BOOL },
	{ IDC_POPUP_ACTIVATE_ON_CLICK,	SettingsManager::POPUP_ACTIVATE_ON_CLICK,	PropPage::T_BOOL },
	{ IDC_POPUP_DONT_SHOW_ACTIVE,	SettingsManager::POPUP_DONT_SHOW_ON_ACTIVE, PropPage::T_BOOL },
	{ IDC_DISPLAYTIME,				SettingsManager::POPUP_TIMEOUT,				PropPage::T_INT  },
	{ IDC_MESSAGE_LENGTH,			SettingsManager::MAX_MSG_LENGTH,			PropPage::T_INT  },
	{ 0,							0,											PropPage::T_END	 }
};


LRESULT FulPopupsPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::read((HWND)*this, items);
	PropPage::translate((HWND)(*this), texts);

	return TRUE;
}

LRESULT FulPopupsPage::onTextColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CColorDialog dlg(SETTING(POPUP_TEXTCOLOR), CC_FULLOPEN);
	if(dlg.DoModal() == IDOK)
		SettingsManager::getInstance()->set(SettingsManager::POPUP_TEXTCOLOR, (int)dlg.GetColor());
	return 0;
}

LRESULT FulPopupsPage::onFont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	LOGFONT font;  
	WinUtil::decodeFont(Text::toT(SETTING(POPUP_FONT)), font);
	CFontDialog dlg(&font, CF_EFFECTS | CF_SCREENFONTS);
	if(dlg.DoModal() == IDOK){
		settings->set(SettingsManager::POPUP_FONT, Text::fromT(WinUtil::encodeFont(font)));
	}
	return 0;
}

LRESULT FulPopupsPage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_FULPOPUPPAGE);
	return 0;
}

LRESULT FulPopupsPage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_FULPOPUPPAGE);
	return 0;
}

void FulPopupsPage::write() {
	PropPage::write((HWND)*this, items);
}