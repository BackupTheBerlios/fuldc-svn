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

#include "HubFrame.h"
#include "LineDlg.h"
#include "SearchFrm.h"
#include "PrivateFrame.h"
#include "PopupManager.h"
#include "TextFrame.h"

#include "../client/QueueManager.h"
#include "../client/ShareManager.h"
#include "../client/Util.h"
#include "../client/StringTokenizer.h"
#include "../client/HubManager.h"
#include "../client/LogManager.h"

HubFrame::FrameMap HubFrame::frames;

int HubFrame::columnSizes[] = { 100, 75, 75, 100, 75, 75, 100};
int HubFrame::columnIndexes[] = { COLUMN_NICK, COLUMN_SHARED, COLUMN_DESCRIPTION, COLUMN_ISP, COLUMN_TAG, COLUMN_CONNECTION, COLUMN_EMAIL };
static ResourceManager::Strings columnNames[] = { ResourceManager::NICK, ResourceManager::SHARED,
ResourceManager::DESCRIPTION, ResourceManager::ISP, ResourceManager::TAG,
ResourceManager::CONNECTION, ResourceManager::EMAIL };


LRESULT HubFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);

	ctrlClient.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_VSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY, WS_EX_CLIENTEDGE);

	ctrlClient.LimitText(0);
	ctrlClient.SetFont(WinUtil::font);
	ctrlClient.SetBackgroundColor(WinUtil::bgColor);
	ctrlClient.SetTextColor(WinUtil::textColor);
	clientContainer.SubclassWindow(ctrlClient.m_hWnd);
	
	ctrlMessage.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL |
		ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE, WS_EX_CLIENTEDGE);
	
	ctrlMessageContainer.SubclassWindow(ctrlMessage.m_hWnd);
	ctrlMessage.SetFont(WinUtil::font);

	ctrlFilter.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_AUTOHSCROLL, WS_EX_CLIENTEDGE);
	
	ctrlFilterContainer.SubclassWindow(ctrlFilter.m_hWnd);
	ctrlFilter.SetFont(WinUtil::font);
	
	ctrlFilterSel.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL |
		WS_VSCROLL | CBS_DROPDOWNLIST, WS_EX_CLIENTEDGE);
	
	ctrlFilterSelContainer.SubclassWindow(ctrlFilterSel.m_hWnd);
	ctrlFilterSel.SetFont(WinUtil::font);

	ctrlUsers.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_USERS);
	
	if(BOOLSETTING(FULL_ROW_SELECT)) {
		ctrlUsers.SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
	} else {
		ctrlUsers.SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP);
	}

	SetSplitterPanes(ctrlClient.m_hWnd, ctrlUsers.m_hWnd, false);
	SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	m_nProportionalPos = 7500;

	ctrlShowUsers.Create(ctrlStatus.m_hWnd, rcDefault, "+/-", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	ctrlShowUsers.SetButtonStyle(BS_AUTOCHECKBOX, false);
	ctrlShowUsers.SetFont(WinUtil::systemFont);
	ctrlShowUsers.SetCheck(showUserList);
	showUsersContainer.SubclassWindow(ctrlShowUsers.m_hWnd);

	WinUtil::splitTokens(columnIndexes, SETTING(HUBFRAME_ORDER), COLUMN_LAST);
	WinUtil::splitTokens(columnSizes, SETTING(HUBFRAME_WIDTHS), COLUMN_LAST);
	
	for(int j=0; j<COLUMN_LAST; j++) {
		int fmt = (j == COLUMN_SHARED) ? LVCFMT_RIGHT : LVCFMT_LEFT;
		ctrlUsers.insertColumn(j, STRING_I(columnNames[j]), fmt, columnSizes[j], j);
	}
	
	ctrlUsers.setColumnOrderArray(COLUMN_LAST, columnIndexes);
	ctrlUsers.setVisible(SETTING(HUBFRAME_VISIBLE));
	
	ctrlUsers.SetBkColor(WinUtil::bgColor);
	ctrlUsers.SetTextBkColor(WinUtil::bgColor);
	ctrlUsers.SetTextColor(WinUtil::textColor);
	
	ctrlUsers.setSortColumn(COLUMN_NICK);
				
	ctrlUsers.SetImageList(WinUtil::userImages, LVSIL_SMALL);

	CToolInfo ti(TTF_SUBCLASS, ctrlStatus.m_hWnd);
	
	ctrlLastLines.Create(ctrlStatus.m_hWnd, rcDefault, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, WS_EX_TOPMOST);
	ctrlLastLines.AddTool(&ti);

	copyMenu.CreatePopupMenu();
	for(int i = 0; i < COLUMN_LAST; ++i)
		copyMenu.AppendMenu(MF_STRING, IDC_COPY+1+i, CSTRING_I(columnNames[i]));

	userMenu.CreatePopupMenu();
	appendUserItems(userMenu);
	userMenu.AppendMenu(MF_STRING, IDC_SHOWLOG, CSTRING(SHOW_LOG));
	userMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)copyMenu, CSTRING(COPY));
	userMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	userMenu.AppendMenu(MF_STRING, IDC_REFRESH, CSTRING(REFRESH_USER_LIST));
	userMenu.SetMenuDefaultItem(IDC_GETLIST);

	tabMenu = CreatePopupMenu();
	tabMenu.AppendMenu(MF_STRING, IDC_SHOW_HUB_LOG, CSTRING(SHOW_LOG));
	tabMenu.AppendMenu(MF_STRING, IDC_OPEN_LOG_DIR, CSTRING(OPEN_LOG_DIR));
	tabMenu.AppendMenu(MF_STRING, IDC_ADD_AS_FAVORITE, CSTRING(ADD_TO_FAVORITES));
	tabMenu.AppendMenu(MF_STRING, ID_FILE_RECONNECT, CSTRING(MENU_RECONNECT));

	searchMenu.CreatePopupMenu();
	
	
	mcMenu.CreatePopupMenu();
	mcMenu.AppendMenu(MF_STRING, IDC_COPY, CSTRING(COPY));
	mcMenu.AppendMenu(MF_STRING, IDC_SEARCH, CSTRING(SEARCH));
	mcMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)searchMenu, CSTRING(SEARCH_SITES));
	

	showJoins = BOOLSETTING(SHOW_JOINS);

	m_hMenu = WinUtil::mainMenu;

	bHandled = FALSE;
	client->connect();
	
	string nick;
	if(!stripIsp)
		nick = client->getNick();
	else {
		int pos = client->getNick().find("]");
		if(pos != string::npos) {
			nick = client->getNick().substr(pos+1);
		} else {
			nick = client->getNick();
		}
	}

	ctrlClient.SetNick(nick);
	ctrlClient.StripIsp(stripIsp);

	for(int j=0; j<COLUMN_LAST; j++) {
		ctrlFilterSel.AddString(CSTRING_I(columnNames[j]));
	}
	ctrlFilterSel.SetCurSel(0);
	

	WinUtil::SetIcon(m_hWnd, "hub.ico");

	FavoriteHubEntry *fhe = HubManager::getInstance()->getFavoriteHubEntry(server);
	if(fhe != NULL){
		showJoins = fhe->getShowJoins();
		showUserList = fhe->getShowUserlist();
		ctrlShowUsers.SetCheck(showUserList ? BST_CHECKED : BST_UNCHECKED);
		

		//retrieve window position
		CRect rc(fhe->getLeft(), fhe->getTop(), fhe->getRight(), fhe->getBottom());
		
		//if we don't have any window position stored, return so we don't
		//set window size to zero 
		if(rc.top == 0 && rc.bottom == 0 && rc.left == 0 && rc.right == 0)
			return 1;		

		MoveWindow(rc, TRUE);
	}

	return 1;
}

