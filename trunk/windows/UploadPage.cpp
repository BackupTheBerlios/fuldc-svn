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

#include "UploadPage.h"
#include "WinUtil.h"
#include "HashProgressDlg.h"
#include "LineDlg.h"

#include "../client/Util.h"
#include "../client/ShareManager.h"
#include "../client/SettingsManager.h"

PropPage::TextItem UploadPage::texts[] = {
	{ IDC_SETTINGS_SHARED_DIRECTORIES, ResourceManager::SETTINGS_SHARED_DIRECTORIES },
	{ IDC_SETTINGS_SHARE_SIZE, ResourceManager::SETTINGS_SHARE_SIZE },
	{ IDC_SHAREHIDDEN, ResourceManager::SETTINGS_SHARE_HIDDEN },
	{ IDC_REMOVE, ResourceManager::REMOVE },
	{ IDC_ADD, ResourceManager::SETTINGS_ADD_FOLDER },
	{ IDC_RENAME, ResourceManager::SETTINGS_RENAME_FOLDER },
	{ IDC_SETTINGS_UPLOADS_MIN_SPEED, ResourceManager::SETTINGS_UPLOADS_MIN_SPEED },
	{ IDC_SETTINGS_KBPS, ResourceManager::KBPS },
	{ IDC_SETTINGS_UPLOADS_SLOTS, ResourceManager::SETTINGS_UPLOADS_SLOTS },
	{ IDC_SETTINGS_ONLY_HASHED, ResourceManager::SETTINGS_ONLY_HASHED },
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
	PropPage::translate((HWND)(*this), texts);
	ctrlDirectories.Attach(GetDlgItem(IDC_DIRECTORIES));
	ctrlDirectories.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
		
	ctrlTotal.Attach(GetDlgItem(IDC_TOTAL));

	PropPage::read((HWND)*this, items);

	// Prepare shared dir list
	ctrlDirectories.InsertColumn(0, CTSTRING(VIRTUAL_NAME), LVCFMT_LEFT, 80, 0);
	ctrlDirectories.InsertColumn(1, CTSTRING(DIRECTORY), LVCFMT_LEFT, 197, 1);
	ctrlDirectories.InsertColumn(2, CTSTRING(SIZE), LVCFMT_RIGHT, 90, 2);
	StringPairList directories = ShareManager::getInstance()->getDirectories();
	for(StringPairIter j = directories.begin(); j != directories.end(); j++)
	{
		int i = ctrlDirectories.insert(ctrlDirectories.GetItemCount(), Text::toT(j->first));
		ctrlDirectories.SetItemText(i, 1, Text::toT(j->second).c_str() );
		ctrlDirectories.SetItemText(i, 2, Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize(j->second))).c_str());
		ctrlDirectories.SetCheckState(i, ShareManager::getInstance()->isIncoming(j->second) );
	}
	
	ctrlTotal.SetWindowText(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize())).c_str());

	CUpDownCtrl updown;
	updown.Attach(GetDlgItem(IDC_SLOTSPIN));
	updown.SetRange(1, 100);
	updown.Detach();
	updown.Attach(GetDlgItem(IDC_MIN_UPLOAD_SPIN));
	updown.SetRange32(0, 30000);
	return TRUE;
}

LRESULT UploadPage::onDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/){
	HDROP drop = (HDROP)wParam;
	TCHAR buf[MAX_PATH];
	UINT nrFiles;
	
	nrFiles = DragQueryFile(drop, (UINT)-1, NULL, 0);
	
	for(UINT i = 0; i < nrFiles; ++i){
		if(DragQueryFile(drop, i, buf, MAX_PATH)){
			if(PathIsDirectory(buf))
				addDirectory(buf);
		}
	}

	DragFinish(drop);

	return 0;
}

void UploadPage::write()
{
	PropPage::write((HWND)*this, items);

	if(SETTING(SLOTS) < 1)
		settings->set(SettingsManager::SLOTS, 1);

	int size = ctrlDirectories.GetItemCount();
	TCHAR buf[MAX_PATH];

	for(int i = 0; i < size; ++i){
		ctrlDirectories.GetItemText(i, 1, buf, MAX_PATH);
		string tmp = Text::fromT(buf);
		ShareManager::getInstance()->setIncoming( tmp, ctrlDirectories.GetCheckState(i) ? true : false );
		
	}

	// Do specialized writing here
	ShareManager::getInstance()->refresh();
}

LRESULT UploadPage::onItemchangedDirectories(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	NM_LISTVIEW* lv = (NM_LISTVIEW*) pnmh;
	::EnableWindow(GetDlgItem(IDC_REMOVE), (lv->uNewState & LVIS_FOCUSED));
	::EnableWindow(GetDlgItem(IDC_RENAME), (lv->uNewState & LVIS_FOCUSED));
	return 0;		
}

LRESULT UploadPage::onClickedAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	tstring target;
	if(WinUtil::browseDirectory(target, m_hWnd)) {
		addDirectory(target);
	}
	
	return 0;
}

