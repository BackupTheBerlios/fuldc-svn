/*
* Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
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

#include "TabsPage.h"

#include "../client/SettingsManager.h"
#include "../client/FavoriteManager.h"
#include "WinUtil.h"

PropPage::Item TabsPage::items[] = {
	{ IDC_MAX_TAB_ROWS, SettingsManager::MAX_TAB_ROWS, PropPage::T_INT },
	{ 0, 0, PropPage::T_END }
};

PropPage::TextItem TabsPage::texts[] = {
	{ IDC_SETTINGS_MAX_TAB_ROWS, ResourceManager::SETTINGS_MAX_TAB_ROWS },
	{ IDC_SETTINGS_BOLD_CONTENTS, ResourceManager::SETTINGS_BOLD_OPTIONS },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::ListItem TabsPage::listItems[] = {
	{ SettingsManager::BOLD_FINISHED_DOWNLOADS, ResourceManager::FINISHED_DOWNLOADS },
	{ SettingsManager::BOLD_FINISHED_UPLOADS, ResourceManager::FINISHED_UPLOADS },
	{ SettingsManager::BOLD_QUEUE, ResourceManager::DOWNLOAD_QUEUE },
	{ SettingsManager::BOLD_HUB, ResourceManager::HUB },
	{ SettingsManager::BOLD_PM, ResourceManager::PRIVATE_MESSAGE },
	{ SettingsManager::BOLD_SEARCH, ResourceManager::SEARCH },
	{ SettingsManager::BOLD_WAITING_USERS, ResourceManager::WAITING_USERS },
	{ SettingsManager::BOLD_SYSTEM_LOG, ResourceManager::SYSTEM_LOG },
	{ SettingsManager::BOLD_TRANSFERS, ResourceManager::BOLD_TRANSFERS },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

LRESULT TabsPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::translate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items, listItems,GetDlgItem(IDC_BOLD_BOOLEANS));

	// Do specialized reading here
	return TRUE;
}

void TabsPage::write() {
	PropPage::write((HWND)*this, items, listItems,GetDlgItem(IDC_BOLD_BOOLEANS));
}

LRESULT TabsPage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_TABSPAGE);
	return 0;
}


LRESULT TabsPage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_TABSPAGE);
	return 0;
}