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

#ifndef __USERINFOBASE_H
#define __USERINFOBASE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/User.h"
#include "resource.h"

class UserInfoBase {
public:
	UserInfoBase(const User::Ptr& u) : user(u) { };

	void getList();
	void matchQueue();
	void pm();
	void pm(const string& aMsg);
	void grant();
	void addFav();
	void removeAll();
	void showLog();

	User::Ptr user;
};

template<class T>
class UserInfoBaseHandler {
public:
	BEGIN_MSG_MAP(UserInfoBaseHandler)
		COMMAND_ID_HANDLER(IDC_GETLIST, onGetList)
		COMMAND_ID_HANDLER(IDC_MATCH_QUEUE, onMatchQueue)
		COMMAND_ID_HANDLER(IDC_PRIVATEMESSAGE, onPrivateMessage)
		COMMAND_ID_HANDLER(IDC_ADD_TO_FAVORITES, onAddToFavorites)
		COMMAND_ID_HANDLER(IDC_GRANTSLOT, onGrantSlot)
		COMMAND_ID_HANDLER(IDC_REMOVEALL, onRemoveAll)
	END_MSG_MAP()

	LRESULT onMatchQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		((T*)this)->getUserList().forEachSelected(&UserInfoBase::matchQueue);
		return 0;
	}
	LRESULT onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		((T*)this)->getUserList().forEachSelected(&UserInfoBase::getList);
		return 0;
	}
	LRESULT onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		((T*)this)->getUserList().forEachSelected(&UserInfoBase::addFav);
		return 0;
	}
	LRESULT onPrivateMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		((T*)this)->getUserList().forEachSelected(&UserInfoBase::pm);
		return 0;
	}
	LRESULT onGrantSlot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
		((T*)this)->getUserList().forEachSelected(&UserInfoBase::grant);
		return 0;
	}
	LRESULT onRemoveAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
		((T*)this)->getUserList().forEachSelected(&UserInfoBase::removeAll);
		return 0;
	}
	void appendUserItems(CMenu& menu) {
		menu.AppendMenu(MF_STRING, IDC_GETLIST, CSTRING(GET_FILE_LIST));
		menu.AppendMenu(MF_STRING, IDC_PRIVATEMESSAGE, CSTRING(SEND_PRIVATE_MESSAGE));
		menu.AppendMenu(MF_STRING, IDC_MATCH_QUEUE, CSTRING(MATCH_QUEUE));
		menu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CSTRING(ADD_TO_FAVORITES));
		menu.AppendMenu(MF_STRING, IDC_GRANTSLOT, CSTRING(GRANT_EXTRA_SLOT));
		menu.AppendMenu(MF_STRING, IDC_REMOVEALL, CSTRING(REMOVE_FROM_ALL));
	}
};

#endif