void HubFrame::openWindow(const string& aServer, const string& aNick /* = Util::emptyString */, const string& aPassword /* = Util::emptyString */, const string& aDescription /* = Util::emptyString */, bool aStripIsp) {
	FrameIter i = frames.find(aServer);
	if(i == frames.end()) {
		HubFrame* frm = new HubFrame(aServer, aNick, aPassword, aDescription, aStripIsp);
		frames[aServer] = frm;
		frm->CreateEx(WinUtil::mdiClient);
	} else {
		i->second->MDIActivate(i->second->m_hWnd);
	}
}

void HubFrame::onEnter() {
	char* message;
	
	if(ctrlMessage.GetWindowTextLength() > 0) {
		message = new char[ctrlMessage.GetWindowTextLength()+1];
		ctrlMessage.GetWindowText(message, ctrlMessage.GetWindowTextLength()+1);
		string s(message, ctrlMessage.GetWindowTextLength());
		delete[] message;

		// save command in history, reset current buffer pointer to the newest command
		curCommandPosition = prevCommands.size();		//this places it one position beyond a legal subscript
		if (!curCommandPosition || curCommandPosition > 0 && prevCommands[curCommandPosition - 1] != s) {
			++curCommandPosition;
			prevCommands.push_back(s);
		}
		currentCommand = "";

		// Special command
		if(s[0] == '/') {
			string cmd = s;
			string param;
			string message;
			string status;
			if(WinUtil::checkCommand(cmd, param, message, status)) {
				if(!message.empty()) {
					client->hubMessage(message);
				}
				if(!status.empty()) {
					addClientLine(status);
				}
			} else if(Util::stricmp(cmd.c_str(), "join")==0) {
				if(!param.empty()) {
					redirect = param;
					BOOL whatever = FALSE;
					onFollow(0, 0, 0, whatever);
				} else {
					addClientLine(STRING(SPECIFY_SERVER));
				}
			} else if(Util::stricmp(cmd.c_str(), "clear") == 0) {
				ctrlClient.SetWindowText("");
			} else if(Util::stricmp(cmd.c_str(), "ts") == 0) {
				timeStamps = !timeStamps;
				if(timeStamps) {
					addClientLine(STRING(TIMESTAMPS_ENABLED));
				} else {
					addClientLine(STRING(TIMESTAMPS_DISABLED));
				}
			} else if( (Util::stricmp(cmd.c_str(), "password") == 0) && waitingForPW ) {
				client->setPassword(param);
				client->password(param);
				waitingForPW = false;
			} else if( Util::stricmp(cmd.c_str(), "showjoins") == 0 ) {
				showJoins = !showJoins;
				if(showJoins) {
					addClientLine(STRING(JOIN_SHOWING_ON));
				} else {
					addClientLine(STRING(JOIN_SHOWING_OFF));
				}
			} else if(Util::stricmp(cmd.c_str(), "close") == 0) {
				PostMessage(WM_CLOSE);
			} else if(Util::stricmp(cmd.c_str(), "userlist") == 0) {
				ctrlShowUsers.SetCheck(showUserList ? BST_UNCHECKED : BST_CHECKED);
			} else if(Util::stricmp(cmd.c_str(), "connection") == 0) {
				addClientLine((STRING(IP) + client->getLocalIp() + ", " + STRING(PORT) + Util::toString(SETTING(IN_PORT))));
			} else if((Util::stricmp(cmd.c_str(), "favorite") == 0) || (Util::stricmp(cmd.c_str(), "fav") == 0)) {
				addAsFavorite();
			} else if(Util::stricmp(cmd.c_str(), "getlist") == 0){
				UserInfo * ui = findUser(param);
				if(ui != NULL)
					ui->getList();
			} else if(Util::stricmp(cmd.c_str(), "grant") == 0) {
				UserInfo* ui = findUser(param);
				if(ui != NULL){
					ui->grant();
					addClientLine(ui->getText(COLUMN_NICK) + "granted");
				}
			} else if(Util::stricmp(cmd.c_str(), "help") == 0) {
				addLine("*** " + WinUtil::commands + ", /join <hub-ip>, /clear, /ts, /showjoins, /close, /userlist, /connection, /favorite, /pm <user> [message]");
			} else if(Util::stricmp(cmd.c_str(), "pm") == 0) {
				string nick, msg;
				string::size_type j = param.find(' ');
				if(j != string::npos ){
					nick = param.substr(0, j);
					msg = param.substr(j+1);
				} else {
					nick = param;
				}

				UserInfo *ui = findUser(nick);
				
				if(ui != NULL){
					if(!msg.empty()){
						if (BOOLSETTING(POPUP_PMS) ) {
							PrivateFrame::openWindow(ui->user, msg);
						} else {
							ui->user->privateMessage("<" + client->getNick() + "> " + msg);
						}
					} else
						PrivateFrame::openWindow(ui->user);
				}
			}else if(Util::stricmp(cmd.c_str(), "topic") == 0) {
				openLinksInTopic();
			} else if(Util::stricmp(cmd.c_str(), "popups") == 0) {
				if(Util::stricmp(param.c_str(), "on") == 0) {
					PopupManager::getInstance()->Mute(false);
					addClientLine(CSTRING(POPUPS_ACTIVATED), BOOLSETTING(STATUS_IN_CHAT));
				} else if(Util::stricmp(param.c_str(), "off") == 0) {
					PopupManager::getInstance()->Mute(true);
					addClientLine(CSTRING(POPUPS_DEACTIVATED), BOOLSETTING(STATUS_IN_CHAT));
				}
			} else if(Util::stricmp(cmd.c_str(), "lastseen") == 0) {
				if(!param.empty()){
					if(!ctrlClient.LastSeen(param))
						addClientLine(CSTRING(LASTSEEN_ERROR));
				}
			}else if(Util::stricmp(cmd.c_str(), "lastlog") == 0) {
				string * str = new string;
				if(param.empty())
					ctrlClient.LastLog(str);
				else
					ctrlClient.LastLog(str, Util::toInt(param));
				TextFrame::openWindow(*str, false);
				delete str;
			} else {
				if (BOOLSETTING(SEND_UNKNOWN_COMMANDS)) {
					client->hubMessage(s);
				} else {
					addClientLine(STRING(UNKNOWN_COMMAND) + cmd);
				}
			}
		} else {
			client->hubMessage(s);
		}
		ctrlMessage.SetWindowText("");
	} else {
		MessageBeep(MB_ICONEXCLAMATION);
	}
}

