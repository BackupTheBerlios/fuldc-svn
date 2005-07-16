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

#if !defined(HUB_FRAME_H)
#define HUB_FRAME_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "FlatTabCtrl.h"
#include "TypedListViewCtrl.h"
#include "FulEditCtrl.h"

#include "../client/Client.h"
#include "../client/NmdcHub.h"
#include "../client/User.h"
#include "../client/ClientManager.h"
#include "../client/TimerManager.h"
#include "../client/FastAlloc.h"

#include "WinUtil.h"
#include "UCHandler.h"

#define EDIT_MESSAGE_MAP 10		// This could be any number, really...
#define FILTER_MESSAGE_MAP 8

#define RESOLVE_IP WM_APP+1

class HubFrame : public MDITabChildWindowImpl<HubFrame>, private ClientListener, 
	public CSplitterImpl<HubFrame>, private TimerManagerListener, public UCHandler<HubFrame>,
	public UserInfoBaseHandler<HubFrame>
{
public:
	DECLARE_FRAME_WND_CLASS_EX(_T("HubFrame"), IDR_HUB, 0, COLOR_3DFACE);

	typedef CSplitterImpl<HubFrame> splitBase;
	typedef MDITabChildWindowImpl<HubFrame> baseClass;
	typedef UCHandler<HubFrame> ucBase;
	typedef UserInfoBaseHandler<HubFrame> uibBase;

	BEGIN_MSG_MAP(HubFrame)
		NOTIFY_HANDLER(IDC_USERS, LVN_GETDISPINFO, ctrlUsers.onGetDispInfo)
		NOTIFY_HANDLER(IDC_USERS, LVN_COLUMNCLICK, ctrlUsers.onColumnClick)
		NOTIFY_HANDLER(IDC_USERS, NM_DBLCLK, onDoubleClickUsers)
		NOTIFY_HANDLER(IDC_USERS, LVN_KEYDOWN, onKeyDownUsers)
		NOTIFY_HANDLER(IDC_USERS, NM_RETURN, onEnterUsers)
		NOTIFY_CODE_HANDLER(TTN_GETDISPINFO, onGetToolTip)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_SETFOCUS, onSetFocus)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		MESSAGE_HANDLER(FTM_CONTEXTMENU, onTabContextMenu)
		MESSAGE_HANDLER(WM_MENUCOMMAND, ctrlClient.onMenuCommand)
		MESSAGE_HANDLER(RESOLVE_IP, onResolvedIP)
		COMMAND_ID_HANDLER(ID_FILE_RECONNECT, OnFileReconnect)
		COMMAND_ID_HANDLER(IDC_FOLLOW, onFollow)
		COMMAND_ID_HANDLER(IDC_SEND_MESSAGE, onSendMessage)
		COMMAND_ID_HANDLER(IDC_ADD_AS_FAVORITE, onAddAsFavorite)
		COMMAND_ID_HANDLER(IDC_CLOSE_WINDOW, onCloseWindow)
		COMMAND_ID_HANDLER(IDC_FIND, ctrlClient.onFind)
		COMMAND_ID_HANDLER(IDC_SHOW_HUB_LOG, onShowHubLog)
		COMMAND_ID_HANDLER(IDC_OPEN_LOG_DIR, onShowHubLog)
		COMMAND_RANGE_HANDLER(IDC_COPY, IDC_COPY+1+COLUMN_LAST, onCopyUserList);
		CHAIN_COMMANDS(ucBase)
		CHAIN_COMMANDS(uibBase)
		CHAIN_MSG_MAP(baseClass)
		CHAIN_MSG_MAP(splitBase)
	ALT_MSG_MAP(EDIT_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, onChar)
		MESSAGE_HANDLER(WM_KEYDOWN, onChar)
		MESSAGE_HANDLER(WM_KEYUP, onChar)
		MESSAGE_HANDLER(BM_SETCHECK, onShowUsers)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, onLButton)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
	ALT_MSG_MAP(FILTER_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, onFilterChar)
		MESSAGE_HANDLER(WM_KEYUP, onFilterChar)
		COMMAND_CODE_HANDLER(CBN_SELCHANGE, onSelChange)
	END_MSG_MAP()

	virtual void OnFinalMessage(HWND /*hWnd*/) {
		dcassert(frames.find(server) != frames.end());
		dcassert(frames[server] == this);
		frames.erase(server);
		delete this;
	}

	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onDoubleClickUsers(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onTabContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onShowUsers(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onFollow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onLButton(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onEnterUsers(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT onGetToolTip(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT onSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onFilterChar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onShowHubLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onCopyUserList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onResolvedIP(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	
	
	
	void UpdateLayout(BOOL bResizeBars = TRUE);
	void addLine(tstring aLine, bool bold = true);
	void addClientLine(const tstring& aLine, bool inChat = true);
	void onEnter();
	void onTab();
	void handleTab(bool reverse);
	void runUserCommand(::UserCommand& uc);

	static void openWindow(const tstring& server);
	static void closeDisconnected();
	static void setClosing();
	
	LRESULT onSetFocus(UINT /* uMsg */, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlMessage.SetFocus();
		return 0;
	}

	LRESULT onSendMessage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		onEnter();
		return 0;
	}
	
	LRESULT onAddAsFavorite(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		addAsFavorite();
		return 0;
	}

	LRESULT onCloseWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		PostMessage(WM_CLOSE);
		return 0;
	}

	LRESULT onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
		HWND hWnd = (HWND)lParam;
		HDC hDC = (HDC)wParam;
		if(hWnd == ctrlClient.m_hWnd || hWnd == ctrlMessage.m_hWnd 
			|| hWnd == ctrlFilter.m_hWnd || ctrlFilterSel.m_hWnd) {
			::SetBkColor(hDC, WinUtil::bgColor);
			::SetTextColor(hDC, WinUtil::textColor);
			return (LRESULT)WinUtil::bgBrush;
		} else {
			return 0;
		}
	}

	LRESULT OnFileReconnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		client->disconnect();
		clearUserList();
		client->connect();
		return 0;
	}

	LRESULT onKeyDownUsers(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLVKEYDOWN* l = (NMLVKEYDOWN*)pnmh;
		if(l->wVKey == VK_TAB) {
			onTab();
		}
		return 0;
	}

private:
	class UserInfo;
public:
	TypedListViewCtrl<UserInfo, IDC_USERS>& getUserList() { return ctrlUsers; };
private:

	enum Speakers { UPDATE_USER, UPDATE_USERS, REMOVE_USER, ADD_CHAT_LINE,
		ADD_STATUS_LINE, ADD_SILENT_STATUS_LINE, SET_WINDOW_TITLE, GET_PASSWORD, 
		PRIVATE_MESSAGE, STATS, CONNECTED, DISCONNECTED
	};

	enum {
		IMAGE_USER = 0, IMAGE_OP
	};
	
	enum {
		COLUMN_FIRST, 
		COLUMN_NICK = COLUMN_FIRST, 
		COLUMN_SHARED, 
		COLUMN_DESCRIPTION, 
		COLUMN_ISP,
		COLUMN_TAG,
		COLUMN_IP,
		COLUMN_CONNECTION, 
		COLUMN_EMAIL, 
		COLUMN_LAST
	};

	struct UpdateInfo {
		UpdateInfo() { }
		UpdateInfo(const OnlineUser& ou) : user(ou.getUser()), identity(ou.getIdentity()) {	}

		User::Ptr user;
		Identity identity;
	};


	friend struct CompareItems;
	class UserInfo : public UserInfoBase, public FastAlloc<UserInfo> {
	public:
		UserInfo(const UpdateInfo& u, bool aStripIsp) : UserInfoBase(u.user), op(false), hidden(false), stripIsp(aStripIsp) { 
			update(u.identity, -1); 
		};

		const tstring& getText(int col) const {
			return columns[col];
		}

		static int compareItems(const UserInfo* a, const UserInfo* b, int col) {
			if(col == COLUMN_NICK) {
				if(a->getOp() && !b->getOp()) {
					return -1;
				} else if(!a->getOp() && b->getOp()) {
					return 1;
				}
			}
			if(col == COLUMN_SHARED) {
				return compare(a->getBytes(), b->getBytes());
			}
			return lstrcmpi(a->columns[col].c_str(), b->columns[col].c_str());	
		}

		bool update(const Identity& identity, int sortCol);

		tstring columns[COLUMN_LAST];
		GETSET(int64_t, bytes, Bytes);
		GETSET(bool, op, Op);
		GETSET(bool, hidden, Hidden);
		bool stripIsp;
	};

	class PMInfo {
	public:
		PMInfo(const User::Ptr& u, const string& m) : user(u), msg(Text::toT(m)) { };
		User::Ptr user;
		tstring msg;
	};

	HubFrame(const tstring& aServer) : 
	waitingForPW(false), extraSort(false), server(aServer), closed(false), 
		updateUsers(true), curCommandPosition(0),
		ctrlMessageContainer(WC_EDIT, this, EDIT_MESSAGE_MAP), 
		showUsersContainer(WC_BUTTON, this, EDIT_MESSAGE_MAP),
		clientContainer(WC_EDIT, this, EDIT_MESSAGE_MAP),
		ctrlFilterContainer(WC_EDIT, this, FILTER_MESSAGE_MAP),
		ctrlFilterSelContainer(WC_COMBOBOX, this, FILTER_MESSAGE_MAP)
	{
		FavoriteHubEntry* fhe = FavoriteManager::getInstance()->getFavoriteHubEntry(Text::fromT(aServer));
		if(fhe != NULL) {
			stripIsp     = fhe->getStripIsp();
			showJoins    = fhe->getShowJoins();
			showUserList = fhe->getShowUserlist();
			logMainChat	 = fhe->getLogMainChat();
		} else {
			stripIsp     = BOOLSETTING(STRIP_ISP);
			showJoins    = BOOLSETTING(SHOW_JOINS);
			showUserList = BOOLSETTING(GET_USER_INFO);
			logMainChat  = BOOLSETTING(LOG_MAIN_CHAT);
		}
		client = ClientManager::getInstance()->getClient(Text::fromT(aServer));
		client->addListener(this);
		timeStamps = BOOLSETTING(TIME_STAMPS);

		resolveBuffer = NULL;
		
		tabList.push_back(_T("/away"));
		tabList.push_back(_T("/back"));
        tabList.push_back(_T("/clear"));
		tabList.push_back(_T("/close"));
		tabList.push_back(_T("/connection"));
		tabList.push_back(_T("/ctopic"));
		tabList.push_back(_T("/dc++"));
		tabList.push_back(_T("/df"));
		tabList.push_back(_T("/dslots"));
		tabList.push_back(_T("/favorite"));
		tabList.push_back(_T("/favshowjoins"));
		tabList.push_back(_T("/fuldc"));
		tabList.push_back(_T("/fuptime"));
		tabList.push_back(_T("/getlist"));
		tabList.push_back(_T("/grant"));
		tabList.push_back(_T("/help"));
		tabList.push_back(_T("/join"));
		tabList.push_back(_T("/lastlog"));
		tabList.push_back(_T("/log"));
		tabList.push_back(_T("/lastseen"));
		tabList.push_back(_T("/pm"));
		tabList.push_back(_T("/popups"));
		tabList.push_back(_T("/rebuild"));
		tabList.push_back(_T("/refresh"));
		tabList.push_back(_T("/refreshi"));
		tabList.push_back(_T("/search"));
		tabList.push_back(_T("/share"));
		tabList.push_back(_T("/showjoins"));
		tabList.push_back(_T("/showlog"));
		tabList.push_back(_T("/slots"));
		tabList.push_back(_T("/topic"));
		tabList.push_back(_T("/ts"));
		tabList.push_back(_T("/unshare"));
		tabList.push_back(_T("/uptime"));
	}

	~HubFrame() {
		ClientManager::getInstance()->putClient(client);

		if(resolveBuffer)
			delete[] resolveBuffer;
	}

	typedef HASH_MAP<tstring, HubFrame*> FrameMap;
	typedef FrameMap::iterator FrameIter;
	static FrameMap frames;

	tstring redirect;
	bool timeStamps;
	bool showJoins;
	bool favShowJoins;
	tstring complete;

	tstring lastKick;
	tstring lastRedir;
	tstring lastServer;
	
	bool waitingForPW;
	bool extraSort;
	bool stripIsp;
	bool showUserList;

	TStringList prevCommands;
	tstring currentCommand;
	TStringList::size_type curCommandPosition;		//can't use an iterator because StringList is a vector, and vector iterators become invalid after resizing

	TStringList tabList;

	Client* client;
	tstring server;
	CContainedWindow ctrlMessageContainer;
	CContainedWindow clientContainer;
	CContainedWindow showUsersContainer;
	CContainedWindow ctrlFilterContainer;
	CContainedWindow ctrlFilterSelContainer;
	
	CMenu userMenu;
	CMenu tabMenu;
	CMenu mcMenu;
	CMenu searchMenu;
	CMenu copyMenu;

	tstring searchTerm;
	tstring filter;
	typedef multimap< tstring, UserInfo* > UserMap;
	typedef pair< tstring, UserInfo* > UserPair;
	typedef UserMap::iterator UserIter;
	UserMap usermap; //save all userinfo items that don't match the filter here
	tstring curNick;
	
	CButton ctrlShowUsers;
	CFulEditCtrl ctrlClient;
	CEdit ctrlMessage;
	CEdit ctrlFilter;
	CComboBox ctrlFilterSel;
	typedef TypedListViewCtrl<UserInfo, IDC_USERS> CtrlUsers;
	CtrlUsers ctrlUsers;
	CStatusBarCtrl ctrlStatus;

	bool closed;
	static bool closing;
	bool logMainChat;

	StringMap ucParams;
	TStringMap tabParams;
	bool tabMenuShown;

	typedef vector<pair<UpdateInfo, Speakers> > UpdateList;
	typedef UpdateList::iterator UpdateIter;
	UpdateList updateList;
	CriticalSection updateCS;
	bool updateUsers;
	bool resort;

	enum { MAX_CLIENT_LINES = 5 };
	TStringList lastLinesList;
	tstring lastLines;
	CToolTipCtrl ctrlLastLines;
	
	static int columnIndexes[COLUMN_LAST];
	static int columnSizes[COLUMN_LAST];

	char *resolveBuffer;
	bool isIP;
	
	int findUser(const User::Ptr& aUser);
	UserInfo* findUser(tstring & nick);

	bool updateUser(const UpdateInfo& u);
	void removeUser(const UpdateInfo& u);

	void updateUserList();
	bool parseFilter(int& mode, int64_t& size);
	void addAsFavorite();
	bool matchFilter(const UserInfo& ui, int sel, bool doSizeCompare = false, int mode = 0, int64_t size = 0);

	void clearUserList();

	int getImage(const User::Ptr& u) {
		int image = u->isSet(User::OP) ? IMAGE_OP : IMAGE_USER;
		
		if(u->isSet(User::DCPLUSPLUS))
			image+=2;
		if(u->isSet(User::PASSIVE)) {
			image+=4;
		}
		return image;	
	}

	void updateStatusBar() {
		if(m_hWnd)
			PostMessage(WM_SPEAKER, STATS);
	}

	// TimerManagerListener
	virtual void on(TimerManagerListener::Second, DWORD /*aTick*/) throw();

	// ClientListener
	virtual void on(Connecting, Client*) throw();
	virtual void on(Connected, Client*) throw();
	virtual void on(BadPassword, Client*) throw();
	virtual void on(UserUpdated, Client*, const OnlineUser&) throw();
	virtual void on(UsersUpdated, Client*, const OnlineUser::List&) throw();
	virtual void on(UserRemoved, Client*, const OnlineUser&) throw();
	virtual void on(Redirect, Client*, const string&) throw();
	virtual void on(Failed, Client*, const string&) throw();
	virtual void on(GetPassword, Client*) throw();
	virtual void on(HubUpdated, Client*) throw();
	virtual void on(Message, Client*, const string&) throw();
	virtual void on(PrivateMessage, Client*, const OnlineUser&, const string&) throw();
	virtual void on(NickTaken, Client*) throw();
	virtual void on(SearchFlood, Client*, const string&) throw();
	virtual void on(UserIp, Client*, const OnlineUser::List&) throw();

	void speak(Speakers s) { PostMessage(WM_SPEAKER, (WPARAM)s); };
	void speak(Speakers s, const string& msg) { PostMessage(WM_SPEAKER, (WPARAM)s, (LPARAM)new tstring(Text::toT(msg))); };
	void speak(Speakers s, const OnlineUser& u) { 
		Lock l(updateCS);
		updateList.push_back(make_pair(UpdateInfo(u), s));
		updateUsers = true;
	};
	void speak(Speakers s, const OnlineUser& u, const string& line) { PostMessage(WM_SPEAKER, (WPARAM)s, (LPARAM)new PMInfo(u.getUser(), line)); };

	void openLinksInTopic();
	bool resolve(const wstring& aDns);
};

#endif // !defined(HUB_FRAME_H)

/**
 * @file
 * $Id: HubFrame.h,v 1.15 2004/02/21 15:15:28 trem Exp $
 */
