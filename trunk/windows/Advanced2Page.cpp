/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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

#include "Advanced2Page.h"
#include "../client/SettingsManager.h"
#include "WinUtil.h"
#include "../client/File.h"

PropPage::TextItem Advanced2Page::texts[] = {
	{ IDC_SETTINGS_LOGGING, ResourceManager::SETTINGS_LOGGING },
	{ IDC_SETTINGS_LOG_DIR, ResourceManager::DIRECTORY},
	{ IDC_BROWSE_LOG, ResourceManager::BROWSE_ACCEL },
	{ IDC_SETTINGS_FORMAT, ResourceManager::SETTINGS_FORMAT },
	{ IDC_SETTINGS_FILE_NAME, ResourceManager::SETTINGS_FILE_NAME },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item Advanced2Page::items[] = {
	{ IDC_LOG_DIRECTORY, SettingsManager::LOG_DIRECTORY, PropPage::T_STR },
	{ 0, 0, PropPage::T_END }
};

PropPage::ListItem Advanced2Page::listItems[] = {
	{ SettingsManager::LOG_MAIN_CHAT,			ResourceManager::SETTINGS_LOG_MAIN_CHAT },
	{ SettingsManager::LOG_PRIVATE_CHAT,		ResourceManager::SETTINGS_LOG_PRIVATE_CHAT },
	{ SettingsManager::LOG_DOWNLOADS,			ResourceManager::SETTINGS_LOG_DOWNLOADS }, 
	{ SettingsManager::LOG_UPLOADS,				ResourceManager::SETTINGS_LOG_UPLOADS },
	{ SettingsManager::LOG_SYSTEM,				ResourceManager::SETTINGS_LOG_SYSTEM_MESSAGES },
	{ SettingsManager::LOG_STATUS_MESSAGES,		ResourceManager::SETTINGS_LOG_STATUS_MESSAGES },
	{ 0,										ResourceManager::SETTINGS_AUTO_AWAY }
};

LRESULT Advanced2Page::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PropPage::translate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items, listItems, GetDlgItem(IDC_LOG_OPTIONS));

	int i = SettingsManager::LOG_FILE_MAIN_CHAT;

	for(int j = 0; j < 6; ++j) {
		options.push_back(Text::toT(SettingsManager::getInstance()->get(static_cast<SettingsManager::StrSetting>(i++), true)));
		options.push_back(Text::toT(SettingsManager::getInstance()->get(static_cast<SettingsManager::StrSetting>(i++), true)));
	}

	// Do specialized reading here
	return TRUE;
}

LRESULT Advanced2Page::onItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/) {
	logOptions.Attach(GetDlgItem(IDC_LOG_OPTIONS));

	TCHAR buf[512];

	if(GetDlgItemText(IDC_LOG_FILE, buf, 512) > 0)
		options[oldSelection*2] = buf;
	if(GetDlgItemText(IDC_LOG_FORMAT, buf, 512) > 0)
		options[oldSelection*2+1] = buf;
	
	int sel = logOptions.GetSelectedIndex();
	
	if(sel >= 0) {
		BOOL checkState = logOptions.GetCheckState(sel) == BST_CHECKED ? TRUE : FALSE;
		
		::EnableWindow(GetDlgItem(IDC_LOG_FORMAT), checkState);
		::EnableWindow(GetDlgItem(IDC_LOG_FILE), checkState);
		
		SetDlgItemText(IDC_LOG_FILE, options[sel*2].c_str());
		SetDlgItemText(IDC_LOG_FORMAT, options[sel*2+1].c_str());

		//save the old selection so we know where to save the values
		oldSelection = sel;
	}
	
	logOptions.Detach();
	return 0;
}

void Advanced2Page::write()
{
	PropPage::write((HWND)*this, items, listItems, GetDlgItem(IDC_LOG_OPTIONS));

	const string& s = SETTING(LOG_DIRECTORY);
	if(s.length() > 0 && s[s.length() - 1] != '\\') {
		SettingsManager::getInstance()->set(SettingsManager::LOG_DIRECTORY, s + '\\');
	}
	File::ensureDirectory(SETTING(LOG_DIRECTORY));
	// Do specialized writing here
	// settings->set(XX, YY);
	// Since the dir might change while running we don't call
	// File::ensureDirectory() here
	
	int i = SettingsManager::LOG_FILE_MAIN_CHAT;

	//make sure we save the last edit too, the user
	//might not have changed the selection
	TCHAR buf[512];

	if(GetDlgItemText(IDC_LOG_FILE, buf, 512) > 0)
		options[oldSelection*2] = buf;
	if(GetDlgItemText(IDC_LOG_FORMAT, buf, 512) > 0)
		options[oldSelection*2+1] = buf;

	for(int j = 0; j < 6; ++j) {
		string tmp = Text::fromT(options[j*2]);
		if(Util::stricmp(Util::getFileExt(tmp), ".log") != 0)
			tmp += ".log";
		
		SettingsManager::getInstance()->set(static_cast<SettingsManager::StrSetting>(i++), tmp);
		SettingsManager::getInstance()->set(static_cast<SettingsManager::StrSetting>(i++), Text::fromT(options[j*2+1]));
	}
}

LRESULT Advanced2Page::onClickedBrowseDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	tstring dir = Text::toT(SETTING(LOG_DIRECTORY));
	if(WinUtil::browseDirectory(dir, m_hWnd))
	{
		// Adjust path string
		if(dir.size() > 0 && dir[dir.size() - 1] != '\\')
			dir += '\\';
		
		SetDlgItemText(IDC_LOG_DIRECTORY, dir.c_str());
	}
	return 0;
}

LRESULT Advanced2Page::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_ADVANCED2PAGE);
	return 0;
}

LRESULT Advanced2Page::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_ADVANCED2PAGE);
	return 0;
}

/**
 * @file
 * $Id: Advanced2Page.cpp,v 1.1 2003/12/15 16:51:56 trem Exp $
 */