struct CompareItems {
	CompareItems(int aCol) : col(aCol) { }
	bool operator()(const HubFrame::UserInfo& a, const HubFrame::UserInfo& b) const {
		return HubFrame::UserInfo::compareItems(&a, &b, col) == -1;
	}
	const int col;
};

int HubFrame::findUser(const User::Ptr& aUser) {
	for(UserMap::iterator i = usermap.begin(); i != usermap.end(); ++i){
		if(Util::stricmp(aUser->getNick(), i->second->user->getNick()) == 0)
			break;
	}
	
	if(i == usermap.end())
		return -1;

	if(ctrlUsers.getSortColumn() == COLUMN_NICK) {
		// Sort order of the other columns changes too late when the user's updated
		UserInfo* ui = i->second;
		dcassert(ctrlUsers.getItemData(ctrlUsers.getSortPos(ui)) == ui);
		return ctrlUsers.getSortPos(ui);
	}
	return ctrlUsers.findItem(i->second);
}

void HubFrame::addAsFavorite() {
	FavoriteHubEntry aEntry;
	char buf[256];
	this->GetWindowText(buf, 255);
	aEntry.setServer(server);
	aEntry.setName(buf);
	aEntry.setDescription(buf);
	aEntry.setConnect(TRUE);
	aEntry.setNick(client->getNick());
	HubManager::getInstance()->addFavorite(aEntry);
	addClientLine(STRING(FAVORITE_HUB_ADDED));
}

LRESULT HubFrame::onCopyNick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i=-1;
	if(client->isConnected()) {
		string nicks;

		while( (i = ctrlUsers.GetNextItem(i, LVNI_SELECTED)) != -1) {
			nicks += (ctrlUsers.getItemData(i))->user->getNick();
			nicks += ' ';
		}
		if(!nicks.empty()) {
			// remove last space
			nicks.erase(nicks.length() - 1);
			WinUtil::copyToClipboard(nicks);
		}
	}
	return 0;
}

LRESULT HubFrame::onDoubleClickUsers(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;
	if(client->isConnected() && item->iItem != -1) {
		ctrlUsers.getItemData(item->iItem)->getList();
	}
	return 0;
}


bool HubFrame::updateUser(const User::Ptr& u) {
	//int i = findUser(u);
	int i = -1;
	string nick;
	if(stripIsp)
		nick = u->getShortNick();
	else
		nick = u->getNick();

	while( ( i = ctrlUsers.findItem(nick, i) ) != -1 ) {
		UserInfo* ui = (UserInfo*)ctrlUsers.GetItemData(i);
		if( Util::stricmp(u->getNick(), ui->user->getNick()) == 0) {
			bool resort = (ui->getOp() != u->isSet(User::OP));
			ctrlUsers.getItemData(i)->update();
			ctrlUsers.updateItem(i);
			ctrlUsers.SetItem(i, 0, LVIF_IMAGE, NULL, getImage(u), 0, 0, NULL);
			if(resort)
				ctrlUsers.resort();
			return false;
		}
	
	}

	UserMap::iterator j = usermap.begin();
	for(; j != usermap.end(); ++j) {
		if(Util::stricmp(u->getNick(), j->second->user->getNick()) == 0) {
			j->second->update();
			return false;
		}

	}
	
	UserInfo *ui = new UserInfo(u, stripIsp);
	usermap.insert( UserPair(Util::toLower(u->getShortNick()), ui) );
	bool add = false;
	
	if(filter.empty()){
		add = true;
	}else {
		if((Util::findSubString(ui->getText(ctrlFilterSel.GetCurSel()), filter) != string::npos)) {
			add = true;
		}
	}
	
	if( add ){
		ctrlUsers.insertItem(ui, getImage(u));
	}

	return true;
}

