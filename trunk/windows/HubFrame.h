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

#if !defined(AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)
#define AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_

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
#define FILTER_MESSAGE_MAP 11

class HubFrame : public MDITabChildWindowImpl<HubFrame>, private ClientListener, 
	public CSplitterImpl<HubFrame>, private TimerManagerListener, public UCHandler<HubFrame>,
	public UserInfoBaseHandler<HubFrame>
{
public:
	DECLARE_FRAME_WND_CLASS_EX("HubFrame", IDR_HUBFRAME, 0, COLOR_3DFACE);
	
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
		COMMAND_ID_HANDLER(ID_FILE_RECONNECT, OnFileReconnect)
		COMMAND_ID_HANDLER(IDC_FOLLOW, onFollow)
		COMMAND_ID_HANDLER(IDC_SEND_MESSAGE, onSendMessage)
		COMMAND_ID_HANDLER(IDC_ADD_AS_FAVORITE, onAddAsFavorite)
		COMMAND_ID_HANDLER(IDC_COPY_NICK, onCopyNick)
		COMMAND_ID_HANDLER(IDC_CLOSE_WINDOW, onCloseWindow)
		COMMAND_ID_HANDLER(IDC_COPY, onCopy)
		COMMAND_ID_HANDLER(IDC_FIND, onFind)
		COMMAND_ID_HANDLER(IDC_SHOWLOG, onShowLog)
		COMMAND_ID_HANDLER(IDC_SHOW_HUB_LOG, onShowHubLog)
		COMMAND_ID_HANDLER(IDC_OPEN_LOG_DIR, onShowHubLog)
		COMMAND_RANGE_HANDLER(IDC_SEARCH, IDC_SEARCH + 15, onSearch)
		COMMAND_RANGE_HANDLER(IDC_COPY+1, IDC_COPY+1+COLUMN_LAST, onCopy);
		//COMMAND_RANGE_HANDLER(IDC_HEADER_MENU, IDC_HEADER_MENU + 10, ctrlUsers.onHeaderMenu)
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
		MESSAGE_HANDLER(WM_RBUTTONDOWN, onContextMenu)
	ALT_MSG_MAP(FILTER_MESSAGE_MAP)
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
	LRESULT onCopyNick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
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
	LRESULT onCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onFilterChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onShowHubLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
	
	
	void UpdateLayout(BOOL bResizeBars = TRUE);
	void addLine(string& aLine);
	void addClientLine(const string& aLine, bool inChat = true);
	void onEnter();
	void onTab();
	void runUserCommand(::UserCommand& uc);

	static void openWindow(const string& server, const string& nick = Util::emptyString, const string& password = Util::emptyString, const string& description = Util::emptyString, bool StripIsp = false);
	static void closeDisconnected();
	static void setClosing();
	
	LRESULT onSetFocus(UINT /* uMsg */, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlMessage.SetFocus();
		return 0;
	}

	LRESULT onFind(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		ctrlClient.Find();
		return 0;
	}
	
	LRESULT onShowLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		int i=-1;
		if(client->isConnected()) {
		
			while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
				string path =	SETTING(LOG_DIRECTORY) + ctrlUsers.getItemData(i)->user->getNick() + ".log";
				ShellExecute(NULL, "open", path.c_str(), NULL, NULL, SW_SHOW);
			}
		}
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
		clearUserList();
		client->addListener(this);
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

	bool closing;

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
		COLUMN_CONNECTION, 
		COLUMN_EMAIL, 
		COLUMN_LAST
	};
	friend struct CompareItems;
	class UserInfo : public UserInfoBase, public FastAlloc<UserInfo> {
	public:
		UserInfo(const User::Ptr& u, bool aStripIsp) : UserInfoBase(u), op(false), stripIsp(aStripIsp) { update(); };

		const string& getText(int col) const {
			switch(col) {
				case COLUMN_NICK: 
					if(stripIsp) 
						return user->getShortNick();
					else
						return user->getNick();
				case COLUMN_SHARED: return shared;
				case COLUMN_DESCRIPTION: return user->getDescription();
				case COLUMN_TAG: return user->getTag();
				case COLUMN_ISP: return user->getIsp();
				case COLUMN_CONNECTION: return user->getConnection();
				case COLUMN_EMAIL: return user->getEmail();
				default: return Util::emptyString;
			}
		}

		static int compareItems(const UserInfo* a, const UserInfo* b, int col) {
			if(a == NULL || b == NULL){
				dcdebug("UserInfo::compareItems: pointer == NULL\n");
				return 0;
			}
			switch(col) {
				case COLUMN_NICK:
					if(a->getOp() && !b->getOp()) {
						return -1;
					} else if(!a->getOp() && b->getOp()) {
						return 1;
					}
					if(a->stripIsp)
						return Util::stricmp(a->user->getShortNick(), b->user->getShortNick());	
					else
						return Util::stricmp(a->user->getNick(), b->user->getNick());	
				case COLUMN_SHARED:	return compare(a->user->getBytesShared(), b->user->getBytesShared());
				case COLUMN_DESCRIPTION: return Util::stricmp(a->user->getDescription(), b->user->getDescription());
				case COLUMN_TAG: return Util::stricmp(a->user->getTag(), b->user->getTag());
				case COLUMN_ISP: return Util::stricmp(a->user->getIsp(), b->user->getIsp());
				case COLUMN_CONNECTION: return Util::stricmp(a->user->getConnection(), b->user->getConnection());
				case COLUMN_EMAIL: return Util::stricmp(a->user->getEmail(), b->user->getEmail());
				default: return 0;
			}
		}

		void update() { shared = Util::formatBytes(user->getBytesShared()); op = user->isSet(User::OP); }

		GETSET(string, shared, Shared);
		GETSET(bool, op, Op);
		bool stripIsp;
	};

	class PMInfo {
	public:
		PMInfo(const User::Ptr& u, const string& m) : user(u), msg(m) { };
		User::Ptr user;
		string msg;
	};

	HubFrame(const string& aServer, const string& aNick, const string& aPassword, const string& aDescription, bool aStripIsp) : 
	waitingForPW(false), extraSort(false), server(aServer), closed(false), closing(false), 
		updateUsers(false), curCommandPosition(0), 
		ctrlMessageContainer("edit", this, EDIT_MESSAGE_MAP), 
		showUsersContainer("BUTTON", this, EDIT_MESSAGE_MAP),
		clientContainer("edit", this, EDIT_MESSAGE_MAP),
		ctrlFilterContainer("edit", this, FILTER_MESSAGE_MAP),
		ctrlFilterSelContainer("COMBOBOX", this, FILTER_MESSAGE_MAP)
	{
		stripIsp = aNick.empty() ? BOOLSETTING(STRIP_ISP) : aStripIsp;
		client = ClientManager::getInstance()->getClient(aServer);
		client->setNick(aNick.empty() ? SETTING(NICK) : aNick);
        if (!aDescription.empty())
			client->setDescription(aDescription);
		client->setPassword(aPassword);
		client->addListener(this);
		TimerManager::getInstance()->addListener(this);
		timeStamps = BOOLSETTING(TIME_STAMPS);
		showUserList = BOOLSETTING(GET_USER_INFO);

		tabList.push_back("/away");
		tabList.push_back("/back");
        tabList.push_back("/clear");
		tabList.push_back("/close");
		tabList.push_back("/connection");
		tabList.push_back("/ctopic");
		tabList.push_back("/dc++");
		tabList.push_back("/dslots");
		tabList.push_back("/favorite");
		tabList.push_back("/fuldc");
		tabList.push_back("/getlist");
		tabList.push_back("/grant");
		tabList.push_back("/help");
		tabList.push_back("/join");
		tabList.push_back("/lastlog");
		tabList.push_back("/lastseen");
		tabList.push_back("/pm");
		tabList.push_back("/popups");
		tabList.push_back("/rebuild");
		tabList.push_back("/refresh");
		tabList.push_back("/search");
		tabList.push_back("/share");
		tabList.push_back("/showjoins");
		tabList.push_back("/slots");
		tabList.push_back("/topic");
		tabList.push_back("/ts");
		tabList.push_back("/unshare");
		tabList.push_back("/uptime");
	}

	~HubFrame() {
		ClientManager::getInstance()->putClient(client);
	}

	typedef HASH_MAP<string, HubFrame*> FrameMap;
	typedef FrameMap::iterator FrameIter;
	static FrameMap frames;

	string redirect;
	bool timeStamps;
	bool showJoins;
	string complete;

	string lastKick;
	string lastRedir;
	string lastServer;
	
	bool waitingForPW;
	bool extraSort;
	bool stripIsp;
	bool showUserList;

	StringList prevCommands;
	string currentCommand;
	StringList::size_type curCommandPosition;		//can't use an iterator because StringList is a vector, and vector iterators become invalid after resizing

	StringList tabList;

	Client* client;
	string server;
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

	string searchTerm;
	string filter;
	//typedef list< UserInfo* > FilterList;
	typedef multimap< string, UserInfo* > UserMap;
	typedef pair< string, UserInfo* > UserPair;
	UserMap usermap; //save all userinfo items that don't match the filter here
	string curNick;
	
	CButton ctrlShowUsers;
	CFulEditCtrl ctrlClient;
	CEdit ctrlMessage;
	CEdit ctrlFilter;
	CComboBox ctrlFilterSel;
	typedef TypedListViewCtrl<UserInfo, IDC_USERS> CtrlUsers;
	CtrlUsers ctrlUsers;
	CStatusBarCtrl ctrlStatus;

	CHARFORMAT2 selFormat;

	bool closed;
	

	StringMap ucParams;
	StringMap tabParams;
	bool tabMenuShown;

	typedef vector<pair<User::Ptr, Speakers> > UpdateList;
	typedef UpdateList::iterator UpdateIter;
	UpdateList updateList;
	CriticalSection updateCS;
	bool updateUsers;

	enum { MAX_CLIENT_LINES = 5 };
	StringList lastLinesList;
	string lastLines;
	CToolTipCtrl ctrlLastLines;
	
	static int columnIndexes[COLUMN_LAST];
	static int columnSizes[COLUMN_LAST];
	
	int findUser(const User::Ptr& aUser);
	UserInfo* findUser(string & nick);

	bool updateUser(const User::Ptr& u);
	void removeUser(const User::Ptr& u);
	void updateUserList();
	void addAsFavorite();

	bool getUserInfo() { return ctrlShowUsers.GetCheck() == BST_CHECKED; }

	void clearUserList() {
		{
			Lock l(updateCS);
			updateList.clear();
		}

		UserMap::iterator i = usermap.begin();
		for(; i != usermap.end(); ++i)
			delete i->second;

		usermap.clear();

		ctrlUsers.DeleteAllItems();
	}

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
	virtual void on(UserUpdated, Client*, const User::Ptr&) throw();
	virtual void on(UsersUpdated, Client*, const User::List&) throw();
	virtual void on(UserRemoved, Client*, const User::Ptr&) throw();
	virtual void on(Redirect, Client*, const string&) throw();
	virtual void on(Failed, Client*, const string&) throw();
	virtual void on(GetPassword, Client*) throw();
	virtual void on(HubUpdated, Client*) throw();
	virtual void on(Message, Client*, const string&) throw();
	virtual void on(PrivateMessage, Client*, const User::Ptr&, const string&) throw() ;
	virtual void on(NickTaken, Client*) throw() ;
	virtual void on(SearchFlood, Client*, const string&) throw() ;

	void speak(Speakers s) { PostMessage(WM_SPEAKER, (WPARAM)s); };
	void speak(Speakers s, const string& msg) { PostMessage(WM_SPEAKER, (WPARAM)s, (LPARAM)new string(msg)); };
	void speak(Speakers s, const User::Ptr& u) { 
		Lock l(updateCS);
		updateList.push_back(make_pair(u, s));
		updateUsers = true;
	};
	void speak(Speakers s, const User::Ptr& u, const string& line) { PostMessage(WM_SPEAKER, (WPARAM)s, (LPARAM)new PMInfo(u, line)); };

	void openLinksInTopic();
};
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)

/**
 * @file
 * $Id: HubFrame.h,v 1.15 2004/02/21 15:15:28 trem Exp $
 */

