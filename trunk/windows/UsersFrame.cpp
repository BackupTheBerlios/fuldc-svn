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

#include "UsersFrame.h"

#include "../client/StringTokenizer.h"
#include "LineDlg.h"

int UsersFrame::columnIndexes[] = { COLUMN_NICK, COLUMN_STATUS, COLUMN_HUB, COLUMN_SEEN, COLUMN_DESCRIPTION };
int UsersFrame::columnSizes[] = { 200, 150, 300, 125, 200 };
static ResourceManager::Strings columnNames[] = { ResourceManager::AUTO_GRANT, ResourceManager::STATUS, ResourceManager::LAST_HUB, ResourceManager::LAST_SEEN, ResourceManager::DESCRIPTION };

LRESULT UsersFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);

	ctrlUsers.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS , WS_EX_CLIENTEDGE, IDC_USERS);
	ctrlUsers.SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP | LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
	ctrlUsers.SetBkColor(WinUtil::bgColor);
	ctrlUsers.SetTextBkColor(WinUtil::bgColor);
	ctrlUsers.SetTextColor(WinUtil::textColor);
	
	// Create listview columns
	WinUtil::splitTokens(columnIndexes, SETTING(USERSFRAME_ORDER), COLUMN_LAST);
	WinUtil::splitTokens(columnSizes, SETTING(USERSFRAME_WIDTHS), COLUMN_LAST);
	
	for(int j=0; j<COLUMN_LAST; j++) {
		ctrlUsers.insertColumn(j, STRING_I(columnNames[j]), LVCFMT_LEFT, columnSizes[j], j);
	}
	
	ctrlUsers.setColumnOrderArray(COLUMN_LAST, columnIndexes);
	ctrlUsers.setSortColumn(COLUMN_NICK);
	usersMenu.CreatePopupMenu();
	appendUserItems(usersMenu);
	usersMenu.AppendMenu(MF_SEPARATOR);
	usersMenu.AppendMenu(MF_STRING, IDC_EDIT, CSTRING(PROPERTIES));
	usersMenu.AppendMenu(MF_STRING, IDC_REMOVE, CSTRING(REMOVE));

	HubManager::getInstance()->addListener(this);
	ClientManager::getInstance()->addListener(this);

	User::List ul = HubManager::getInstance()->getFavoriteUsers();
	ctrlUsers.SetRedraw(FALSE);
	for(User::Iter i = ul.begin(); i != ul.end(); ++i) {
		addUser(*i);
	}
	ctrlUsers.SetRedraw(TRUE);

	m_hMenu = WinUtil::mainMenu;

	WinUtil::SetIcon(m_hWnd, "favoriteuser.ico");

	startup = false;

	bHandled = FALSE;
	return TRUE;

}

LRESULT UsersFrame::onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ctrlUsers.forEachSelected(&UserInfo::remove);
	return 0;
}

LRESULT UsersFrame::onEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlUsers.GetSelectedCount() == 1) {
		int i = ctrlUsers.GetNextItem(-1, LVNI_SELECTED);
		UserInfo* ui = ctrlUsers.getItemData(i);
		dcassert(i != -1);
		LineDlg dlg;
		dlg.description = STRING(DESCRIPTION);
		dlg.title = ui->user->getNick();
		dlg.line = ui->user->getUserDescription();
		if(dlg.DoModal(m_hWnd)) {
			ui->user->setUserDescription(dlg.line);
			ui->update();
			ctrlUsers.updateItem(i);
			HubManager::getInstance()->save();
		}
	}
	return 0;
}

LRESULT UsersFrame::onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* l = (NMITEMACTIVATE*)pnmh;
	if(!startup && l->iItem != -1 && ((l->uNewState & LVIS_STATEIMAGEMASK) != (l->uOldState & LVIS_STATEIMAGEMASK))) {
		ctrlUsers.getItemData(l->iItem)->user->setFavoriteGrantSlot(ctrlUsers.GetCheckState(l->iItem) != FALSE);
		HubManager::getInstance()->save();
	}
	return 0;
} 

void UsersFrame::addUser(const User::Ptr& aUser) {
	int i = ctrlUsers.insertItem(new UserInfo(aUser), 0);
	bool b = aUser->getFavoriteGrantSlot();
	ctrlUsers.SetCheckState(i, b);
}

void UsersFrame::updateUser(const User::Ptr& aUser) {
	int i = -1;
	while((i = ctrlUsers.findItem(aUser->getNick(), i)) != -1) {
		UserInfo *ui = ctrlUsers.getItemData(i);
		if(ui->user == aUser) {
			ui->update();
			ctrlUsers.updateItem(i);
		}
	}
}

void UsersFrame::removeUser(const User::Ptr& aUser) {
	int i = -1;
	while((i = ctrlUsers.findItem(aUser->getNick(), i)) != -1) {
		UserInfo *ui = ctrlUsers.getItemData(i);
		if(ui->user == aUser) {
			ctrlUsers.DeleteItem(i);
			delete ui;
			return;
		}
	}
}

LRESULT UsersFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if(!closed) {
		HubManager::getInstance()->removeListener(this);
		ClientManager::getInstance()->removeListener(this);

		closed = true;
		PostMessage(WM_CLOSE);
		return 0;
	} else {
		WinUtil::saveHeaderOrder(ctrlUsers, SettingsManager::USERSFRAME_ORDER, 
			SettingsManager::USERSFRAME_WIDTHS, COLUMN_LAST, columnIndexes, columnSizes);

		for(int i = 0; i < ctrlUsers.GetItemCount(); ++i) {
			delete ctrlUsers.getItemData(i);
		}
		
		checkButton(false);
		MDIDestroy(m_hWnd);
		return 0;
	}
}

/**
 * @file
 * $Id: UsersFrame.cpp,v 1.3 2004/01/06 01:52:18 trem Exp $
 */