LRESULT HubFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if(wParam == UPDATE_USERS) {
		ctrlUsers.SetRedraw(FALSE);
		{
			Lock l(updateCS);
			for(UpdateIter i = updateList.begin(); i != updateList.end(); ++i) {
				User::Ptr& u = i->first;
				switch(i->second) {
				case UPDATE_USER:
					if(updateUser(u)) {
						if(showJoins)
							addLine("*** " + STRING(JOINS) + (stripIsp ? u->getShortNick() : u->getNick()));
					} 
					break;
				case UPDATE_USERS:
					updateUser(u);
					break;
				case REMOVE_USER:
					removeUser(u);
					break;
				}
			}
			updateList.clear();
		}
		if(ctrlUsers.getSortColumn() != COLUMN_NICK)
			ctrlUsers.resort();
		ctrlUsers.SetRedraw(TRUE);
	} else if(wParam == DISCONNECTED) {
		clearUserList();
		setDisconnected(true);
	} else if(wParam == CONNECTED) {
		addClientLine(STRING(CONNECTED));
		setDisconnected(false);
	} else if(wParam == ADD_CHAT_LINE) {
		string* x = (string*)lParam;
		addLine(*x);
		delete x;
	} else if(wParam == ADD_STATUS_LINE) {
		string* x = (string*)lParam;
		addClientLine(*x);
		delete x;
	} else if(wParam == ADD_SILENT_STATUS_LINE) {
		string* x = (string*)lParam;
		addClientLine(*x, false);
		delete x;
	} else if(wParam == SET_WINDOW_TITLE) {
		string* x = (string*)lParam;
		SetWindowText(x->c_str());
		delete x;
	} else if(wParam == STATS) {
		ctrlStatus.SetText(1, (Util::toString(client->getUserCount()) + " " + STRING(HUB_USERS)).c_str());
		ctrlStatus.SetText(2, Util::formatBytes(client->getAvailable()).c_str());
		if(client->getUserCount() > 0)
			ctrlStatus.SetText(3, (Util::formatBytes(client->getAvailable() / client->getUserCount()) + "/" + CSTRING(USER)).c_str());
		else
			ctrlStatus.SetText(3, "");
	} else if(wParam == GET_PASSWORD) {
		if(client->getPassword().size() > 0) {
			client->password(client->getPassword());
			addClientLine(STRING(STORED_PASSWORD_SENT));
		} else {
			ctrlMessage.SetWindowText("/password ");
			ctrlMessage.SetFocus();
			ctrlMessage.SetSel(10, 10);
			waitingForPW = true;
		}
	} else if(wParam == PRIVATE_MESSAGE) {
		PMInfo* i = (PMInfo*)lParam;
		if(i->user->isOnline()) {
			if(BOOLSETTING(POPUP_PMS) || PrivateFrame::isOpen(i->user)) {
				PrivateFrame::gotMessage(i->user, i->msg);
			} else {
				addLine(STRING(PRIVATE_MESSAGE_FROM) + i->user->getNick() + ": " + i->msg);
			}
		} else {
			if(BOOLSETTING(IGNORE_OFFLINE)) {
				addClientLine(STRING(IGNORED_MESSAGE) + i->msg, false);
			} else if(BOOLSETTING(POPUP_OFFLINE)) {
				PrivateFrame::gotMessage(i->user, i->msg);
			} else {
				addLine(STRING(PRIVATE_MESSAGE_FROM) + i->user->getNick() + ": " + i->msg);
			}
		}
		delete i;
	}

	return 0;
};

void HubFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */) {
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);
	
	if(ctrlStatus.IsWindow()) {
		CRect sr;
		int w[5];
		ctrlStatus.GetClientRect(sr);

		int tmp = (sr.Width()) > 332 ? 232 : ((sr.Width() > 132) ? sr.Width()-100 : 32);
		
		w[0] = sr.right - tmp -30;
		w[1] = w[0] + (tmp-100)/2;
		w[2] = w[0] + (tmp-100);
		w[3] = w[2] + 96;
		w[4] = w[3] + 16;
		
		ctrlStatus.SetParts(5, w);

		ctrlLastLines.SetMaxTipWidth(w[0]);
		ctrlLastLines.SetWindowPos(HWND_TOPMOST, sr.left, sr.top, sr.Width(), sr.Height(), SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

		// Strange, can't get the correct width of the last field...
		ctrlStatus.GetRect(3, sr);
		sr.left = sr.right + 2;
		sr.right = sr.left + 16;
		ctrlShowUsers.MoveWindow(sr);
	}
	int h = WinUtil::fontHeight + 4;

	CRect rc = rect;
	rc.bottom -= h + 10;
	if(!showUserList) {
		if(GetSinglePaneMode() == SPLIT_PANE_NONE)
			SetSinglePaneMode(SPLIT_PANE_LEFT);
	} else {
		if(GetSinglePaneMode() != SPLIT_PANE_NONE)
			SetSinglePaneMode(SPLIT_PANE_NONE);
	}
	SetSplitterRect(rc);
	
	rc = rect;
	rc.bottom -= 2;
	rc.top = rc.bottom - h - 5;
	rc.left +=2;
	rc.right -= showUserList ? 202 : 2;
	ctrlMessage.MoveWindow(rc);

	if(showUserList){
		//borrowed some values from odc =)
		rc.left = rc.right + 4;
		rc.right = rc.left + 116;
		ctrlFilter.MoveWindow(rc);

		rc.left = rc.right + 4;
		rc.right = rc.left + 76;
		rc.top = rc.top + 0;
		rc.bottom = rc.bottom + 120;
		ctrlFilterSel.MoveWindow(rc);
	}
}

LRESULT HubFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if(!closed) {
		if(!closing && BOOLSETTING(HUBFRAME_CONFIRMATION)) {
			int ret = MessageBox("Do you really want to close this hub?", "Confirmation", MB_YESNO | MB_ICONQUESTION);
			if(ret == IDNO)
				return 0;
		}
		TimerManager::getInstance()->removeListener(this);
		client->removeListener(this);
		client->disconnect();

		closed = true;

		clearUserList();

		PostMessage(WM_CLOSE);
		return 0;
	} else {
		SettingsManager::getInstance()->set(SettingsManager::GET_USER_INFO, showUserList);

		int i = 0;
		int j = ctrlUsers.GetItemCount();
		while(i < j) {
			delete ctrlUsers.getItemData(i);
			i++;
		}

		ctrlUsers.saveHeaderOrder(SettingsManager::HUBFRAME_ORDER, SettingsManager::HUBFRAME_WIDTHS,
			SettingsManager::HUBFRAME_VISIBLE);

		FavoriteHubEntry *fhe = HubManager::getInstance()->getFavoriteHubEntry(server);
		if(fhe != NULL){
			CRect rc;
			
			//Get position of window
			GetWindowRect(&rc);
			
			//convert the position so it's relative to main window
			::ScreenToClient(GetParent(), &rc.TopLeft());
			::ScreenToClient(GetParent(), &rc.BottomRight());
			
			//save the position
			fhe->setBottom((u_int16_t)(rc.bottom > 0 ? rc.bottom : 0));
			fhe->setTop((u_int16_t)(rc.top > 0 ? rc.top : 0));
			fhe->setLeft((u_int16_t)(rc.left > 0 ? rc.left : 0));
			fhe->setRight((u_int16_t)(rc.right > 0 ? rc.right : 0));

			fhe->setShowJoins(showJoins);
			fhe->setShowUserlist(showUserList);
			HubManager::getInstance()->save();
		}

		MDIDestroy(m_hWnd);
		return 0;
	}
}

