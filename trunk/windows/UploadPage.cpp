/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

#include "UploadPage.h"
#include "WinUtil.h"

#include "../client/Util.h"
#include "../client/ShareManager.h"
#include "../client/SettingsManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PropPage::TextItem UploadPage::texts[] = {
	{ IDC_SETTINGS_SHARED_DIRECTORIES, ResourceManager::SETTINGS_SHARED_DIRECTORIES },
	{ IDC_SETTINGS_SHARE_SIZE, ResourceManager::SETTINGS_SHARE_SIZE }, 
	{ IDC_SHAREHIDDEN, ResourceManager::SETTINGS_SHARE_HIDDEN },
	{ IDC_REMOVE, ResourceManager::REMOVE },
	{ IDC_ADD, ResourceManager::SETTINGS_ADD_FOLDER },
	{ IDC_SETTINGS_UPLOADS_MIN_SPEED, ResourceManager::SETTINGS_UPLOADS_MIN_SPEED },
	{ IDC_SETTINGS_KBPS, ResourceManager::KBPS }, 
	{ IDC_SETTINGS_UPLOADS_SLOTS, ResourceManager::SETTINGS_UPLOADS_SLOTS },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item UploadPage::items[] = {
	{ IDC_SLOTS, SettingsManager::SLOTS, PropPage::T_INT }, 
	{ IDC_SHAREHIDDEN, SettingsManager::SHARE_HIDDEN, PropPage::T_BOOL },
	{ IDC_MIN_UPLOAD_SPEED, SettingsManager::MIN_UPLOAD_SPEED, PropPage::T_INT },
	{ 0, 0, PropPage::T_END }
};

LRESULT UploadPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	LVITEM item;

	// Get the text (directory name) of the item that just was (un)checked
	::ZeroMemory(&item, sizeof(item));
	item.mask = LVIS_STATEIMAGEMASK;

	PropPage::translate((HWND)(*this), texts);
	ctrlDirectories.Attach(GetDlgItem(IDC_DIRECTORIES));

	if(BOOLSETTING(FULL_ROW_SELECT)) {
		ctrlDirectories.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
	}
	// Enable checkboxes
	ctrlDirectories.SetExtendedListViewStyle(LVS_EX_CHECKBOXES);
		
	ctrlTotal.Attach(GetDlgItem(IDC_TOTAL));

	PropPage::read((HWND)*this, items);

	// Prepare shared dir list
	ctrlDirectories.InsertColumn(0, CSTRING(DIRECTORY), LVCFMT_LEFT, 277, 0);
	ctrlDirectories.InsertColumn(1, CSTRING(SIZE), LVCFMT_RIGHT, 90, 1);
	
	blnInitializing = true;

	StringList directories = ShareManager::getInstance()->getDirectories();
	for(StringIter j = directories.begin(); j != directories.end(); j++)
	{
		int i = ctrlDirectories.insert(ctrlDirectories.GetItemCount(), *j);
		ctrlDirectories.SetItemText(i, 1, Util::formatBytes(ShareManager::getInstance()->getShareSize(*j)).c_str());
		//dcdebug("init row %d = %s\n", i, ShareManager::getInstance()->IsIncomingDir(*j) ? "true" : "false");
		ListView_SetCheckState(ctrlDirectories, i, ShareManager::getInstance()->isIncoming(*j));
	}
	blnInitializing = false;
	
	ctrlTotal.SetWindowText(Util::formatBytes(ShareManager::getInstance()->getShareSize()).c_str());

	CUpDownCtrl updown;
	updown.Attach(GetDlgItem(IDC_SLOTSPIN));
	updown.SetRange(1, 100);
	updown.Detach();
	updown.Attach(GetDlgItem(IDC_MIN_UPLOAD_SPIN));
	updown.SetRange32(0, 30000);
	return TRUE;
}

void UploadPage::write()
{
	PropPage::write((HWND)*this, items);

	if(SETTING(SLOTS) < 1)
		settings->set(SettingsManager::SLOTS, 1);

	// Do specialized writing here
	ShareManager::getInstance()->refresh();
}

