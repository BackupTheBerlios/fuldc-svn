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

#if !defined(USERS_FRAME_H)
#define USERS_FRAME_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlatTabCtrl.h"
#include "TypedListViewCtrl.h"
#include "WinUtil.h"

#include "../client/ClientManager.h"
#include "../client/FavoriteManager.h"

class UsersFrame : public MDITabChildWindowImpl<UsersFrame>, public StaticFrame<UsersFrame, ResourceManager::FAVORITE_USERS, IDC_FAVUSERS>,
	private FavoriteManagerListener, private ClientManagerListener, public UserInfoBaseHandler<UsersFrame> {
public:

	UsersFrame() : closed(false), startup(true) { }
	virtual ~UsersFrame() { }

	DECLARE_FRAME_WND_CLASS_EX(_T("UsersFrame"), IDR_USERS, 0, COLOR_3DFACE);

	typedef MDITabChildWindowImpl<UsersFrame> baseClass;
	typedef UserInfoBaseHandler<UsersFrame> uibBase;

	BEGIN_MSG_MAP(UsersFrame)
		NOTIFY_HANDLER(IDC_USERS, LVN_GETDISPINFO, ctrlUsers.onGetDispInfo)
		NOTIFY_HANDLER(IDC_USERS, LVN_COLUMNCLICK, ctrlUsers.onColumnClick)
		NOTIFY_HANDLER(IDC_USERS, LVN_ITEMCHANGED, onItemChanged)
		NOTIFY_HANDLER(IDC_USERS, LVN_KEYDOWN, onKeyDown)
		NOTIFY_HANDLER(IDC_USERS, NM_DBLCLK, onDoubleClick)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_SETFOCUS, onSetFocus)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDC_EDIT, onEdit)
		NOTIFY_HANDLER(IDC_USERS, LVN_KEYDOWN, onKeyDown)
		CHAIN_MSG_MAP(uibBase)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onDoubleClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);

	void UpdateLayout(BOOL bResizeBars = TRUE);

	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
		if(wParam == USER_ADDED) {
			addUser(((UserInfoBase*)lParam)->user);
			delete (UserInfoBase*)lParam;
		} else if(wParam == USER_UPDATED) {
			updateUser(((UserInfoBase*)lParam)->user);
			delete (UserInfoBase*)lParam;
		} else if(wParam == USER_REMOVED) {
			removeUser(((UserInfoBase*)lParam)->user);
			delete (UserInfoBase*)lParam;
		}
		return 0;
	}

	LRESULT onSetFocus(UINT /* uMsg */, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlUsers.SetFocus();
		return 0;
	}

private:
	class UserInfo;
public:
	TypedListViewCtrl<UserInfo, IDC_USERS>& getUserList() { return ctrlUsers; }
private:
	enum {
		COLUMN_FIRST,
		COLUMN_NICK = COLUMN_FIRST,
		COLUMN_STATUS,
		COLUMN_HUB,
		COLUMN_SEEN,
		COLUMN_DESCRIPTION,
		COLUMN_LAST
	};

	enum {
		USER_ADDED,
		USER_UPDATED,
		USER_REMOVED,
	};

	class UserInfo : public UserInfoBase {
	public:
		UserInfo(const User::Ptr& u) : UserInfoBase(u) { 
			columns[COLUMN_NICK] = Text::toT(u->getNick());
			update();
		}

		const tstring& getText(int col) const {
			return columns[col];
		}

		static int compareItems(UserInfo* a, UserInfo* b, int col) {
			return lstrcmpi(a->columns[col].c_str(), b->columns[col].c_str());
		}

		void remove() { FavoriteManager::getInstance()->removeFavoriteUser(user); }

		void update() {
			columns[COLUMN_STATUS] = user->isOnline() ? TSTRING(ONLINE) : TSTRING(OFFLINE);
			columns[COLUMN_HUB] = Text::toT(user->getClientName());
			if(!user->getLastHubAddress().empty()) {
				columns[COLUMN_HUB] += Text::toT(" (" + user->getLastHubAddress() + ")");
			}
			columns[COLUMN_SEEN] = user->isOnline() ? Util::emptyStringT : Text::toT(Util::formatTime("%Y-%m-%d %H:%M", user->getFavoriteLastSeen()));
			columns[COLUMN_DESCRIPTION] = Text::toT(user->getUserDescription());
		}

		tstring columns[COLUMN_LAST];
	};

	CStatusBarCtrl ctrlStatus;
	CMenu usersMenu;

	TypedListViewCtrl<UserInfo, IDC_USERS> ctrlUsers;

	bool closed;

	bool startup;
	static int columnSizes[COLUMN_LAST];
	static int columnIndexes[COLUMN_LAST];

	// FavoriteManagerListener
	virtual void on(UserAdded, const User::Ptr& aUser) throw() {
		PostMessage(WM_SPEAKER, USER_ADDED, (LPARAM) new UserInfoBase(aUser));
	}
	virtual void on(UserRemoved, const User::Ptr& aUser) throw() {
		PostMessage(WM_SPEAKER, USER_REMOVED, (LPARAM) new UserInfoBase(aUser));
	}

	// ClientManagerListener
	virtual void on(ClientManagerListener::UserUpdated, const User::Ptr& aUser) throw() {
		if(aUser->isFavoriteUser()) {
			PostMessage(WM_SPEAKER, USER_UPDATED, (LPARAM) new UserInfoBase(aUser));
		}
	}

	void addUser(const User::Ptr& aUser);
	void updateUser(const User::Ptr& aUser);
	void removeUser(const User::Ptr& aUser);
};

#endif // !defined(USERS_FRAME_H)