LRESULT HubFrame::onLButton(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	HWND focus = GetFocus();
	bHandled = false;
	if(focus == ctrlClient.m_hWnd) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		string x;
		string::size_type start = (string::size_type)ctrlClient.TextUnderCursor(pt, x);
		
		if( (Util::strnicmp(x.c_str() + start, "http://", 7) == 0) || 
			(Util::strnicmp(x.c_str() + start, "www.", 4) == 0) ||
			(Util::strnicmp(x.c_str() + start, "ftp://", 6) == 0) )	{

			bHandled = true;
			// Web links...
			string::size_type end = x.find(' ', start + 7);
			if(end == string::npos) {
				end = x.length();
			}
			if(end < start + 10) {
				return 0;
			}

			WinUtil::openLink(x.substr(start, end-start));
		} else if(Util::strnicmp(x.c_str() + start, "dchub://", 8) == 0) {
			bHandled = true;
			string server, file;
			short port = 411;
			Util::decodeUrl((x.c_str() + start), server, port, file);
			HubFrame::openWindow(server + ":" + Util::toString(port));
		} else {
			string::size_type end = x.find_first_of(" >\t", start+1);

			if(end == string::npos) // get EOL as well
				end = x.length();
			else if(end == start + 1)
				return 0;

			// Nickname click, let's see if we can find one like it in the name list...
			int pos = ctrlUsers.findItem(x.substr(start, end - start));
			if(pos != -1) {
				bHandled = true;
				if (wParam & MK_CONTROL) { // MK_CONTROL = 0x0008
					PrivateFrame::openWindow((ctrlUsers.getItemData(pos))->user);
				} else if (wParam & MK_SHIFT) {
					try {
						QueueManager::getInstance()->addList((ctrlUsers.getItemData(pos))->user, QueueItem::FLAG_CLIENT_VIEW);
					} catch(const Exception& e) {
						addClientLine(e.getError());
					}
				} else {
					int items = ctrlUsers.GetItemCount();
					ctrlUsers.SetRedraw(FALSE);
					for(int i = 0; i < items; ++i) {
						ctrlUsers.SetItemState(i, (i == pos) ? LVIS_SELECTED | LVIS_FOCUSED : 0, LVIS_SELECTED | LVIS_FOCUSED);
					}
					ctrlUsers.SetRedraw(TRUE);
					ctrlUsers.EnsureVisible(pos, FALSE);
				}
			}
		}
	}
	return 0;
}

void HubFrame::addLine(string& aLine) {
	if(BOOLSETTING(LOG_MAIN_CHAT)) {
		StringMap params;
		params["message"] = aLine;
		if(BOOLSETTING(ROTATE_LOG)) {
			LOGMC(client->getAddressPort(), Util::formatParams(SETTING(LOG_FORMAT_MAIN_CHAT), params));
		} else {
			LOG(client->getAddressPort(), Util::formatParams(SETTING(LOG_FORMAT_MAIN_CHAT), params));
		}
	}
	
	if(ctrlClient.AddLine(aLine, timeStamps)) 
		setNotify();
	
	setDirty();
}

LRESULT HubFrame::onTabContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
	tabMenuShown = true;
	prepareMenu(tabMenu, ::UserCommand::CONTEXT_HUB, client->getAddressPort(), client->getOp());
	tabMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	tabMenu.AppendMenu(MF_STRING, IDC_CLOSE_WINDOW, CSTRING(CLOSE));
	tabMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
	tabMenu.DeleteMenu(tabMenu.GetMenuItemCount()-1, MF_BYPOSITION);
	tabMenu.DeleteMenu(tabMenu.GetMenuItemCount()-1, MF_BYPOSITION);
	cleanMenu(tabMenu);
	return TRUE;
}

LRESULT HubFrame::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	RECT rc; 
	POINT pt; 
	GetCursorPos(&pt);			//need cursor pos
	ctrlClient.GetWindowRect(&rc);

	bool doMenu = false;
	bool doMcMenu = false;
	
	WinUtil::AppendSearchMenu(searchMenu);

	if (PtInRect(&rc, pt)) {
		string x;
		ctrlClient.ScreenToClient(&pt);
		string::size_type start = (string::size_type)ctrlClient.TextUnderCursor(pt, x);

		string::size_type end = x.find_first_of(" >\t", start+1);
		if(end == string::npos) // get EOL as well
			end = x.length();
		else if(end == start + 1) {
			bHandled = FALSE;
			return FALSE;
		}

		// Nickname click, let's see if we can find one like it in the name list...
		int pos = ctrlUsers.findItem(x.substr(start, end - start));
		if(pos != -1) {
			int items = ctrlUsers.GetItemCount();
			ctrlUsers.SetRedraw(FALSE);
			for(int i = 0; i < items; ++i) {
				ctrlUsers.SetItemState(i, (i == pos) ? LVIS_SELECTED | LVIS_FOCUSED : 0, LVIS_SELECTED | LVIS_FOCUSED);
			}
			ctrlUsers.SetRedraw(TRUE);
			ctrlUsers.EnsureVisible(pos, FALSE);

			ctrlClient.ClientToScreen(&pt);
			doMenu = true; 
		} else {
			doMcMenu = true;
		}
	} else {
		// Get the bounding rectangle of the client area. 
		ctrlUsers.GetWindowRect(&rc);
		
		CRect rc2;
		ctrlUsers.GetHeader().GetWindowRect(&rc2);
		
		if (PtInRect(&rc2, pt)) {
			ctrlUsers.showMenu(pt);
			return TRUE;
		}else if (PtInRect(&rc, pt)) { 
			doMenu = true;
		}else{
			bHandled = FALSE; //needed to popup context menu under userlist
		}
	}

	if(doMenu) {
		tabMenuShown = false;
		prepareMenu(userMenu, ::UserCommand::CONTEXT_CHAT, client->getAddressPort(), client->getOp());
		userMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		cleanMenu(userMenu);
		return TRUE;
	}

	if(doMcMenu) {
		CHARRANGE cr;
		ctrlClient.GetSel(cr);
		if(cr.cpMax != cr.cpMin) {
			char *buf = new char[cr.cpMax - cr.cpMin + 1];
			ctrlClient.GetSelText(buf);
			searchTerm = buf;
			delete[] buf;
		} else {
			string line;
			int start = ctrlClient.TextUnderCursor(pt, line);
			if( start != string::npos ) {
				int end = line.find_first_of(" \t\r\n", start+1);
				if(end == string::npos)
					end = line.length();
				searchTerm = line.substr(start, end-start);
			}
		}
		ctrlClient.ClientToScreen(&pt);
		mcMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		bHandled = true;
		return TRUE;
	}
	return FALSE;
}

void HubFrame::runUserCommand(::UserCommand& uc) {
	if(!WinUtil::getUCParams(m_hWnd, uc, ucParams))
		return;

	ucParams["mynick"] = client->getNick();

	if(tabMenuShown) {
		client->send(Util::formatParams(uc.getCommand(), ucParams));
	} else {
		int sel = -1;
		while((sel = ctrlUsers.GetNextItem(sel, LVNI_SELECTED)) != -1) {
			UserInfo* u = (UserInfo*) ctrlUsers.GetItemData(sel);
			u->user->getParams(ucParams);
			client->send(Util::formatParams(uc.getCommand(), ucParams));
		}
	}
	return;
};