LRESULT UploadPage::onItemchangedDirectories(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	char buf[MAX_PATH];
	LVITEM item;

	NM_LISTVIEW* lv = (NM_LISTVIEW*) pnmh;
	
	// Set default item
	if (lv->uNewState & LVIS_FOCUSED) {
		::EnableWindow(GetDlgItem(IDC_REMOVE), TRUE);
	}

	// Are we initialzing?
	if (!blnInitializing) {
		// No. Check checkbox status
		if ((lv->uChanged & LVIF_STATE) != 0) {
			// Have the checkbox state changed?
			if ((((lv->uNewState & INDEXTOSTATEIMAGEMASK(2)) != 0) && ((lv->uOldState & INDEXTOSTATEIMAGEMASK(1)) != 0)) || (((lv->uNewState & INDEXTOSTATEIMAGEMASK(1)) != 0) && ((lv->uOldState & INDEXTOSTATEIMAGEMASK(2)) != 0))) {
				// Item was just checked or unchecked
				bool bSelected = ((lv->uNewState & INDEXTOSTATEIMAGEMASK(2)) != 0);
				dcdebug("bSelected: %s\n", (bSelected ? "true" : "false"));
				// Extract filename
				::ZeroMemory(&item, sizeof(item));
				item.mask = LVIF_TEXT | LVIS_STATEIMAGEMASK;
				item.cchTextMax = sizeof(buf);
				item.pszText = buf;
				item.iItem = lv->iItem;
				if (ctrlDirectories.GetItem(&item)) {
					// Set the incoming directory or not
					ShareManager::getInstance()->setIncoming(buf, bSelected);
				}
			}
		}
	}
	return 0;		
}

LRESULT UploadPage::onClickedAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	string target;
	if(WinUtil::browseDirectory(target, (HWND) *this)) {
		try {
			// Remove trailing \ if exists
			if (target[target.size() - 1] == '\\') {
				target = target.substr(0, target.size() - 1);
			}
			ShareManager::getInstance()->addDirectory(target);
			int i = ctrlDirectories.insert(ctrlDirectories.GetItemCount(), target);
			ctrlDirectories.SetItemText(i, 1, Util::formatBytes(ShareManager::getInstance()->getShareSize(target)).c_str());
			ctrlTotal.SetWindowText(Util::formatBytes(ShareManager::getInstance()->getShareSize()).c_str());
		} catch(const ShareException& e) {
			MessageBox(e.getError().c_str(), APPNAME " " VERSIONSTRING, MB_ICONSTOP | MB_OK);
		}
	}
	
	return 0;
}

LRESULT UploadPage::onClickedRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	char buf[MAX_PATH];
	LVITEM item;
	::ZeroMemory(&item, sizeof(item));
	item.mask = LVIF_TEXT;
	item.cchTextMax = sizeof(buf);
	item.pszText = buf;
	if(ctrlDirectories.GetSelectedItem(&item)) {
		ShareManager::getInstance()->removeDirectory(buf);
		ShareManager::getInstance()->removeIncoming(buf);
		ctrlTotal.SetWindowText(Util::formatBytes(ShareManager::getInstance()->getShareSize()).c_str());
		ctrlDirectories.DeleteItem(item.iItem);
	}
	
	return 0;
}

LRESULT UploadPage::onClickedShareHidden(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// Save the checkbox state so that ShareManager knows to include/disclude hidden files
	Item i = items[1]; // The checkbox. Explicit index used - bad!
	if(::IsDlgButtonChecked((HWND)* this, i.itemID) == BST_CHECKED){
		settings->set((SettingsManager::IntSetting)i.setting, true);
	} else {
		settings->set((SettingsManager::IntSetting)i.setting, false);
	}

	// Refresh the share. This is a blocking refresh. Might cause problems?
	// Hopefully people won't click the checkbox enough for it to be an issue. :-)
	ShareManager::getInstance()->setDirty();
	ShareManager::getInstance()->refresh(true, false, true);

	// Clear the GUI list, for insertion of updated shares
	ctrlDirectories.DeleteAllItems();
	StringList directories = ShareManager::getInstance()->getDirectories();
	for(StringIter j = directories.begin(); j != directories.end(); j++)
	{
		int i = ctrlDirectories.insert(ctrlDirectories.GetItemCount(), *j);
		ctrlDirectories.SetItemText(i, 1, Util::formatBytes(ShareManager::getInstance()->getShareSize(*j)).c_str());
		ListView_SetCheckState(ctrlDirectories, i, ShareManager::getInstance()->isIncoming(*j));
	}

	// Display the new total share size
	ctrlTotal.SetWindowText(Util::formatBytes(ShareManager::getInstance()->getShareSize()).c_str());
	return 0;
}

/**
 * @file
 * $Id: UploadPage.cpp,v 1.5 2004/02/15 01:21:43 trem Exp $
 */

