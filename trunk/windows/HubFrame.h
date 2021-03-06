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

#if !defined(HUB_FRAME_H)
#define HUB_FRAME_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "FlatTabCtrl.h"
#include "TypedListViewCtrl.h"
#include "FulEditCtrl.h"

#include "../client/Client.h"
#include "../client/User.h"
#include "../client/ClientManager.h"
#include "../client/TimerManager.h"
#include "../client/FastAlloc.h"
#include "../client/TaskQueue.h"

#include "WinUtil.h"
#include "UCHandler.h"

#define EDIT_MESSAGE_MAP 10		// This could be any number, really...
#define FILTER_MESSAGE_MAP 8
#define RESOLVE_IP WM_APP+1

struct CompareItems;

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
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		MESSAGE_HANDLER(FTM_CONTEXTMENU, onTabContextMenu)
		MESSAGE_HANDLER(WM_MENUCOMMAND, ctrlClient.onMenuCommand)
		MESSAGE_HANDLER(RESOLVE_IP, onResolvedIP)
		COMMAND_ID_HANDLER(ID_FILE_RECONNECT, onFileReconnect)
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

	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
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
	LRESULT onCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onFileReconnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

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

	LRESULT onItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/) {
		updateStatusBar();
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
	TypedListViewCtrl<UserInfo, IDC_USERS>& getUserList() { return ctrlUsers; }
private:
	enum Tasks { UPDATE_USER_JOIN, UPDATE_USER, REMOVE_USER, ADD_CHAT_LINE,
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
		COLUMN_TAG,
		COLUMN_IP,
		COLUMN_CONNECTION,
		COLUMN_EMAIL,
		COLUMN_CID,
		COLUMN_LAST
	};

	enum FilterModes{
		NONE,
		EQUAL,
		GREATER_EQUAL,
		LESS_EQUAL,
		GREATER,
		LESS,
		NOT_EQUAL
	};

	struct UserTask : public Task {
		UserTask(const OnlineUser& ou) : user(ou.getUser()), identity(ou.getIdentity()) { }

		User::Ptr user;
		Identity identity;
	};

	struct PMTask : public StringTask {
		PMTask(const OnlineUser& from_, const OnlineUser& to_, const OnlineUser& replyTo_, const string& m) : StringTask(m),
			from(from_.getUser()), to(to_.getUser()), replyTo(replyTo_.getUser()), hub(replyTo_.getIdentity().isHub()), bot(replyTo_.getIdentity().isBot()) { }

		User::Ptr from;
		User::Ptr to;
		User::Ptr replyTo;

		bool hub;
		bool bot;
	};

	friend struct CompareItems;
	class UserInfo : public UserInfoBase, public FastAlloc<UserInfo> {
	public:
		UserInfo(const UserTask& u) : UserInfoBase(u.user) {
			update(u.identity, -1);
		}

		const tstring& getText(int col) const {
			dcassert(col >= 0 && col < COLUMN_LAST);
			return columns[col];
		}

		const tstring& copy(int col) {
			if(col >= 0 && col < COLUMN_LAST)
				return getText(col);

			return Util::emptyStringT;
		}

		static int compareItems(const UserInfo* a, const UserInfo* b, int col) {
			if(col == COLUMN_NICK) {
				if(a->getIdentity().isOp() && !b->getIdentity().isOp()) {
					return -1;
				} else if(!a->getIdentity().isOp() && b->getIdentity().isOp()) {
					return 1;
				}
			}
			if(col == COLUMN_SHARED) {
				return compare(a->identity.getBytesShared(), b->identity.getBytesShared());;
			}
			return lstrcmpi(a->columns[col].c_str(), b->columns[col].c_str());
		}

		bool update(const Identity& identity, int sortCol);

		string getNick() const { return identity.getNick(); }
		bool isHidden() const { return identity.isHidden(); }

		tstring columns[COLUMN_LAST];
		GETSET(Identity, identity, Identity);
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
			showJoins    = fhe->getShowJoins();
			showUsers = fhe->getShowUserList();
			logMainChat	 = fhe->getLogMainChat();
		} else {
			showJoins    = BOOLSETTING(SHOW_JOINS);
			showUsers = BOOLSETTING(GET_USER_INFO);
			logMainChat  = BOOLSETTING(LOG_MAIN_CHAT);
		}
		client = ClientManager::getInstance()->getClient(Text::fromT(aServer));
		client->addListener(this);
		timeStamps = BOOLSETTING(TIME_STAMPS);

		resolveBuffer = NULL;
		
		/* @todo
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
		tabList.push_back(_T("/userlist"));
		*/
	}

	virtual ~HubFrame() {
		ClientManager::getInstance()->putClient(client);

		dcassert(frames.find(server) != frames.end());
		dcassert(frames[server] == this);
		frames.erase(server);

		clearTaskList();
	}

	typedef HASH_MAP<tstring, HubFrame*> FrameMap;
	typedef FrameMap::iterator FrameIter;
	static FrameMap frames;

	typedef HASH_MAP<User::Ptr, UserInfo*, User::HashFunction> UserMap;
	typedef UserMap::iterator UserMapIter;

	tstring redirect;
	bool timeStamps;
	bool showJoins;
	bool favShowJoins;
	tstring complete;

	bool waitingForPW;
	bool extraSort;

	TStringList prevCommands;
	tstring currentCommand;
	TStringList::size_type curCommandPosition;		//can't use an iterator because StringList is a vector, and vector iterators become invalid after resizing

	struct CountAvailable {
		CountAvailable() : available(0) { }
		int64_t available;
		void operator()(UserInfo *ui) {
			available += ui->getIdentity().getBytesShared();
		}
		void operator()(UserMap::const_reference ui) {
			available += ui.second->getIdentity().getBytesShared();
		}
	};

	string getUsersTextForStatusBar() const;

	int64_t getAvailable() {
		if (ctrlUsers.GetSelectedCount() > 1) {
			return ctrlUsers.forEachSelectedT(CountAvailable()).available;
		} else
			return for_each(userMap.begin(), userMap.end(), CountAvailable()).available;
	}

	const tstring& getNick(const User::Ptr& u);

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
	
	CButton ctrlShowUsers;
	CFulEditCtrl ctrlClient;
	CEdit ctrlMessage;
	CEdit ctrlFilter;
	CComboBox ctrlFilterSel;
	typedef TypedListViewCtrl<UserInfo, IDC_USERS> CtrlUsers;
	CtrlUsers ctrlUsers;
	CStatusBarCtrl ctrlStatus;

	tstring filter;

	bool closed;
	bool showUsers;
	static bool closing;
	bool logMainChat;

	TStringMap tabParams;
	bool tabMenuShown;

	UserMap userMap;
	TaskQueue tasks;
	bool updateUsers;
	bool resort;

	StringMap ucLineParams;

	enum { MAX_CLIENT_LINES = 5 };
	TStringList lastLinesList;
	tstring lastLines;
	CToolTipCtrl ctrlLastLines;

	static int columnIndexes[COLUMN_LAST];
	static int columnSizes[COLUMN_LAST];

	char *resolveBuffer;
	bool isIP;
	
	bool updateUser(const UserTask& u);
	void removeUser(const User::Ptr& aUser);

	void updateUserList(UserInfo* ui = NULL);
	bool parseFilter(FilterModes& mode, int64_t& size);
	bool matchFilter(const UserInfo& ui, int sel, bool doSizeCompare = false, FilterModes mode = NONE, int64_t size = 0);
	UserInfo* findUser(const tstring& nick);

	void addAsFavorite();
	void removeFavoriteHub();

	void clearUserList();
	void clearTaskList();

	int getImage(const Identity& u);

	void updateStatusBar() { if(m_hWnd) speak(STATS); }

	void openLinksInTopic();
	bool resolve(const wstring& aDns);

	// TimerManagerListener
	virtual void on(TimerManagerListener::Second, DWORD /*aTick*/) throw();

	// ClientListener
	virtual void on(Connecting, Client*) throw();
	virtual void on(Connected, Client*) throw();
	virtual void on(UserUpdated, Client*, const OnlineUser&) throw();
	virtual void on(UsersUpdated, Client*, const OnlineUser::List&) throw();
	virtual void on(UserRemoved, Client*, const OnlineUser&) throw();
	virtual void on(Redirect, Client*, const string&) throw();
	virtual void on(Failed, Client*, const string&) throw();
	virtual void on(GetPassword, Client*) throw();
	virtual void on(HubUpdated, Client*) throw();
	virtual void on(Message, Client*, const OnlineUser&, const string&) throw();
	virtual void on(StatusMessage, Client*, const string&) throw();
	virtual void on(PrivateMessage, Client*, const OnlineUser&, const OnlineUser&, const OnlineUser&, const string&) throw();
	virtual void on(NickTaken, Client*) throw();
	virtual void on(SearchFlood, Client*, const string&) throw();

	void speak(Tasks s) { tasks.add(s, 0); PostMessage(WM_SPEAKER); }
	void speak(Tasks s, const string& msg) { tasks.add(s, new StringTask(msg)); PostMessage(WM_SPEAKER); }
	void speak(Tasks s, const OnlineUser& u) { tasks.add(s, new UserTask(u)); updateUsers = true; }
	void speak(const OnlineUser& from, const OnlineUser& to, const OnlineUser& replyTo, const string& line) { tasks.add(PRIVATE_MESSAGE, new PMTask(from, to, replyTo, line)); PostMessage(WM_SPEAKER); }
};

#endif // !defined(HUB_FRAME_H)