void HubFrame::onTab() {
	if(	!BOOLSETTING(TAB_COMPLETION)) {
		HWND focus = GetFocus();

		if(focus == ctrlClient.m_hWnd) {
			ctrlMessage.SetFocus();
		} else if(focus == ctrlMessage.m_hWnd) {
			ctrlUsers.SetFocus();
		} else if(focus == ctrlUsers.m_hWnd) {
			ctrlClient.SetFocus();
		} 

		return;
	}
		
	bool up = (GetAsyncKeyState(VK_SHIFT) & 0x8000) ;

	int n = ctrlMessage.GetWindowTextLength();
	AutoArray<char> buf(n+1);
	ctrlMessage.GetWindowText(buf, n+1);
	string text(buf, n);
	string::size_type textStart = text.find_last_of(" \n\t");

	if(complete.empty()) {
		if(textStart != string::npos) {
			complete = text.substr(textStart + 1);
		} else {
			complete = text;
		}
		if(complete.empty()) {
			// Still empty, no text entered...
			return;
		}
	}

	if(textStart == string::npos)
		textStart = 0;
	else
		textStart++;

	//check if it's a command
	if(complete[0] == '/'){
		StringIter i;
		
		//if text is more than just / find the last command
		if(text.size() > 1){
			for(i = tabList.begin(); i != tabList.end(); ++i) {
				if(Util::findSubString(*i, text) == 0){
					if(up){
						if(i == tabList.begin()){
							i = tabList.end();
						}
						--i;
					} else
						++i;
					break;
				}
			}
		//otherwise set it to the beginning/end depending on which direction
		//we're searching
		} else{
			if(up){
				i = tabList.end();
				--i;
			} else
				i = tabList.begin();
		}

		//just iterate through the list once
		for(int j = 0; j < tabList.size(); ++j){
			//make sure we're not outside the list
			if(i == tabList.end()){
				i = tabList.begin();
			}
						
			if(Util::findSubString(*i, complete) == 0 ){
				ctrlMessage.SetSel(textStart, ctrlMessage.GetWindowTextLength(), TRUE);
				ctrlMessage.ReplaceSel((*i).c_str());
				return;
			}
			
			//if we didn't find a match, move the iterator and make sure
			//we're not outside the list
			if(up){
				if(i == tabList.begin())
					i = tabList.end();
				--i;
			} else
				++i;
		}
	}

	int y = ctrlUsers.GetItemCount();

	for(int x = 0; x < y; ++x)
		ctrlUsers.SetItemState(x, 0, LVNI_FOCUSED | LVNI_SELECTED);

	int end = usermap.size();
	bool found = false;

	Lock l(updateCS);

	UserMap::iterator curUser = usermap.begin();
	for(UserMap::iterator i = usermap.begin(); i != usermap.end(); ++i){
		if(Util::stricmp(curNick, i->second->user->getNick()) == 0){
			curUser = i;
			break;
		}
	}
	
	if(!up && curUser == usermap.end())
		curUser = usermap.begin();

	for(int i = 0; i < end-1; ++i){
		if(up){
			if(curUser == usermap.begin())
				curUser = usermap.end();
			--curUser;
		} else{
			++curUser;
			if(curUser == usermap.end())
				curUser = usermap.begin();
		}
		
		if( Util::findSubString(curUser->first, complete) == 0 ){
				found = true;
				break;
		}
	}

	if(found) {
		UserInfo* u = curUser->second;
		int pos = -1;
		while((pos = ctrlUsers.findItem(stripIsp ? u->user->getShortNick(): u->user->getNick(), pos)) != -1){
			UserInfo* ui = ctrlUsers.getItemData(pos);
			if(Util::stricmp(u->user->getNick(), ui->user->getNick()) == 0){
				ctrlUsers.SetItemState(pos, LVNI_FOCUSED | LVNI_SELECTED, LVNI_FOCUSED | LVNI_SELECTED);
				ctrlUsers.EnsureVisible(pos, false);
			}
		}
		ctrlMessage.SetSel(textStart, ctrlMessage.GetWindowTextLength(), TRUE);
		string tmp = u->user->getShortNick();;
		ctrlMessage.ReplaceSel(tmp.c_str());
		curNick = u->user->getNick();
		return;
	}
	
}

LRESULT HubFrame::onChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
	if(!complete.empty() && (wParam != VK_TAB && wParam != VK_SHIFT) && uMsg == WM_KEYDOWN){
		complete.clear();
		curNick.clear();
	}

	if (uMsg != WM_KEYDOWN) {
		switch(wParam) {
			case VK_RETURN:
				if( (GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000) ) {
					bHandled = FALSE;
				}
				break;
		case VK_TAB:
				bHandled = TRUE;
  				break;
  			default:
  				bHandled = FALSE;
				break;
			}
		return 0;
			}

	switch(wParam) {
		case VK_TAB:
				onTab();
			break;
		case VK_RETURN:
			if( (GetKeyState(VK_CONTROL) & 0x8000) || 
				(GetKeyState(VK_MENU) & 0x8000) ) {
					bHandled = FALSE;
				} else {
						onEnter();
					}
			break;
		case VK_UP:
			if ((GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000)) {
				//scroll up in chat command history
				//currently beyond the last command?
				if (curCommandPosition > 0) {
					//check whether current command needs to be saved
					if (curCommandPosition == prevCommands.size()) {
						auto_ptr<char> messageContents(new char[ctrlMessage.GetWindowTextLength()+2]);
						ctrlMessage.GetWindowText(messageContents.get(), ctrlMessage.GetWindowTextLength()+1);
						currentCommand = string(messageContents.get());
					}

					//replace current chat buffer with current command
					ctrlMessage.SetWindowText(prevCommands[--curCommandPosition].c_str());
				}
			} else {
				bHandled = FALSE;
			}

			break;
		case VK_DOWN:
			if ((GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000)) {
				//scroll down in chat command history

				//currently beyond the last command?
				if (curCommandPosition + 1 < prevCommands.size()) {
					//replace current chat buffer with current command
					ctrlMessage.SetWindowText(prevCommands[++curCommandPosition].c_str());
				} else if (curCommandPosition + 1 == prevCommands.size()) {
					//revert to last saved, unfinished command

					ctrlMessage.SetWindowText(currentCommand.c_str());
					++curCommandPosition;
				}
			} else {
				bHandled = FALSE;
			}

			break;
		case VK_HOME:
			if (!prevCommands.empty() && (GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000)) {
				curCommandPosition = 0;
				
				auto_ptr<char> messageContents(new char[ctrlMessage.GetWindowTextLength()+2]);
				ctrlMessage.GetWindowText(messageContents.get(), ctrlMessage.GetWindowTextLength()+1);
				currentCommand = string(messageContents.get());

				ctrlMessage.SetWindowText(prevCommands[curCommandPosition].c_str());
			} else {
				bHandled = FALSE;
			}

			break;
		case VK_END:
			if ((GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000)) {
				curCommandPosition = prevCommands.size();

				ctrlMessage.SetWindowText(currentCommand.c_str());
			} else {
				bHandled = FALSE;
				}
				break;
		default:
			bHandled = FALSE;
	}
	return 0;
}