LRESULT UploadPage::onClickedRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR buf[MAX_PATH];
	LVITEM item;
	::ZeroMemory(&item, sizeof(item));
	item.mask = LVIF_TEXT;
	item.cchTextMax = sizeof(buf);
	item.pszText = buf;

	int i = -1;
	while((i = ctrlDirectories.GetNextItem(-1, LVNI_SELECTED)) != -1) {
		item.iItem = i;
		item.iSubItem = 1;
		ctrlDirectories.GetItem(&item);
		string t = Text::fromT(buf);
		ShareManager::getInstance()->removeDirectory( t );
		HashManager::getInstance()->stopHashing( t );
		ctrlTotal.SetWindowText(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize())).c_str());
		ctrlDirectories.DeleteItem(i);
	}
	
	return 0;
}

LRESULT UploadPage::onClickedRename(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR buf[MAX_PATH];
	LVITEM item;
	::ZeroMemory(&item, sizeof(item));
	item.mask = LVIF_TEXT;
	item.cchTextMax = sizeof(buf);
	item.pszText = buf;

	bool setDirty = false;

	int i = -1;
	while((i = ctrlDirectories.GetNextItem(i, LVNI_SELECTED)) != -1) {
		item.iItem = i;
		item.iSubItem = 0;
		ctrlDirectories.GetItem(&item);

		try {
			LineDlg virt;
			virt.title = TSTRING(VIRTUAL_NAME);
			virt.description = TSTRING(VIRTUAL_NAME_LONG);
			virt.line = tstring(buf);
			if(virt.DoModal(m_hWnd) == IDOK) {
				if (Util::stricmp(buf, virt.line) != 0) {
					item.iSubItem = 1;
					ctrlDirectories.GetItem(&item);

					ShareManager::getInstance()->renameDirectory(Text::fromT(buf), Text::fromT(virt.line));
					ctrlDirectories.SetItemText(i, 0, virt.line.c_str());

					setDirty = true;
				} else {
					MessageBox(CTSTRING(SKIP_RENAME), _T(FULDC) _T(" ") _T(FULVERSIONSTRING), MB_ICONINFORMATION | MB_OK);
				}
			}
		} catch(const ShareException& e) {
			MessageBox(Text::toT(e.getError()).c_str(), _T(FULDC) _T(" ") _T(FULVERSIONSTRING), MB_ICONSTOP | MB_OK);
		}
	}

	if( setDirty )
		ShareManager::getInstance()->setDirty();

	return 0;
}

LRESULT UploadPage::onClickedShareHidden(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// Save the checkbox state so that ShareManager knows to include/exclude hidden files
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
	StringPairList directories = ShareManager::getInstance()->getDirectories();
	for(StringPairIter j = directories.begin(); j != directories.end(); j++)
	{
		int i = ctrlDirectories.insert(ctrlDirectories.GetItemCount(), Text::toT(j->first));
		ctrlDirectories.SetItemText(i, 1, Text::toT(j->second).c_str() );
		ctrlDirectories.SetItemText(i, 2, Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize(j->second))).c_str());
	}

	// Display the new total share size
	ctrlTotal.SetWindowText(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize())).c_str());
	return 0;
}

LRESULT UploadPage::onHelpInfo(LPNMHDR /*pnmh*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_UPLOADPAGE);
	return 0;
}

LRESULT UploadPage::onHelp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HtmlHelp(m_hWnd, WinUtil::getHelpFile().c_str(), HH_HELP_CONTEXT, IDD_UPLOADPAGE);
	return 0;
}

void UploadPage::addDirectory(const tstring& aPath){
	tstring path = aPath;
	if( path[ path.length() -1 ] != _T('\\') )
		path += _T('\\');

	try {
		LineDlg virt;
		virt.title = TSTRING(VIRTUAL_NAME);
		virt.description = TSTRING(VIRTUAL_NAME_LONG);
		virt.line = Text::toT(ShareManager::getInstance()->validateVirtual(
			Util::getLastDir(Text::fromT(path))));
		if(virt.DoModal(m_hWnd) == IDOK) {
			ShareManager::getInstance()->addDirectory(Text::fromT(path), Text::fromT(virt.line));
			ShareManager::getInstance()->setIncoming(Text::fromT(path), false);
			int i = ctrlDirectories.insert(ctrlDirectories.GetItemCount(), virt.line );
			ctrlDirectories.SetItemText(i, 1, path.c_str());
			ctrlDirectories.SetItemText(i, 2, Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize(Text::fromT(path)))).c_str());
			ctrlTotal.SetWindowText(Text::toT(Util::formatBytes(ShareManager::getInstance()->getShareSize())).c_str());
		}
	} catch(const ShareException& e) {
		MessageBox(Text::toT(e.getError()).c_str(), _T(FULDC) _T(" ") _T(FULVERSIONSTRING), MB_ICONSTOP | MB_OK);
	}

	::PostMessage( WinUtil::mainWnd, WM_COMMAND, IDC_HASH_PROGRESS, 0);
}

/**
 * @file
 * $Id: UploadPage.cpp,v 1.5 2004/02/15 01:21:43 trem Exp $
 */

