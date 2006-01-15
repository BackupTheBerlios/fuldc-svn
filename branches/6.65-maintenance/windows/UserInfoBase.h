/* 
* Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
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
	void pm(const tstring& aMsg);
	void grant();
	void addFav();
	void removeAll();
	void showLog();
	void ignore();
	void unignore();

	User::Ptr& getUser() { return user; }	
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
		COMMAND_ID_HANDLER(IDC_IGNORE, onIgnore)
		COMMAND_ID_HANDLER(IDC_UNIGNORE, onUnIgnore)
		COMMAND_ID_HANDLER(IDC_SHOWLOG, onShowLog)
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

	LRESULT onIgnore(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
		((T*)this)->getUserList().forEachSelected(&UserInfoBase::ignore);
		return 0;
	}

	LRESULT onUnIgnore(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
		((T*)this)->getUserList().forEachSelected(&UserInfoBase::unignore);
		return 0;
	}

	LRESULT onShowLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { 
		((T*)this)->getUserList().forEachSelected(&UserInfoBase::showLog);
		return 0;
	}
	struct NmdcOnly {
		NmdcOnly() : nmdcOnly(true) { }
		void operator()(UserInfoBase* ui) { if(!ui->getUser()->getCID().isZero()) nmdcOnly = false; }

		bool nmdcOnly;
	};
	void checkAdcItems(CMenu& menu) { menu; /*avoid warning*/ }

	void appendUserItems(CMenu& menu) {
		menu.AppendMenu(MF_STRING, IDC_GETLIST, CTSTRING(GET_FILE_LIST));
		menu.AppendMenu(MF_STRING, IDC_PRIVATEMESSAGE, CTSTRING(SEND_PRIVATE_MESSAGE));
		menu.AppendMenu(MF_STRING, IDC_MATCH_QUEUE, CTSTRING(MATCH_QUEUE));
		menu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CTSTRING(ADD_TO_FAVORITES));
		menu.AppendMenu(MF_STRING, IDC_GRANTSLOT, CTSTRING(GRANT_EXTRA_SLOT));
		menu.AppendMenu(MF_STRING, IDC_REMOVEALL, CTSTRING(REMOVE_FROM_ALL));
		menu.AppendMenu(MF_STRING, IDC_SHOWLOG, CTSTRING(SHOW_LOG));
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, IDC_IGNORE, CTSTRING(IGNOREA));
		menu.AppendMenu(MF_STRING, IDC_UNIGNORE, CTSTRING(UNIGNORE));
		menu.AppendMenu(MF_SEPARATOR);
	}
};

#endif