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

#if !defined(AFX_PRIVATEFRAME_H__8F6D05EC_ADCF_4987_8881_6DF3C0E355FA__INCLUDED_)
#define AFX_PRIVATEFRAME_H__8F6D05EC_ADCF_4987_8881_6DF3C0E355FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/User.h"
#include "../client/CriticalSection.h"
#include "../client/ClientManagerListener.h"
#include "../client/ResourceManager.h"

#include "FlatTabCtrl.h"
#include "WinUtil.h"
#include "UCHandler.h"
#include "FulEditCtrl.h"
#include "UserInfoBase.h"

#define PM_MESSAGE_MAP 8		// This could be any number, really...

class PrivateFrame : public MDITabChildWindowImpl<PrivateFrame>, 
	private ClientManagerListener, public UCHandler<PrivateFrame>
{
public:
	static void gotMessage(const User::Ptr& aUser, const string& aMessage);
	static void openWindow(const User::Ptr& aUser, const string& aMessage = Util::emptyString);
	static bool isOpen(const User::Ptr u) { return frames.find(u) != frames.end(); };
	static void closeAll();

	enum {
		USER_UPDATED
	};

	DECLARE_FRAME_WND_CLASS_EX("PrivateFrame", IDR_PRIVATEFRAME, 0, COLOR_3DFACE);

	virtual void OnFinalMessage(HWND /*hWnd*/) {
		delete this;
	}

	typedef MDITabChildWindowImpl<PrivateFrame> baseClass;
	typedef UCHandler<PrivateFrame> ucBase;

	BEGIN_MSG_MAP(PrivateFrame)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(FTM_CONTEXTMENU, onTabContextMenu)
		COMMAND_ID_HANDLER(IDC_GETLIST, onGetList)
		COMMAND_ID_HANDLER(IDC_MATCH_QUEUE, onMatchQueue)
		COMMAND_ID_HANDLER(IDC_GRANTSLOT, onGrantSlot)
		COMMAND_ID_HANDLER(IDC_ADD_TO_FAVORITES, onAddToFavorites)
		COMMAND_ID_HANDLER(IDC_SEND_MESSAGE, onSendMessage)
		COMMAND_ID_HANDLER(IDC_CLOSE_WINDOW, onCloseWindow)
		COMMAND_ID_HANDLER(IDC_COPY_NICK, onCopyNick)
		COMMAND_ID_HANDLER(IDC_COPY, onCopy)
		COMMAND_ID_HANDLER(IDC_SHOWLOG, onViewLog)
		COMMAND_ID_HANDLER(IDC_FIND, onFind)
		COMMAND_RANGE_HANDLER(IDC_SEARCH, IDC_SEARCH + 15, onSearch)
		CHAIN_COMMANDS(ucBase)
		CHAIN_MSG_MAP(baseClass)
	ALT_MSG_MAP(PM_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, onLButton)
		MESSAGE_HANDLER(WM_CHAR, onChar)
		MESSAGE_HANDLER(WM_KEYDOWN, onChar)
		MESSAGE_HANDLER(WM_KEYUP, onChar)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onMatchQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onGrantSlot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onTabContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onCopyNick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onViewLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onLButton(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

	void addLine(const string& aLine);
	void onEnter();
	void UpdateLayout(BOOL bResizeBars = TRUE);	
	void runUserCommand(UserCommand& uc);
	
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	
	LRESULT onSendMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		onEnter();
		return 0;
	}

	LRESULT onFind(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		ctrlClient.Find();
		return 0;
	}

	LRESULT onCloseWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		PostMessage(WM_CLOSE);
		return 0;
	}

	LRESULT PrivateFrame::onSpeaker(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /* bHandled */) {
		updateTitle();
		return 0;
	}
	
	LRESULT onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		HWND hWnd = (HWND)lParam;
		HDC hDC = (HDC)wParam;
		if(hWnd == ctrlClient.m_hWnd || hWnd == ctrlMessage.m_hWnd) {
			::SetBkColor(hDC, WinUtil::bgColor);
			::SetTextColor(hDC, WinUtil::textColor);
			return (LRESULT)WinUtil::bgBrush;
		}
		bHandled = FALSE;
		return FALSE;
	};

	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlMessage.SetFocus();
		return 0;
	}
	
	void addClientLine(const string& aLine) {
		if(!created) {
			CreateEx(WinUtil::mdiClient);
		}
		ctrlStatus.SetText(0, ("[" + Util::getShortTimeString() + "] " + aLine).c_str());
		
		if(BOOLSETTING(PM_BOLD_TABS))
			setDirty();
	}
	
	void setUser(const User::Ptr& aUser) { user = aUser; };
	void sendMessage(const string& msg) {
		if(user && user->isOnline()) {
			string s = "<" + user->getClientNick() + "> " + msg;
			user->privateMessage(s);
			addLine(s);
		}
	}
	
	User::Ptr& getUser() { return user; };

	bool muted;
	
private:
	PrivateFrame(const User::Ptr& aUser) : user(aUser), 
		created(false), closed(false), muted(false), doPopups(true),
		ctrlMessageContainer("edit", this, PM_MESSAGE_MAP),
		ctrlClientContainer("edit", this, PM_MESSAGE_MAP) {
	}
	
	~PrivateFrame() {
	}
	
	bool doPopups;
	bool created;
	typedef HASH_MAP<User::Ptr, PrivateFrame*, User::HashFunction> FrameMap;
	typedef FrameMap::iterator FrameIter;
	static FrameMap frames;
	CFulEditCtrl ctrlClient;
	CEdit ctrlMessage;
	CStatusBarCtrl ctrlStatus;
	static CriticalSection cs;

	CMenu tabMenu;

	StringMap ucParams;

	User::Ptr user;
	CContainedWindow ctrlMessageContainer;

	bool closed;
	
	

	//needed to receive WM_CONTEXTMENU
	CContainedWindow ctrlClientContainer;

	StringList prevCommands;
	string currentCommand;
	StringList::size_type curCommandPosition;		//can't use an iterator because StringList is a vector, and vector iterators become invalid after resizing

	CMenu userMenu;
	CMenu mcMenu;
	CMenu searchMenu;

	string searchTerm;
	
	void updateTitle() {
		if(user->isOnline()) {
			SetWindowText(user->getFullNick().c_str());
			setDisconnected(false);
		} else {
			if(user->getClientName() == STRING(OFFLINE)) {
				SetWindowText(user->getFullNick().c_str());
			} else {
				SetWindowText((user->getFullNick() + " [" + STRING(OFFLINE) + "]").c_str());
			}
			setDisconnected(true);
		}
	}
	
	// ClientManagerListener
	virtual void onAction(ClientManagerListener::Types type, const User::Ptr& aUser) throw();
};

#endif // !defined(AFX_PRIVATEFRAME_H__8F6D05EC_ADCF_4987_8881_6DF3C0E355FA__INCLUDED_)

/**
 * @file
 * $Id: PrivateFrame.h,v 1.10 2004/02/21 15:44:10 trem Exp $
 */