LRESULT HubFrame::onShowUsers(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
	bHandled = FALSE;

	if(wParam == BST_CHECKED)
		showUserList = true;
	else
		showUserList = false;

	UpdateLayout(FALSE);
	return 0;
}

LRESULT HubFrame::onFollow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	
	if(!redirect.empty()) {
		string s, f;
		short p = 411;
		Util::decodeUrl(redirect, s, p, f);
		if(ClientManager::getInstance()->isConnected(s, p)) {
			addClientLine(STRING(REDIRECT_ALREADY_CONNECTED));
			return 0;
		}
		
		dcassert(frames.find(server) != frames.end());
		dcassert(frames[server] == this);
		frames.erase(server);
		server = redirect;
		frames[server] = this;

		// Is the redirect hub a favorite? Then honor settings for it.
		FavoriteHubEntry* hub = HubManager::getInstance()->getFavoriteHubEntry(server);
		if(hub) {
			client->setNick(hub->getNick(true));
			client->setDescription(hub->getUserDescription());
			client->setPassword(hub->getPassword());
		}
		// else keep current settings

		client->addListener(this);
		//client->connect(redirect);
	}
	return 0;
}

LRESULT HubFrame::onEnterUsers(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& /*bHandled*/) {
	int item = ctrlUsers.GetNextItem(-1, LVNI_FOCUSED);
	if(client->isConnected() && (item != -1)) {
		try {
			QueueManager::getInstance()->addList((ctrlUsers.getItemData(item))->user, QueueItem::FLAG_CLIENT_VIEW);
		} catch(const Exception& e) {
			addClientLine(e.getError());
		}
	}
	return 0;
}

LRESULT HubFrame::onGetToolTip(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMTTDISPINFO* nm = (NMTTDISPINFO*)pnmh;
	lastLines.clear();
	for(StringIter i = lastLinesList.begin(); i != lastLinesList.end(); ++i) {
		lastLines += *i;
		lastLines += "\r\n";
	}
	if(lastLines.size() > 2) {
		lastLines.erase(lastLines.size() - 2);
	}
	nm->lpszText = const_cast<char*>(lastLines.c_str());
	return 0;
}

void HubFrame::addClientLine(const string& aLine, bool inChat /* = true */) {
	string line = "[" + Util::getShortTimeString() + "] " + aLine;

	ctrlStatus.SetText(0, line.c_str());
	while(lastLinesList.size() + 1 > MAX_CLIENT_LINES)
		lastLinesList.erase(lastLinesList.begin());
	lastLinesList.push_back(line);

	if(BOOLSETTING(HUB_BOLD_TABS))
		setDirty();
	
	if(BOOLSETTING(STATUS_IN_CHAT) && inChat) {
		addLine("*** " + aLine);
	}

	if(BOOLSETTING(POPUP_ON_HUBSTATUS)) {
		if( aLine.find("Disconnected") != string::npos ) {
			PopupManager::getInstance()->ShowDisconnected(server);
		}
	}

}

void HubFrame::closeDisconnected() {
	for(FrameIter i=frames.begin(); i!= frames.end(); ++i) {
		if (!(i->second->client->isConnected())) {
			i->second->PostMessage(WM_CLOSE);
		}
	}
};

void HubFrame::setClosing() {
	for(FrameIter i=frames.begin(); i!= frames.end(); ++i) {
		i->second->closing = true;
	}
}

void HubFrame::on(TimerManagerListener::Second, DWORD /*aTick*/) throw() {
	updateStatusBar();
	if(updateUsers) {
		updateUsers = false;
		PostMessage(WM_SPEAKER, UPDATE_USERS);
	}
}

void HubFrame::on(Connecting, Client*) throw() { 
	speak(ADD_STATUS_LINE, STRING(CONNECTING_TO) + client->getAddressPort() + "...");
	speak(SET_WINDOW_TITLE, client->getAddressPort());
}
void HubFrame::on(Connected, Client*) throw() { 
	speak(CONNECTED);
}
void HubFrame::on(BadPassword, Client*) throw() { 
	client->setPassword(Util::emptyString);
}
void HubFrame::on(UserUpdated, Client*, const User::Ptr& user) throw() { 
	if(getUserInfo() && !user->isSet(User::HIDDEN)) 
		speak(UPDATE_USER, user);
}
void HubFrame::on(UsersUpdated, Client*, const User::List& aList) throw() {
	Lock l(updateCS);
	updateList.reserve(aList.size());
	for(User::List::const_iterator i = aList.begin(); i != aList.end(); ++i) {
		if(!(*i)->isSet(User::HIDDEN))
			updateList.push_back(make_pair(*i, UPDATE_USERS));
	}
	if(!updateList.empty()) {
		PostMessage(WM_SPEAKER, UPDATE_USERS);
	}
}

void HubFrame::on(UserRemoved, Client*, const User::Ptr& user) throw() {
	if(getUserInfo()) 
		speak(REMOVE_USER, user);
}

