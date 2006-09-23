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

#include "../client/Client.h"
#include "../client/ClientManager.h"
#include "../client/QueueManager.h"
#include "../client/FavoriteManager.h"
#include "../client/IgnoreManager.h"
#include "../client/LogManager.h"
#include "WaitingUsersFrame.h"
#include "PrivateFrame.h"

// Frame creation
LRESULT WaitingUsersFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);

	// Create tree control
	ctrlQueued.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE, 0U);

	ctrlQueued.SetBkColor(WinUtil::bgColor);
	ctrlQueued.SetTextColor(WinUtil::textColor);

	closed = false;

	// Create context menu
	contextMenu.CreatePopupMenu();
	contextMenu.AppendMenu(MF_STRING, IDC_GETLIST, CTSTRING(GET_FILE_LIST));
	contextMenu.AppendMenu(MF_STRING, IDC_PRIVATEMESSAGE, CTSTRING(SEND_PRIVATE_MESSAGE));
	contextMenu.AppendMenu(MF_STRING, IDC_MATCH_QUEUE, CTSTRING(MATCH_QUEUE));
	contextMenu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CTSTRING(ADD_TO_FAVORITES));
	contextMenu.AppendMenu(MF_STRING, IDC_GRANTSLOT, CTSTRING(GRANT_EXTRA_SLOT));
	contextMenu.AppendMenu(MF_STRING, IDC_REMOVEALL, CTSTRING(REMOVE_FROM_ALL));
	contextMenu.AppendMenu(MF_STRING, IDC_SHOWLOG, CTSTRING(SHOW_LOG));
	contextMenu.AppendMenu(MF_STRING, IDC_COPY_FILENAME, CTSTRING(COPY_FILENAME));
	contextMenu.AppendMenu(MF_SEPARATOR);
	contextMenu.AppendMenu(MF_STRING, IDC_IGNORE, CTSTRING(IGNOREA));
	contextMenu.AppendMenu(MF_STRING, IDC_UNIGNORE, CTSTRING(UNIGNORE));
	contextMenu.AppendMenu(MF_SEPARATOR);
	contextMenu.AppendMenu(MF_STRING, IDC_REMOVE, CTSTRING(REMOVE));


	// Load all waiting users & files.
	LoadAll();

	UpdateLayout(TRUE);
	updateStatus();

	WinUtil::SetIcon(m_hWnd, _T("wuicon.ico"));

	UploadManager::getInstance()->addListener(this);

	bHandled = FALSE;
	return TRUE;
}

LRESULT WaitingUsersFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) 
{
	if (!closed) {
		UploadManager::getInstance()->removeListener(this);

		closed = true;
		PostMessage(WM_CLOSE);
		return 0;
	} else {
		HTREEITEM userNode = ctrlQueued.GetRootItem();

		while (userNode) {
			delete reinterpret_cast<UserPtr *>(ctrlQueued.GetItemData(userNode));
			userNode = ctrlQueued.GetNextSiblingItem(userNode);
		}

		checkButton(false);
		frame = NULL;

		bHandled = FALSE;
		return 0;
	}
}

// Recalculate frame control layout
void WaitingUsersFrame::UpdateLayout(BOOL bResizeBars) {
	RECT rect;
	GetClientRect(&rect);

	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);

	if(ctrlStatus.IsWindow()) {
		CRect sr;
		int w[3];
		ctrlStatus.GetClientRect(sr);
		w[2] = sr.right - 16;
		w[1] = max(w[2] - 100, 0);
		w[0] = max(w[1] - 140, 0);

		ctrlStatus.SetParts(3, w);
	}

	// Position tree control
	CRect rc(rect);
//	rc.top += 1;
//	rc.bottom -= 2;
	ctrlQueued.MoveWindow(rc);
}

void WaitingUsersFrame::updateStatus() {
	ctrlStatus.SetText(1, (TSTRING(STATUS_FILES) + Util::toStringW(UploadManager::getInstance()->getWaitingUserFileCount())).c_str());
	ctrlStatus.SetText(2, (TSTRING(STATUS_USERS) + Util::toStringW(UploadManager::getInstance()->getWaitingUserCount())).c_str());
}

// Keyboard shortcuts
LRESULT WaitingUsersFrame::onChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	switch(wParam)
	{
	case VK_DELETE:
		onRemove(0, 0, 0, bHandled);
		break;
	default:
		bHandled = FALSE;
	}
	return 0;
}

LRESULT WaitingUsersFrame::onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	User::Ptr user = getSelectedUser();
	if (user) {
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	}
	return 0;
}

