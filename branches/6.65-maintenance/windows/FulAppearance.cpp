/* 
* Copyright (C) 2003-2005 Pär Björklund, per.bjorklund@gmail.com
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

#include "fulappearance.h"
#include "SearchFrm.h"
#include "WinUtil.h"

PropPage::TextItem FulAppearancePage::texts[] = {
	{ IDC_SB_CHATBUFFERSIZE,  ResourceManager::SETTINGS_SB_CHATBUFFERSIZE	 },
	{ IDC_ST_CHATBUFFERSIZE,  ResourceManager::SETTINGS_ST_CHATBUFFERSIZE	 },
	{ IDC_BTN_FONT,			  ResourceManager::SETTINGS_BTN_FONT			 },
	{ IDC_BTN_TEXTCOLOR,	  ResourceManager::SETTINGS_BTN_TEXTCOLOR		 },
	{ IDC_DUPE_COLOR,		  ResourceManager::SETTINGS_BTN_COLOR			 },
	{ IDC_DUPES,			  ResourceManager::SETTINGS_DUPES				 },
	{ IDC_DUPE_DESCRIPTION,	  ResourceManager::SETTINGS_DUPE_DESCRIPTION	 },
	{ IDC_NOTTH_BOX,		  ResourceManager::SETTINGS_NOTTH_BOX			 },
	{ IDC_NOTTH,			  ResourceManager::SETTINGS_BTN_COLOR			 },
	{ IDC_NOTTH_DESCRIPTION,  ResourceManager::SETTINGS_NOTTH_DESCRIPTION	 },
	{ IDC_HISTORY,			  ResourceManager::SETTINGS_HISTORY				 },
	{ IDC_SEARCH_HISTORY,	  ResourceManager::SETTINGS_CLEAR_SEARCH_HISTORY },
	{ IDC_FILTER_HISTORY,	  ResourceManager::SETTINGS_CLEAR_FILTER_HISTORY },
	{ 0,					  ResourceManager::SETTINGS_AUTO_AWAY			 }
};

PropPage::Item FulAppearancePage::items[] = {
	{ IDC_CHATBUFFERSIZE,	SettingsManager::CHATBUFFERSIZE,	PropPage::T_INT },
	{ 0,					0,									PropPage::T_END }
};

LRESULT FulAppearancePage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::read((HWND)*this, items);
	PropPage::translate((HWND)(*this), texts);

	dupeColor = SETTING(DUPE_COLOR);
	noTTHColor = SETTING(NO_TTH_COLOR);

	return TRUE;
}

void FulAppearancePage::write() {
	PropPage::write((HWND)*this, items);

	settings->set(SettingsManager::DUPE_COLOR, static_cast<int>(dupeColor));
	settings->set(SettingsManager::NO_TTH_COLOR, static_cast<int>(noTTHColor));
}

LRESULT FulAppearancePage::onDupeColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CColorDialog c(dupeColor, CC_FULLOPEN);
	if( c.DoModal() == IDOK )
		dupeColor = c.GetColor();

	return 0;
}

LRESULT FulAppearancePage::onNoTTHColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CColorDialog c(noTTHColor, CC_FULLOPEN);
	if( c.DoModal() == IDOK )
		noTTHColor = c.GetColor();

	return 0;
}

LRESULT FulAppearancePage::onSearchHistory(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(wID == IDC_SEARCH_HISTORY) {
		SettingsManager::getInstance()->clearSearchHistory();
		SearchFrame::clearHistory(wID);
	} else {
		SettingsManager::getInstance()->clearFilterHistory();
		SearchFrame::clearHistory(wID);
	}

	return 0;
}

LRESULT FulAppearancePage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_FULAPPEARANCEPAGE);
	return 0;
}

LRESULT FulAppearancePage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_FULAPPEARANCEPAGE);
	return 0;
}