void HubFrame::on(Redirect, Client*, const string& line) throw() { 
	string s, f;
	short p = 411;
	Util::decodeUrl(line, s, p, f);
	if(ClientManager::getInstance()->isConnected(s, p)) {
		speak(ADD_STATUS_LINE, STRING(REDIRECT_ALREADY_CONNECTED));
		return;
	}

	redirect = line;
	if(BOOLSETTING(AUTO_FOLLOW)) {
		PostMessage(WM_COMMAND, IDC_FOLLOW, 0);
	} else {
		speak(ADD_STATUS_LINE, STRING(PRESS_FOLLOW) + line);
	}
}
void HubFrame::on(Failed, Client*, const string& line) throw() { 
	speak(ADD_STATUS_LINE, line); 
	speak(DISCONNECTED); 
}
void HubFrame::on(GetPassword, Client*) throw() { 
	speak(GET_PASSWORD);
}
void HubFrame::on(HubUpdated, Client*) throw() { 
	speak(SET_WINDOW_TITLE, Util::validateMessage(client->getNameWithTopic(), true, false) + " (" + client->getAddressPort() + ")");
}
void HubFrame::on(Message, Client*, const string& line) throw() { 
	if(SETTING(FILTER_MESSAGES)) {
		if((line.find("Hub-Security") != string::npos) && (line.find("was kicked by") != string::npos)) {
			// Do nothing...
		} else if((line.find("is kicking") != string::npos) && (line.find("because:") != string::npos)) {
			speak(ADD_SILENT_STATUS_LINE, line);
		} else {
			speak(ADD_CHAT_LINE, line);
		}
	} else {
		speak(ADD_CHAT_LINE, line);
	}
}
void HubFrame::on(PrivateMessage, Client*, const User::Ptr& user, const string& line) throw() { 
	speak(PRIVATE_MESSAGE, user, line);
}
void HubFrame::on(NickTaken, Client*) throw() { 
	speak(ADD_STATUS_LINE, STRING(NICK_TAKEN));
	speak(DISCONNECTED);
}
void HubFrame::on(SearchFlood, Client*, const string& line) throw() {
	speak(ADD_STATUS_LINE, STRING(SEARCH_SPAM_FROM) + line);
}

LRESULT HubFrame::onSearch(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	WinUtil::search(searchTerm, wID - IDC_SEARCH);
	searchTerm = Util::emptyString;
	return 0;
}

LRESULT HubFrame::onCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(wID == IDC_COPY){
		if(searchTerm.empty()){
			ctrlClient.Copy();
		} else {
			WinUtil::copyToClipboard(searchTerm);
			searchTerm = Util::emptyString;
		}
	}else {
		int id = wID - IDC_COPY -1;
		UserInfo *ui = ctrlUsers.getSelectedItem();
		if(ui != NULL)
			WinUtil::copyToClipboard(ui->getText(id));
	}

	return 0;
}

void HubFrame::openLinksInTopic() {
	char buf[512];
	GetWindowText(buf, 512);
	string topic = buf;
	int pos = -1;
	StringList urls;
	
	while( (pos = topic.find("http://", pos+1)) != string::npos ){
		int pos2 = topic.find(" ", pos+1);
		urls.push_back(topic.substr(pos, pos2-pos));
	}
	pos = -1;
	while( (pos = topic.find("www.", pos+1)) != string::npos ) {
		if(topic[pos-1] != '/') {
			int pos2 = topic.find(" ", pos+1);
			urls.push_back(topic.substr(pos, pos2-pos));
		}
	}
	for( StringIter i = urls.begin(); i != urls.end(); ++i ) {
		WinUtil::openLink((*i));
	}
}

void HubFrame::removeUser(const User::Ptr& u) {
	int j = -1;
	string nick;
	if(stripIsp)
		nick = u->getShortNick();
	else
		nick = u->getNick();

	while( ( j = ctrlUsers.findItem(nick, j) ) != -1 ) {
		UserInfo* ui = ctrlUsers.getItemData(j);
		if(Util::stricmp(u->getNick(), ui->user->getNick()) == 0 ) {
			ctrlUsers.DeleteItem(j);
			break;
		}
	}

	UserMap::iterator i = usermap.begin();
	for(; i != usermap.end(); ++i) {
		if(Util::stricmp(i->second->user->getNick(), u->getNick()) == 0){
			delete i->second;
			i->second = NULL;
			usermap.erase(i);
			
			break;
		}
	}

	if(showJoins) {
		addLine("*** " + STRING(PARTS) + nick);
	}
}

LRESULT HubFrame::onFilterChar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	char *buf = new char[ctrlFilter.GetWindowTextLength()+1];
	ctrlFilter.GetWindowText(buf, ctrlFilter.GetWindowTextLength()+1);
	filter = buf;
	delete buf;
	
	updateUserList();

	bHandled = false;

	return 0;
}

LRESULT HubFrame::onSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	char *buf = new char[ctrlFilter.GetWindowTextLength()+1];
	ctrlFilter.GetWindowText(buf, ctrlFilter.GetWindowTextLength()+1);
	filter = buf;
	delete buf;
	
	updateUserList();
	
	bHandled = false;

	return 0;
}

void HubFrame::updateUserList() {
	Lock l(updateCS);

	ctrlUsers.DeleteAllItems();

	if(filter.empty()) {
		UserMap::iterator i = usermap.begin();
		for(; i != usermap.end(); ++i){
			if(i->second != NULL)
				ctrlUsers.insertItem(i->second, getImage(i->second->user));	
		}
		return;
	}
	
	int sel = ctrlFilterSel.GetCurSel();

	UserMap::iterator i = usermap.begin();
	for(; i != usermap.end(); ++i){
		if( i->second != NULL ) {
			if(Util::findSubString(i->second->getText(sel), filter) != string::npos)
				ctrlUsers.insertItem(i->second, getImage(i->second->user));	
		}
	}
}

LRESULT HubFrame::onShowHubLog(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	string path = SETTING(LOG_DIRECTORY);
	if(BOOLSETTING(ROTATE_LOG))
		path += server;
	if(wID == IDC_SHOW_HUB_LOG){
		path += "\\";
		if(BOOLSETTING(ROTATE_LOG)){
			char buf[20];
			time_t now = time(NULL);
			strftime(buf, 20, "%Y-%m-%d ", localtime(&now));
			path += buf;
		}
		path += server + ".log";
	}
	
	ShellExecute(NULL, "open", Util::validateFileName(path).c_str(), NULL, NULL, SW_SHOWNORMAL);
	
	return 0;
}

HubFrame::UserInfo* HubFrame::findUser(string & nick){
	if( !nick.empty() ){
		UserInfo* ui = ctrlUsers.getItemData(ctrlUsers.GetNextItem(-1, LVNI_SELECTED));
		if(ui != NULL) {
			if( ui->getText(COLUMN_NICK).find(nick) != string::npos ) {
				return ui;
			}else {
				int k = ctrlUsers.findItem(nick);
				if(k != -1) {
					return ctrlUsers.getItemData(k);
				}
			}
		} else {
			int k = ctrlUsers.findItem(nick);
			if(k != -1) {
				return ctrlUsers.getItemData(k);
			}
		}
	}

	return NULL;
}



/**
 * @file
 * $Id: HubFrame.cpp,v 1.25 2004/02/21 15:15:28 trem Exp $
 */