LRESULT WaitingUsersFrame::onCopyFilename(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HTREEITEM selectedItem = ctrlQueued.GetSelectedItem(), parentItem = ctrlQueued.GetParentItem(selectedItem);

	if (!selectedItem || !parentItem || selectedItem == parentItem)
		return 0;
	TCHAR filenameBuf[256];
	ctrlQueued.GetItemText(selectedItem, filenameBuf, 255);
	*_tcschr(filenameBuf, _T('(')) = NULL;
	tstring tmpstr(filenameBuf);
	if(!tmpstr.empty()) {
		// remove last space
		tmpstr.erase(tmpstr.length() - 1);
		WinUtil::setClipboard(tmpstr);
	}
	return 0;
}

// Remove queued item
LRESULT WaitingUsersFrame::onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	User::Ptr user = getSelectedUser();
	if (user) {
		UploadManager::getInstance()->clearUserFiles(user);
	}
	return 0;
}

LRESULT WaitingUsersFrame::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	// Get the bounding rectangle of the client area.
	RECT rc;
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	ctrlQueued.GetClientRect(&rc);
	ctrlQueued.ScreenToClient(&pt);

	// Change selected item
	HTREEITEM item = ctrlQueued.HitTest(pt, NULL);
	if (item == NULL) return FALSE;
	ctrlQueued.SelectItem(item);

	// Hit-test
	if(PtInRect(&rc, pt))
	{
		User::Ptr user = getSelectedUser();
		if( user ) {
			if(IgnoreManager::getInstance()->isUserIgnored(user->getNick())) {
				contextMenu.EnableMenuItem(IDC_IGNORE, MF_GRAYED);
				contextMenu.EnableMenuItem(IDC_UNIGNORE, MF_ENABLED);
			} else {
				contextMenu.EnableMenuItem(IDC_IGNORE, MF_ENABLED);
				contextMenu.EnableMenuItem(IDC_UNIGNORE, MF_GRAYED);
			}
		} else {
			contextMenu.EnableMenuItem(IDC_IGNORE, MF_GRAYED);
			contextMenu.EnableMenuItem(IDC_UNIGNORE, MF_GRAYED);
		}

		ctrlQueued.ClientToScreen(&pt);
		contextMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		return TRUE;
	}

	return FALSE;
}

LRESULT WaitingUsersFrame::onPrivateMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	User::Ptr user = getSelectedUser();
	if (user) {
		PrivateFrame::openWindow(user);
	}
	return 0;
}

LRESULT WaitingUsersFrame::onGrantSlot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	User::Ptr user = getSelectedUser();
	if (user) {
		UploadManager::getInstance()->reserveSlot(user);
	}
	return 0;
}

LRESULT WaitingUsersFrame::onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	User::Ptr user = getSelectedUser();
	if (user) {
		FavoriteManager::getInstance()->addFavoriteUser(user);
	}
	return 0;
};

LRESULT WaitingUsersFrame::onMatchQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	User::Ptr user = getSelectedUser();
	if (user) {
		try {
			QueueManager::getInstance()->addList(user, QueueItem::FLAG_MATCH_QUEUE);
		} catch(const Exception&) { }
	}
	return 0;
}

LRESULT WaitingUsersFrame::onRemoveAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	User::Ptr user = getSelectedUser();
	if (user) {
		try {
			QueueManager::getInstance()->removeUserFromQueue(user, QueueItem::Source::FLAG_REMOVED);
		} catch(const Exception&) { }
	}
	return 0;
}

LRESULT WaitingUsersFrame::onIgnore(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	User::Ptr user = getSelectedUser();
	if (user) {
			IgnoreManager::getInstance()->ignore(user->getNick());
	}
	return 0;
}

LRESULT WaitingUsersFrame::onUnIgnore(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	User::Ptr user = getSelectedUser();
	if (user) {
		IgnoreManager::getInstance()->unignore(user->getNick());
	}
	return 0;
}

