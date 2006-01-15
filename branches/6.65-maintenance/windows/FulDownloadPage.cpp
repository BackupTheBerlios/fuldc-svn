/* 
* Copyright (C) 2003-2006 Pär Björklund, per.bjorklund@gmail.com
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
#include "../client/StringTokenizer.h"
#include "../client/version.h"
#include "WinUtil.h"
#include "LineDlg.h"

#include "FulDownloadPage.h"

PropPage::TextItem FulDownloadPage::texts[] =  {
	{ IDC_SB_SKIPLIST_DOWNLOAD,	ResourceManager::SETTINGS_SB_SKIPLIST_DOWNLOAD	},
	{ IDC_ST_SKIPLIST_DOWNLOAD,	ResourceManager::SETTINGS_ST_SKIPLIST_DOWNLOAD	},
	{ IDC_SB_MAX_SOURCES,		ResourceManager::SETTINGS_SB_MAX_SOURCES		},
	{ IDC_ST_PATHS,				ResourceManager::SETTINGS_ST_PATHS				},
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

	return 0;
}

void FulDownloadPage::write() {
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