LRESULT WaitingUsersFrame::onShowLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	User::Ptr user = getSelectedUser();
	if (user) {
		StringMap params;
		params["user"] = user->getNick();
		params["hub"] = user->getClientName();
		params["hubaddr"] = user->getClientUrl();
		params["mynick"] = user->getClientNick(); 

		tstring path = Text::toT(LogManager::getInstance()->getLogFilename(LogManager::PM, params));
		if(!path.empty())
			ShellExecute(NULL, _T("open"), Util::validateFileName(path).c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
	return 0;
}

// Load all searches from manager
void WaitingUsersFrame::LoadAll()
{
	// Load queue
	typedef vector<User::Ptr> UserVect;
	UserVect users = UploadManager::getInstance()->getWaitingUsers();
	for (UserVect::const_iterator uit = users.begin(); uit != users.end(); ++uit) {
		HTREEITEM lastInserted = ctrlQueued.InsertItem(TVIF_PARAM | TVIF_TEXT,
			Text::toT((*uit)->getNick() + " - " + (*uit)->getClientName()).c_str(),
			0, 0, 0, 0, (LPARAM)(new UserPtr(*uit)), TVI_ROOT, TVI_LAST);
		UploadManager::FileSet files = UploadManager::getInstance()->getWaitingUserFiles(*uit);
		for (UploadManager::FileSet::const_iterator fit = files.begin(); fit != files.end(); ++fit) {
			ctrlQueued.InsertItem(Text::toT(*fit).c_str(), lastInserted, TVI_LAST);
		}
	}
}

// UploadManagerListener
void WaitingUsersFrame::on(UploadManagerListener::WaitingRemoveUser, const User::Ptr aUser) throw() {
	PostMessage(WM_SPEAKER, SPEAK_REMOVE_USER, (LPARAM)new UserPtr(aUser));
}

void WaitingUsersFrame::on(UploadManagerListener::WaitingAddFile, const User::Ptr aUser, const string& aFilename) throw() {
	PostMessage(WM_SPEAKER, SPEAK_ADD_FILE, (LPARAM)new pair<User::Ptr, string>(aUser, aFilename));
}

void WaitingUsersFrame::onRemoveUser(const User::Ptr aUser) {
	HTREEITEM userNode = ctrlQueued.GetRootItem();

	while (userNode) {
		UserPtr *u = reinterpret_cast<UserPtr *>(ctrlQueued.GetItemData(userNode));
		if (aUser == u->u) {
			delete u;
			ctrlQueued.DeleteItem(userNode);
			return;
		}
		userNode = ctrlQueued.GetNextSiblingItem(userNode);
	}
}

void WaitingUsersFrame::onAddFile(const User::Ptr aUser, const string& aFile) {
	HTREEITEM userNode = ctrlQueued.GetRootItem();

	while (userNode) {
		if (aUser == reinterpret_cast<UserPtr *>(ctrlQueued.GetItemData(userNode))->u) {
			HTREEITEM childNode = ctrlQueued.GetChildItem(userNode);
			while (childNode) {
				TCHAR nickBuf[256];
				ctrlQueued.GetItemText(childNode, nickBuf, 255);
				if (aFile.substr(0, aFile.find(_T('('))) ==
					Text::fromT(tstring(nickBuf).substr(0, tstring(nickBuf).find(_T('('))))) {
						delete reinterpret_cast<UserPtr *>(ctrlQueued.GetItemData(childNode));
						ctrlQueued.DeleteItem(childNode);
						break;
					}
					childNode = ctrlQueued.GetNextSiblingItem(childNode);
			}

			//file isn't already listed, add it
			ctrlQueued.InsertItem(TVIF_PARAM | TVIF_TEXT, Text::toT(aFile).c_str(), 0,
				0, 0, 0, (LPARAM)new UserPtr(aUser), userNode, TVI_LAST);

			return;
		}

		userNode = ctrlQueued.GetNextSiblingItem(userNode);
	}

	string aNick = aUser->getNick();
	userNode = ctrlQueued.InsertItem(TVIF_PARAM | TVIF_TEXT, Text::toT(aUser->getNick() + " - " + aUser->getClientName()).c_str(), 
		0, 0, 0, 0, (LPARAM)new UserPtr(aUser),	TVI_ROOT, TVI_LAST);
	ctrlQueued.InsertItem(Text::toT(aFile).c_str(), userNode, TVI_LAST);
	ctrlQueued.Expand(userNode);
}

HTREEITEM WaitingUsersFrame::GetParentItem() {
	HTREEITEM item = ctrlQueued.GetSelectedItem();
	HTREEITEM parent = ctrlQueued.GetParentItem(item);
	return parent ? parent : item;
}

LRESULT WaitingUsersFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	//disable drawing here to avoid the flicker while updating the tree
	ctrlQueued.SetRedraw(FALSE);
	if(wParam == SPEAK_ADD_FILE) {
		const pair<User::Ptr, string> *p = (pair<User::Ptr, string> *)lParam;
		onAddFile(p->first, p->second);
		delete p;
		if(BOOLSETTING(BOLD_WAITING_USERS))
			setDirty();
		updateStatus();
	} else if(wParam == SPEAK_REMOVE_USER) {
		onRemoveUser(reinterpret_cast<UserPtr *>(lParam)->u);
		delete reinterpret_cast<UserPtr *>(lParam);
		if(BOOLSETTING(BOLD_WAITING_USERS))
			setDirty();
		updateStatus();
	}
	ctrlQueued.SetRedraw(TRUE);

	return 0;
}
