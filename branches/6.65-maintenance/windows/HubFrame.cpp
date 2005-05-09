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
#include "../client/AdcCommand.h"
#include "../client/version.h"
#include "../client/IgnoreManager.h"

HubFrame::FrameMap HubFrame::frames;
bool HubFrame::closing = false;

int HubFrame::columnSizes[] = { 100, 75, 75, 100, 75, 100,75, 100};
int HubFrame::columnIndexes[] = { COLUMN_NICK, COLUMN_SHARED, COLUMN_DESCRIPTION, COLUMN_ISP, COLUMN_TAG, COLUMN_IP, COLUMN_CONNECTION, COLUMN_EMAIL };
static ResourceManager::Strings columnNames[] = { ResourceManager::NICK, ResourceManager::SHARED,
ResourceManager::DESCRIPTION, ResourceManager::ISP, ResourceManager::TAG, ResourceManager::IP_BARE,
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
	ctrlUsers.SetExtendedListViewStyle(LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);

	SetSplitterPanes(ctrlClient.m_hWnd, ctrlUsers.m_hWnd, false);
	SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	m_nProportionalPos = 7500;

	ctrlShowUsers.Create(ctrlStatus.m_hWnd, rcDefault, _T("+/-"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	ctrlShowUsers.SetButtonStyle(BS_AUTOCHECKBOX, false);
	ctrlShowUsers.SetFont(WinUtil::systemFont);
	ctrlShowUsers.SetCheck(showUserList ? BST_CHECKED : BST_UNCHECKED);
	showUsersContainer.SubclassWindow(ctrlShowUsers.m_hWnd);

	FavoriteHubEntry *fhe = HubManager::getInstance()->getFavoriteHubEntry(Text::fromT(server));

	if(fhe) {
		WinUtil::splitTokens(columnIndexes, fhe->getHeaderOrder(), COLUMN_LAST);
		WinUtil::splitTokens(columnSizes, fhe->getHeaderWidths(), COLUMN_LAST);
	} else {
		WinUtil::splitTokens(columnIndexes, SETTING(HUBFRAME_ORDER), COLUMN_LAST);
		WinUtil::splitTokens(columnSizes, SETTING(HUBFRAME_WIDTHS), COLUMN_LAST);
	}
	
	for(int j=0; j<COLUMN_LAST; j++) {
		int fmt = (j == COLUMN_SHARED) ? LVCFMT_RIGHT : LVCFMT_LEFT;
		ctrlUsers.InsertColumn(j, TSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
	}
	
	ctrlUsers.setColumnOrderArray(COLUMN_LAST, columnIndexes);

	if(fhe) {
		ctrlUsers.setVisible(fhe->getHeaderVisible());
	} else {
		ctrlUsers.setVisible(SETTING(HUBFRAME_VISIBLE));
	}
	
	ctrlUsers.SetBkColor(WinUtil::bgColor);
	ctrlUsers.SetTextBkColor(WinUtil::bgColor);
	ctrlUsers.SetTextColor(WinUtil::textColor);
	
	ctrlUsers.setSortColumn(COLUMN_NICK);
				
	ctrlUsers.SetImageList(WinUtil::userImages, LVSIL_SMALL);

	CToolInfo ti(TTF_SUBCLASS, ctrlStatus.m_hWnd);
	
	ctrlLastLines.Create(ctrlStatus.m_hWnd, rcDefault, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, WS_EX_TOPMOST);
	ctrlLastLines.SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	ctrlLastLines.AddTool(&ti);

	copyMenu.CreatePopupMenu();
	ctrlUsers.buildCopyMenu(copyMenu);
	

	userMenu.CreatePopupMenu();
	appendUserItems(userMenu);
	userMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)copyMenu, CTSTRING(COPY));
	userMenu.SetMenuDefaultItem(IDC_GETLIST);

	tabMenu = CreatePopupMenu();
	tabMenu.AppendMenu(MF_STRING, IDC_SHOW_HUB_LOG, CTSTRING(SHOW_LOG));
	tabMenu.AppendMenu(MF_STRING, IDC_OPEN_LOG_DIR, CTSTRING(OPEN_LOG_DIR));
	tabMenu.AppendMenu(MF_STRING, IDC_ADD_AS_FAVORITE, CTSTRING(ADD_TO_FAVORITES));
	tabMenu.AppendMenu(MF_STRING, ID_FILE_RECONNECT, CTSTRING(MENU_RECONNECT));


	favShowJoins = BOOLSETTING(FAV_SHOW_JOINS);

	for(int j=0; j<COLUMN_LAST; j++) {
		ctrlFilterSel.AddString(CTSTRING_I(columnNames[j]));
	}
	ctrlFilterSel.SetCurSel(0);
	

	WinUtil::SetIcon(m_hWnd, _T("hub.ico"));

	bHandled = FALSE;
	client->connect();

	if(stripIsp)
		ctrlClient.setFlag(CFulEditCtrl::STRIP_ISP);

	ctrlClient.SetNick(Text::toT(client->getNick()));
	
	if(fhe != NULL){
		//retrieve window position
		CRect rc(fhe->getLeft(), fhe->getTop(), fhe->getRight(), fhe->getBottom());
		
		//check that we have a window position stored
		if(! (rc.top == 0 && rc.bottom == 0 && rc.left == 0 && rc.right == 0) )
			MoveWindow(rc, TRUE);
	}

	TimerManager::getInstance()->addListener(this);

	return 1;
}

void HubFrame::openWindow(const tstring& aServer) {
	FrameIter i = frames.find(aServer);
	if(i == frames.end()) {
		HubFrame* frm = new HubFrame(aServer);
		frames[aServer] = frm;
		frm->CreateEx(WinUtil::mdiClient);
	} else {
		if(::IsIconic(i->second->m_hWnd))
			::ShowWindow(i->second->m_hWnd, SW_RESTORE);
		i->second->MDIActivate(i->second->m_hWnd);
	}
}

void HubFrame::onEnter() {
	if(ctrlMessage.GetWindowTextLength() > 0) {
		AutoArray<TCHAR> msg(ctrlMessage.GetWindowTextLength()+1);
		ctrlMessage.GetWindowText(msg, ctrlMessage.GetWindowTextLength()+1);
		tstring s(msg, ctrlMessage.GetWindowTextLength());

		// save command in history, reset current buffer pointer to the newest command
		curCommandPosition = prevCommands.size();		//this places it one position beyond a legal subscript
		if (!curCommandPosition || curCommandPosition > 0 && prevCommands[curCommandPosition - 1] != s) {
			++curCommandPosition;
			prevCommands.push_back(s);
		}
		currentCommand = _T("");

		// Special command
		if(s[0] == _T('/')) {
			tstring cmd = s;
			tstring param;
			tstring message;
			tstring status;
			if(WinUtil::checkCommand(cmd, param, message, status)) {
				if(!message.empty()) {
					client->hubMessage(Text::fromT(message));
				}
				if(!status.empty()) {
					addClientLine(status);
				}
			} else if(Util::stricmp(cmd.c_str(), _T("join"))==0) {
				if(!param.empty()) {
					redirect = param;
					if(BOOLSETTING(SETTINGS_OPEN_NEW_WINDOW)) {
						HubFrame::openWindow(param);
					} else {
						BOOL whatever = FALSE;
						onFollow(0, 0, 0, whatever);
					}
				} else {
					addClientLine(TSTRING(SPECIFY_SERVER));
				}
			} else if(Util::stricmp(cmd.c_str(), _T("clear")) == 0) {
				ctrlClient.SetWindowText(_T(""));
			} else if(Util::stricmp(cmd.c_str(), _T("ts")) == 0) {
				int res = WinUtil::checkParam( param );
				if( 1 == res || (param.empty() && !timeStamps) ) {
					timeStamps = true;
					addClientLine(TSTRING(TIMESTAMPS_ENABLED));
				} else if( 0 == res || (param.empty() && timeStamps) ) {
					timeStamps = false;
					addClientLine(TSTRING(TIMESTAMPS_DISABLED));
				}
			} else if( (Util::stricmp(cmd.c_str(), _T("password")) == 0) && waitingForPW ) {
				client->setPassword(Text::fromT(param));
				client->password(Text::fromT(param));
				waitingForPW = false;
			} else if( Util::stricmp(cmd.c_str(), _T("showjoins")) == 0 ) {
				int res = WinUtil::checkParam( param );
				if( 1 == res || (param.empty() && !showJoins) ) {
					showJoins = true;
					addClientLine(TSTRING(JOIN_SHOWING_ON));
				} else if( 0 == res || (param.empty() && showJoins) ) {
					showJoins = false;
					addClientLine(TSTRING(JOIN_SHOWING_OFF));
				}
			} else if( Util::stricmp(cmd.c_str(), _T("favshowjoins")) == 0 ) {
				int res = WinUtil::checkParam( param );
				if( 1 == res || (param.empty() && !favShowJoins) ) {
					favShowJoins = true;
					addClientLine(TSTRING(FAV_JOIN_SHOWING_ON));
				} else if( 0 == res || (param.empty() && favShowJoins) ) {
					favShowJoins = false;
					addClientLine(TSTRING(FAV_JOIN_SHOWING_OFF));
				}
			} else if(Util::stricmp(cmd.c_str(), _T("close")) == 0) {
				PostMessage(WM_CLOSE);
			} else if(Util::stricmp(cmd.c_str(), _T("userlist")) == 0) {
				int res = WinUtil::checkParam( param );
				if( 1 == res || (param.empty() && !showUserList) ) {
					ctrlShowUsers.SetCheck(BST_CHECKED);	
				} else if( 0 == res || (param.empty() && showUserList) ) {
					ctrlShowUsers.SetCheck(BST_CHECKED);
				}
			} else if(Util::stricmp(cmd.c_str(), _T("connection")) == 0) {
				addClientLine(Text::toT((STRING(IP) + client->getLocalIp() + ", " + STRING(PORT) + Util::toString(SETTING(IN_PORT)) + "/" + Util::toString(SETTING(UDP_PORT)))));
			} else if((Util::stricmp(cmd.c_str(), _T("favorite")) == 0) || (Util::stricmp(cmd.c_str(), _T("fav")) == 0)) {
				addAsFavorite();
			} else if(Util::stricmp(cmd.c_str(), _T("getlist")) == 0){
				UserInfo * ui = findUser(param);
				if(ui != NULL)
					ui->getList();
			} else if(Util::stricmp(cmd.c_str(), _T("grant")) == 0) {
				UserInfo* ui = findUser(param);
				if(ui != NULL){
					ui->grant();
					addClientLine(ui->getText(COLUMN_NICK) + _T(" granted"));
				}
			} else if(Util::stricmp(cmd.c_str(), _T("showlog")) == 0) {
				SendMessage(WM_COMMAND, IDC_SHOW_HUB_LOG);
			} else if(Util::stricmp(cmd.c_str(), _T("pm")) == 0) {
				tstring nick, msg;
				tstring::size_type j = param.find(_T(' '));
				if(j != tstring::npos ){
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
							ui->user->privateMessage("<" + client->getNick() + "> " + Text::fromT(msg));
						}
					} else
						PrivateFrame::openWindow(ui->user);
				}
			}else if(Util::stricmp(cmd.c_str(), _T("topic")) == 0) {
				addLine(_T("*** ") + Text::toT(client->getNameWithTopic()));
			}else if(Util::stricmp(cmd.c_str(), _T("ctopic")) == 0) {
				openLinksInTopic();
			} else if(Util::stricmp(cmd.c_str(), _T("popups")) == 0) {
				int res = WinUtil::checkParam( param );
				if( 1 == res || (param.empty() && PopupManager::getInstance()->isMuted()) ) {
					PopupManager::getInstance()->Mute(false);
					addClientLine(TSTRING(POPUPS_ACTIVATED), BOOLSETTING(STATUS_IN_CHAT));
				} else if( 0 == res || (param.empty() && !PopupManager::getInstance()->isMuted()) ) {
					PopupManager::getInstance()->Mute(true);
					addClientLine(TSTRING(POPUPS_DEACTIVATED), BOOLSETTING(STATUS_IN_CHAT));
				}
			} else if(Util::stricmp(cmd.c_str(), _T("log")) == 0) {
				int res = WinUtil::checkParam(param);
				if( 1 == res || (param.empty() && !logMainChat) ) {
					logMainChat = true;
					addClientLine(TSTRING(LOGGING_ENABLED), BOOLSETTING(STATUS_IN_CHAT));
				} else if( 0 == res || (param.empty() && logMainChat) ) {
					logMainChat = false;
					addClientLine(TSTRING(LOGGING_DISABLED), BOOLSETTING(STATUS_IN_CHAT));
				}
			} else if(Util::stricmp(cmd.c_str(), _T("lastseen")) == 0) {
				if(!param.empty()){
					if(!ctrlClient.LastSeen(param))
						addClientLine(TSTRING(LASTSEEN_ERROR));
				}
			}else if(Util::stricmp(cmd.c_str(), _T("lastlog")) == 0) {
				TextFrame::openWindow(ctrlClient.LastLog());
			}else if(Util::stricmp(cmd.c_str(), _T("me")) == 0) {
				client->hubMessage(Text::fromT(s));
			} else if(Util::stricmp(cmd.c_str(), _T("dns")) == 0) {
				if( !param.empty() ) {
					if( resolve(param) )
						addClientLine(TSTRING(RESOLVING) + _T(" ") + param + _T(" ..."), BOOLSETTING(STATUS_IN_CHAT));
					else
						addClientLine(TSTRING(FAILED_RESOLVE) + _T(" ") + param, BOOLSETTING(STATUS_IN_CHAT));
				}
			} else {
				if (BOOLSETTING(SEND_UNKNOWN_COMMANDS)) {
					client->hubMessage(Text::fromT(s));
				} else {
					addClientLine(TSTRING(UNKNOWN_COMMAND) + cmd);
				}
			}
			ctrlMessage.SetWindowText(_T(""));
		} else if(waitingForPW) {
			addClientLine(TSTRING(DONT_REMOVE_SLASH_PASSWORD));
			ctrlMessage.SetWindowText(_T("/password "));
			ctrlMessage.SetFocus();
			ctrlMessage.SetSel(10, 10);
		} else {
			client->hubMessage(Text::fromT(s));
			ctrlMessage.SetWindowText(_T(""));
		}
	} else {
		MessageBeep(MB_ICONEXCLAMATION);
	}
}

struct CompareItems {
	CompareItems(int aCol) : col(aCol) { }
	bool operator()(const HubFrame::UserInfo& a, const HubFrame::UserInfo& b) const {
		return HubFrame::UserInfo::compareItems(&a, &b, col) < 0;
	}
	const int col;
};

int HubFrame::findUser(const User::Ptr& aUser) {
	for(UserIter i = usermap.begin(); i != usermap.end(); ++i){
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
	TCHAR buf[256];
	this->GetWindowText(buf, 255);
	aEntry.setServer(Text::fromT(server));
	aEntry.setName(Text::fromT(buf));
	aEntry.setDescription(Text::fromT(buf));
	aEntry.setConnect(false);
	aEntry.setNick(client->getNick());
	aEntry.setShowJoins(showJoins);
	aEntry.setShowUserlist(showUserList);
	aEntry.setStripIsp(stripIsp);
	aEntry.setLogMainChat(logMainChat);

	HubManager::getInstance()->addFavorite(aEntry);
	addClientLine(TSTRING(FAVORITE_HUB_ADDED));
}

LRESULT HubFrame::onDoubleClickUsers(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;
	if(client->isConnected() && item->iItem != -1) {
		ctrlUsers.getItemData(item->iItem)->getList();
	}
	return 0;
}


bool HubFrame::updateUser(const User::Ptr& u) {
	int i = -1;
	tstring nick = stripIsp ? Text::toT(u->getShortNick()) : Text::toT(u->getNick());
	
	while( ( i = ctrlUsers.findItem(nick, i) ) != -1 ) {
		UserInfo* ui = (UserInfo*)ctrlUsers.GetItemData(i);
		if( Util::stricmp(u->getNick(), ui->user->getNick()) == 0) {
			bool resort = (ui->getOp() != u->isSet(User::OP));
			ui->update();
			ctrlUsers.updateItem(i);
			ctrlUsers.SetItem(i, 0, LVIF_IMAGE, NULL, getImage(u), 0, 0, NULL);
			if(resort)
				ctrlUsers.resort();
			return false;
		}
	
	}

	UserIter j = usermap.begin();
	for(; j != usermap.end(); ++j) {
		if(Util::stricmp(u->getNick(), j->second->user->getNick()) == 0) {
			j->second->update();
			return false;
		}

	}
	
	UserInfo *ui = new UserInfo(u, stripIsp);
	usermap.insert( UserPair(Text::toT(Text::toLower(u->getShortNick())), ui) );
	bool add = false;
	
	if(filter.empty()){
		add = true;
	}else {
		if((ui->getText(ctrlFilterSel.GetCurSel()).find(filter) != tstring::npos)) {
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
		bool resort = false;
		{
			Lock l(updateCS);
			for(UpdateIter i = updateList.begin(); i != updateList.end(); ++i) {
				User::Ptr& u = i->first;
				switch(i->second) {
				case UPDATE_USER:
					if(updateUser(u)) {
						if(showJoins) {
							if (!favShowJoins | u->isFavoriteUser()) {
								addLine(_T("*** ") + TSTRING(JOINS) + Text::toT(stripIsp ? u->getShortNick() : u->getNick()), BOOLSETTING(HUB_BOLD_TABS));
							}
						}
					} else {
						resort = true;
					}
					break;
				case UPDATE_USERS:
					if(!updateUser(u))
						resort = true;
					
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
		addClientLine(TSTRING(CONNECTED));
		setDisconnected(false);
	} else if(wParam == ADD_CHAT_LINE) {
		tstring* x = (tstring*)lParam;
		addLine(*x);
		delete x;
	} else if(wParam == ADD_STATUS_LINE) {
		tstring* x = (tstring*)lParam;
		addClientLine(*x);
		delete x;
	} else if(wParam == ADD_SILENT_STATUS_LINE) {
		tstring* x = (tstring*)lParam;
		addClientLine(*x, false);
		delete x;
	} else if(wParam == SET_WINDOW_TITLE) {
		tstring* x = (tstring*)lParam;
		SetWindowText(x->c_str());
		delete x;
	} else if(wParam == STATS) {
		if(ctrlUsers.GetItemCount() == usermap.size())
			ctrlStatus.SetText(1, (Util::toStringW(client->getUserCount()) + _T(" ") + TSTRING(HUB_USERS)).c_str());
		else
			ctrlStatus.SetText(1, (Util::toStringW(ctrlUsers.GetItemCount()) + _T("/") + Util::toStringW(client->getUserCount()) + _T(" ") + TSTRING(HUB_USERS)).c_str());
		ctrlStatus.SetText(2, Util::formatBytesW(client->getAvailable()).c_str());
		if(client->getUserCount() > 0)
			ctrlStatus.SetText(3, (Util::formatBytesW(client->getAvailable() / client->getUserCount()) + _T("/") + CTSTRING(USER)).c_str());
		else
			ctrlStatus.SetText(3, _T(""));
	} else if(wParam == GET_PASSWORD) {
		if(client->getPassword().size() > 0) {
			client->password(client->getPassword());
			addClientLine(TSTRING(STORED_PASSWORD_SENT));
		} else {
			ctrlMessage.SetWindowText(_T("/password "));
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
				addLine(TSTRING(PRIVATE_MESSAGE_FROM) + Text::toT(i->user->getNick()) + _T(": ") + i->msg);
			}
		} else {
			if(BOOLSETTING(IGNORE_OFFLINE)) {
				addClientLine(TSTRING(IGNORED_MESSAGE) + i->msg, false);
			} else if(BOOLSETTING(POPUP_OFFLINE)) {
				PrivateFrame::gotMessage(i->user, i->msg);
			} else {
				addLine(TSTRING(PRIVATE_MESSAGE_FROM) + Text::toT(i->user->getNick()) + _T(": ") + i->msg);
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
		
		w[0] = sr.right - tmp -80;
		w[1] = w[0] + (tmp-50)/2;
		w[2] = w[0] + (tmp-50);
		w[3] = w[2] + 96;
		w[4] = w[3] + 16;
		
		ctrlStatus.SetParts(5, w);

		ctrlLastLines.SetMaxTipWidth(w[0]);

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

LRESULT HubFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	if(!closed) {
		if(!closing && BOOLSETTING(HUBFRAME_CONFIRMATION)) {
			if(IDNO == MessageBox(CTSTRING(CONFIRM_CLOSE), _T(FULDC) _T(" ") _T(FULVERSIONSTRING), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2))
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
		SettingsManager::getInstance()->set(SettingsManager::GET_USER_INFO, ctrlShowUsers.GetCheck() == BST_CHECKED);
		HubManager::getInstance()->removeUserCommand(Text::fromT(server));

		string tmp, tmp2, tmp3;
		ctrlUsers.saveHeaderOrder(tmp, tmp2, tmp3);

		FavoriteHubEntry *fhe = HubManager::getInstance()->getFavoriteHubEntry(Text::fromT(server));
		if(fhe != NULL){
			CRect rc;
			
			if(!IsIconic()){
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
			}

			fhe->setShowJoins(showJoins);
			fhe->setShowUserlist(showUserList);
			fhe->setLogMainChat(logMainChat);
			fhe->setHeaderOrder(tmp);
			fhe->setHeaderWidths(tmp2);
			fhe->setHeaderVisible(tmp3);
			

			HubManager::getInstance()->setDirty();
		} else {
			SettingsManager::getInstance()->set(SettingsManager::HUBFRAME_ORDER, tmp);
			SettingsManager::getInstance()->set(SettingsManager::HUBFRAME_WIDTHS, tmp2);
			SettingsManager::getInstance()->set(SettingsManager::HUBFRAME_VISIBLE, tmp3);
		}

		bHandled = FALSE;
		return 0;
	}
}

LRESULT HubFrame::onLButton(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	HWND focus = GetFocus();
	bHandled = false;
	if(focus == ctrlClient.m_hWnd) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		tstring x;
		tstring::size_type start = (tstring::size_type)ctrlClient.TextUnderCursor(pt, x);
		tstring::size_type end = x.find(_T(" "), start);

		if(end == string::npos)
			end = x.length();
		
		if( (Util::strnicmp(x.c_str() + start, _T("http://"), 7) == 0) || 
			(Util::strnicmp(x.c_str() + start, _T("www."), 4) == 0) ||
			(Util::strnicmp(x.c_str() + start, _T("ftp://"), 6) == 0) ||
 			(Util::strnicmp(x.c_str() + start, _T("irc://"), 6) == 0) ||
			(Util::strnicmp(x.c_str() + start, _T("https://"), 8) == 0) )	{

			bHandled = true;
			WinUtil::openLink(x.substr(start, end-start));
		} else if(Util::strnicmp(x.c_str() + start, _T("dchub://"), 8) == 0) {
			bHandled = true;
			WinUtil::parseDchubUrl(x.substr(start, end-start));
		} else {
			string::size_type end = x.find_first_of(_T(" >\t"), start+1);

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
						addClientLine(Text::toT(e.getError()));
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

void HubFrame::addLine(tstring aLine, bool bold) {
	if(logMainChat) {
		StringMap params;
		params["message"] = Text::fromT(aLine);
		params["hub"] = client->getName();
		params["hubaddr"] = client->getAddressPort();
		params["mynick"] = client->getNick(); 
		LOG(LogManager::CHAT, params);
	}
	
	if(ctrlClient.AddLine(aLine, timeStamps)) 
		setNotify();
	
	if(bold)
		setDirty();
}

LRESULT HubFrame::onTabContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
	tabMenuShown = true;
	prepareMenu(tabMenu, ::UserCommand::CONTEXT_HUB, Text::toT(client->getAddressPort()), client->getOp());
	tabMenu.AppendMenu(MF_SEPARATOR);
	tabMenu.AppendMenu(MF_STRING, IDC_CLOSE_WINDOW, CTSTRING(CLOSE));
	tabMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
	tabMenu.DeleteMenu(tabMenu.GetMenuItemCount()-1, MF_BYPOSITION);
	tabMenu.DeleteMenu(tabMenu.GetMenuItemCount()-1, MF_BYPOSITION);
	cleanMenu(tabMenu);
	return TRUE;
}

LRESULT HubFrame::onContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) }; 
	
	bool doMenu = false;
	bool doMcMenu = false;

	if(reinterpret_cast<HWND>(wParam) == ctrlClient) {
		if(pt.x == -1 && pt.y == -1) {
			WinUtil::getContextMenuPos(ctrlClient, pt);
		}

		tstring x;
		ctrlClient.ScreenToClient(&pt);
		string::size_type start = (string::size_type)ctrlClient.TextUnderCursor(pt, x);

		string::size_type end = x.find_first_of(_T(" >\t"), start+1);
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

			UserInfo* ui = (UserInfo*)ctrlUsers.getItemData(pos);
			if(IgnoreManager::getInstance()->isUserIgnored(ui->user->getNick())) {
				userMenu.EnableMenuItem(IDC_IGNORE, MF_GRAYED);
				userMenu.EnableMenuItem(IDC_UNIGNORE, MF_ENABLED);
			} else {
				userMenu.EnableMenuItem(IDC_IGNORE, MF_ENABLED);
				userMenu.EnableMenuItem(IDC_UNIGNORE, MF_GRAYED);
			}

			ctrlUsers.SetRedraw(TRUE);
			ctrlUsers.EnsureVisible(pos, FALSE);

			doMenu = true; 
			ctrlClient.ClientToScreen(&pt);
		} else {
			doMcMenu = true;
		}
	} else if(reinterpret_cast<HWND>(wParam) == ctrlUsers && ctrlUsers.GetSelectedCount() > 0) {
		if(pt.x == -1 && pt.y == -1) {
			WinUtil::getContextMenuPos(ctrlUsers, pt);
		}
		doMenu = true;
	} else {
		bHandled = FALSE;
	}
			
	if(doMenu) {
		if(ctrlUsers.GetSelectedCount() == 1) {
			int pos = ctrlUsers.GetNextItem(-1, LVNI_SELECTED);
			if(pos != -1) {
				UserInfo* ui = (UserInfo*)ctrlUsers.getItemData(pos);
				if(IgnoreManager::getInstance()->isUserIgnored(ui->user->getNick())) {
					userMenu.EnableMenuItem(IDC_IGNORE, MF_GRAYED);
					userMenu.EnableMenuItem(IDC_UNIGNORE, MF_ENABLED);
				} else {
					userMenu.EnableMenuItem(IDC_IGNORE, MF_ENABLED);
					userMenu.EnableMenuItem(IDC_UNIGNORE, MF_GRAYED);
				}
			}
		} else {
			userMenu.EnableMenuItem(IDC_IGNORE, MF_ENABLED);
			userMenu.EnableMenuItem(IDC_UNIGNORE, MF_ENABLED);
		}

		tabMenuShown = false;
		prepareMenu(userMenu, ::UserCommand::CONTEXT_CHAT, Text::toT(client->getAddressPort()), client->getOp());
		checkAdcItems(userMenu);

		userMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		cleanMenu(userMenu);
		return TRUE;
	}

	if(doMcMenu) {
		ctrlClient.ClientToScreen(&pt);
		ctrlClient.ShowMenu(m_hWnd, pt);
		bHandled = TRUE;
		return TRUE;
	}
	return FALSE;
}

void HubFrame::runUserCommand(::UserCommand& uc) {
	if(!WinUtil::getUCParams(m_hWnd, uc, ucParams))
		return;

	ucParams["mynick"] = client->getNick();
	ucParams["mycid"] = client->getMe()->getCID().toBase32();

	if(tabMenuShown) {
		client->escapeParams(ucParams);
		client->sendUserCmd(Util::formatParams(uc.getCommand(), ucParams));
	} else {
		int sel = -1;
		while((sel = ctrlUsers.GetNextItem(sel, LVNI_SELECTED)) != -1) {
			UserInfo* u = (UserInfo*) ctrlUsers.GetItemData(sel);
			StringMap tmp = ucParams;
			u->user->getParams(tmp);
			client->escapeParams(tmp);
			client->sendUserCmd(Util::formatParams(uc.getCommand(), tmp));
		}
	}
	return;
};

void HubFrame::onTab() {
	if(!BOOLSETTING(TAB_COMPLETION)) {
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
		
	bool up = ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0);

	int n = ctrlMessage.GetWindowTextLength();
	AutoArray<TCHAR> buf(n+1);
	ctrlMessage.GetWindowText(buf, n+1);
	tstring text(buf, n);
	tstring::size_type textStart = text.find_last_of(_T(" \n\t"));

	if(complete.empty()) {
		if(textStart != tstring::npos) {
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
	if(complete[0] == _T('/')){
		TStringIter i;
		
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
		for(u_int j = 0; j < tabList.size(); ++j){
			//make sure we're not outside the list
			if(i == tabList.end()){
				i = tabList.begin();
			}
						
			if(Text::toLower(*i).find(Text::toLower(complete)) == 0 ){
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

	UserIter curUser = usermap.begin();
	for(UserIter i = usermap.begin(); i != usermap.end(); ++i){
		if(Util::stricmp(curNick, Text::toT(i->second->user->getNick())) == 0){
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
		
		if( Text::toLower(curUser->first).find(Text::toLower(complete)) == 0 ){
				found = true;
				break;
		}
	}

	if(found) {
		UserInfo* u = curUser->second;
		int pos = -1;
		while((pos = ctrlUsers.findItem(stripIsp ? Text::toT(u->user->getShortNick()): Text::toT(u->user->getNick()), pos)) != -1){
			UserInfo* ui = ctrlUsers.getItemData(pos);
			if(Util::stricmp(u->user->getNick(), ui->user->getNick()) == 0){
				ctrlUsers.SetItemState(pos, LVNI_FOCUSED | LVNI_SELECTED, LVNI_FOCUSED | LVNI_SELECTED);
				ctrlUsers.EnsureVisible(pos, false);
			}
		}
		ctrlMessage.SetSel(textStart, ctrlMessage.GetWindowTextLength(), TRUE);
		tstring tmp = Text::toT(u->user->getShortNick());
		ctrlMessage.ReplaceSel(tmp.c_str());
		curNick = Text::toT(u->user->getNick());
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
			if ( (GetKeyState(VK_MENU) & 0x8000) ||	( ((GetKeyState(VK_CONTROL) & 0x8000) == 0) ^ (BOOLSETTING( SETTINGS_USE_CTRL_FOR_LINE_HISTORY ) == true) ) ) {
				//scroll up in chat command history
				//currently beyond the last command?
				if (curCommandPosition > 0) {
					//check whether current command needs to be saved
					if (curCommandPosition == prevCommands.size()) {
						TCHAR* messageContents = new TCHAR[ctrlMessage.GetWindowTextLength()+2];
						ctrlMessage.GetWindowText(messageContents.get(), ctrlMessage.GetWindowTextLength()+1);
						currentCommand = tstring(messageContents.get());
						delete[] messageContents;
					}

					//replace current chat buffer with current command
					ctrlMessage.SetWindowText(prevCommands[--curCommandPosition].c_str());
				}
				// move cursor to end of line
				ctrlMessage.SetSel(ctrlMessage.GetWindowTextLength(), ctrlMessage.GetWindowTextLength());
			} else {
				bHandled = FALSE;
			}

			break;
		case VK_DOWN:
			if ( (GetKeyState(VK_MENU) & 0x8000) ||	( ((GetKeyState(VK_CONTROL) & 0x8000) == 0) ^ (BOOLSETTING( SETTINGS_USE_CTRL_FOR_LINE_HISTORY ) == true) ) ) {
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
				// move cursor to end of line
				ctrlMessage.SetSel(ctrlMessage.GetWindowTextLength(), ctrlMessage.GetWindowTextLength());
			} else {
				bHandled = FALSE;
			}

			break;
		case VK_PRIOR: // page up
			ctrlClient.SendMessage(WM_VSCROLL, SB_PAGEUP);

			break;
		case VK_NEXT: // page down
			ctrlClient.SendMessage(WM_VSCROLL, SB_PAGEDOWN);

			break;
		case VK_HOME:
			if (!prevCommands.empty() && (GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000)) {
				curCommandPosition = 0;
				
				TCHAR* messageContents = new TCHAR[ctrlMessage.GetWindowTextLength()+2];
				ctrlMessage.GetWindowText(messageContents, ctrlMessage.GetWindowTextLength()+1);
				currentCommand = tstring(messageContents);
				delete[] messageContents;

				ctrlMessage.SetWindowText(prevCommands[curCommandPosition].c_str());
				int pos = ctrlMessage.GetWindowTextLength();
				ctrlMessage.SetSel(pos, pos);
			} else {
				bHandled = FALSE;
			}

			break;
		case VK_END:
			if ((GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000)) {
				curCommandPosition = prevCommands.size();

				ctrlMessage.SetWindowText(currentCommand.c_str());
				int pos = ctrlMessage.GetWindowTextLength();
				ctrlMessage.SetSel(pos, pos);
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
		u_int16_t p = 411;
		Util::decodeUrl(Text::fromT(redirect), s, p, f);
		if(ClientManager::getInstance()->isConnected(s, p)) {
			addClientLine(TSTRING(REDIRECT_ALREADY_CONNECTED));
			return 0;
		}
		
		dcassert(frames.find(server) != frames.end());
		dcassert(frames[server] == this);
		frames.erase(server);
		server = redirect;
		frames[server] = this;

		// the client is dead, long live the client!
		client->removeListener(this);
		ClientManager::getInstance()->putClient(client);
		clearUserList();
		client = ClientManager::getInstance()->getClient(Text::fromT(server));
		client->addListener(this);
		client->connect();
	}
	return 0;
}

LRESULT HubFrame::onEnterUsers(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& /*bHandled*/) {
	int item = ctrlUsers.GetNextItem(-1, LVNI_FOCUSED);
	if(client->isConnected() && (item != -1)) {
		try {
			QueueManager::getInstance()->addList((ctrlUsers.getItemData(item))->user, QueueItem::FLAG_CLIENT_VIEW);
		} catch(const Exception& e) {
			addClientLine(Text::toT(e.getError()));
		}
	}
	return 0;
}

LRESULT HubFrame::onGetToolTip(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMTTDISPINFO* nm = (NMTTDISPINFO*)pnmh;
	lastLines.clear();
	for(TStringIter i = lastLinesList.begin(); i != lastLinesList.end(); ++i) {
		lastLines += *i;
		lastLines += _T("\r\n");
	}
	if(lastLines.size() > 2) {
		lastLines.erase(lastLines.size() - 2);
	}
	nm->lpszText = const_cast<TCHAR*>(lastLines.c_str());
	nm->uFlags &= ~TTF_DI_SETITEM;
	return 0;
}

void HubFrame::addClientLine(const tstring& aLine, bool inChat /* = true */) {
	tstring line = _T("[") + Util::getShortTimeString() + _T("] ") + aLine;

	ctrlStatus.SetText(0, line.c_str());
	while(lastLinesList.size() + 1 > MAX_CLIENT_LINES)
		lastLinesList.erase(lastLinesList.begin());
	lastLinesList.push_back(line);

	if(BOOLSETTING(HUB_BOLD_TABS))
		setDirty();
	
	if(BOOLSETTING(STATUS_IN_CHAT) && inChat) {
		addLine(_T("*** ") + aLine, BOOLSETTING(HUB_BOLD_TABS));
	}

	if(BOOLSETTING(POPUP_ON_HUBSTATUS)) {
		if( aLine.find(_T("Disconnected")) != tstring::npos ) {
			PopupManager::getInstance()->ShowDisconnected(server, m_hWnd);
		}
	}
	if(BOOLSETTING(LOG_STATUS_MESSAGES)) {
		StringMap params;
		params["hub"] = client->getName();
		params["hubaddr"] = client->getAddressPort();
		params["message"] = Text::fromT(aLine);
		LOG(LogManager::STATUS, params);
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
	closing = true;
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
	if(!user->isSet(User::HIDDEN)) 
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

void HubFrame::on(UserIp, Client*, const User::List& aList) throw() {
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
	speak(REMOVE_USER, user);
}

void HubFrame::on(Redirect, Client*, const string& line) throw() { 
	string s, f;
	u_int16_t p = 411;
	Util::decodeUrl(line, s, p, f);
	if(ClientManager::getInstance()->isConnected(s, p)) {
		speak(ADD_STATUS_LINE, STRING(REDIRECT_ALREADY_CONNECTED));
		return;
	}

	redirect = Text::toT(line);
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

LRESULT HubFrame::onCopyUserList(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int id = wID - IDC_COPY;
	ctrlUsers.copy(id);
	
	return 0;
}

void HubFrame::openLinksInTopic() {
	int length = GetWindowTextLength();
	TCHAR* buf = new TCHAR[length + 1];
	
	GetWindowText(buf, length);
	
	tstring topic = buf;
	delete[] buf;

	int pos = -1;
	TStringList urls;
	
	while( (pos = topic.find(_T("http://"), pos+1)) != string::npos ){
		int pos2 = topic.find(_T(" "), pos+1);
		urls.push_back(topic.substr(pos, pos2-pos));
	}
	pos = -1;
	while( (pos = topic.find(_T("www."), pos+1)) != string::npos ) {
		if(topic[pos-1] != _T('/')) {
			int pos2 = topic.find(_T(" "), pos+1);
			urls.push_back(topic.substr(pos, pos2-pos));
		}
	}
	for( TStringIter i = urls.begin(); i != urls.end(); ++i ) {
		WinUtil::openLink((*i));
	}
}

void HubFrame::removeUser(const User::Ptr& u) {
	int j = -1;
	tstring nick = (stripIsp ? Text::toT(u->getShortNick()) : Text::toT(u->getNick()));
	while( ( j = ctrlUsers.findItem(nick, j) ) != -1 ) {
		UserInfo* ui = ctrlUsers.getItemData(j);
		if(Util::stricmp(u->getNick(), ui->user->getNick()) == 0 ) {
			ctrlUsers.DeleteItem(j);
			break;
		}
	}

	for(UserIter i = usermap.begin(); i != usermap.end(); ++i) {
		if(Util::stricmp(i->second->user->getNick(), u->getNick()) == 0){
			delete i->second;
			i->second = NULL;
			usermap.erase(i);
			
			break;
		}
	}

	if(showJoins) {
		if (!favShowJoins || u->isFavoriteUser()) {
			addLine(_T("*** ") + TSTRING(PARTS) + nick, BOOLSETTING(HUB_BOLD_TABS));
		}
	}
}

LRESULT HubFrame::onFilterChar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	TCHAR *buf = new TCHAR[ctrlFilter.GetWindowTextLength()+1];
	ctrlFilter.GetWindowText(buf, ctrlFilter.GetWindowTextLength()+1);
	filter = buf;
	delete[] buf;
	
	updateUserList();

	bHandled = FALSE;

	return 0;
}

LRESULT HubFrame::onSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	TCHAR *buf = new TCHAR[ctrlFilter.GetWindowTextLength()+1];
	ctrlFilter.GetWindowText(buf, ctrlFilter.GetWindowTextLength()+1);
	filter = buf;
	delete[] buf;
	
	updateUserList();
	
	bHandled = FALSE;

	return 0;
}

bool HubFrame::parseFilter(int& mode, int64_t& size) {
	size_t start = tstring::npos;
	size_t end = tstring::npos;
	int64_t multiplier = 1;
	
	if(Util::strnicmp(filter.c_str(), _T(">="), 2) == 0) {
		mode = 1;
		start = 2;
	} else if(Util::strnicmp(filter.c_str(), _T("<="), 2) == 0) {
		mode = 2;
		start = 2;
	} else if(Util::strnicmp(filter.c_str(), _T("=="), 2) == 0) {
		mode = 0;
		start = 2;
	} else if(Util::strnicmp(filter.c_str(), _T("!="), 2) == 0) {
		mode = 5;
		start = 2;
	} else if(filter[0] == _T('<')) {
		mode = 4;
		start = 1;
	} else if(filter[0] == _T('>')) {
		mode = 3;
		start = 1;
	} else if(filter[0] == _T('=')) {
		mode = 1;
		start = 1;
	}

	if(start == tstring::npos)
		return false;
	if(filter.length() <= start)
		return false;

	if((end = Util::findSubString(filter, _T("TiB"))) != tstring::npos) {
		//hmms ugly but vs complains about integral constant overflow otherwise
		multiplier = int64_t(1024*1024*1024)*1024;
	} else if((end = Util::findSubString(filter, _T("GiB"))) != tstring::npos) {
		multiplier = 1024*1024*1024;
	} else if((end = Util::findSubString(filter, _T("MiB"))) != tstring::npos) {
		multiplier = 1024*1024;
	} else if((end = Util::findSubString(filter, _T("KiB"))) != tstring::npos) {
		multiplier = 1024;
	} else if((end = Util::findSubString(filter, _T("TB"))) != tstring::npos) {
		//hmms ugly but vs complains about integral constant overflow otherwise
		multiplier = int64_t(1000*1000*1000)*1000;
	} else if((end = Util::findSubString(filter, _T("GB"))) != tstring::npos) {
		multiplier = 1000*1000*1000;
	} else if((end = Util::findSubString(filter, _T("MB"))) != tstring::npos) {
		multiplier = 1000*1000;
	} else if((end = Util::findSubString(filter, _T("kB"))) != tstring::npos) {
		multiplier = 1000;
	} else if((end = Util::findSubString(filter, _T("B"))) != tstring::npos) {
		multiplier = 1;
	}


	if(end == tstring::npos) {
		end = filter.length();
	}
	
	tstring tmpSize = filter.substr(start, end-start);
	size = static_cast<int64_t>(Util::toDouble(Text::fromT(tmpSize)) * multiplier);
	
	return true;
}

void HubFrame::updateUserList() {
	Lock l(updateCS);

	ctrlUsers.SetRedraw(FALSE);
	ctrlUsers.DeleteAllItems();

	int64_t size = -1;

	//0 - ==
	//1 - >=
	//2 - <=
	//3 - >
	//4 - <
	//5 - !=
	int mode = -1;

	int sel = ctrlFilterSel.GetCurSel();

	bool doSizeCompare = parseFilter(mode, size) && sel == COLUMN_SHARED;

	if(filter.empty()) {
		for(UserIter i = usermap.begin(); i != usermap.end(); ++i){
			if(i->second != NULL)
				ctrlUsers.insertItem(i->second, getImage(i->second->user));	
		}
		ctrlUsers.SetRedraw(TRUE);
		return;
	}
	
	for(UserIter i = usermap.begin(); i != usermap.end(); ++i){
		if( i->second != NULL ) {
			bool insert = false;

			if(doSizeCompare) {
				switch(mode) {
					case 0: insert = (size == i->second->user->getBytesShared()); break;
					case 1: insert = (size <=  i->second->user->getBytesShared()); break;
					case 2: insert = (size >=  i->second->user->getBytesShared()); break;
					case 3: insert = (size < i->second->user->getBytesShared()); break;
					case 4: insert = (size > i->second->user->getBytesShared()); break;
					case 5: insert = (size != i->second->user->getBytesShared()); break;
				}
			} else {
				if(Util::findSubString(i->second->getText(sel), filter) != string::npos)
					insert = true;
			}

			if(insert) {
				ctrlUsers.insertItem(i->second, getImage(i->second->user));	
			}
		}
	}

	ctrlUsers.SetRedraw(TRUE);
}

LRESULT HubFrame::onShowHubLog(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	
	StringMap params;
	params["hub"] = client->getName();
	params["hubaddr"] = client->getAddressPort();
	params["mynick"] = client->getNick(); 

	tstring path = Text::toT(LogManager::getInstance()->getLogFilename(LogManager::CHAT, params));
	if(!path.empty()) {
		if(wID == IDC_OPEN_LOG_DIR) {
			path = Util::getFilePath(path);	
		}
		ShellExecute(NULL, _T("open"), Util::validateFileName(path).c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
	return 0;
}

HubFrame::UserInfo* HubFrame::findUser(tstring & nick){
	if( !nick.empty() ){
		UserInfo* ui = ctrlUsers.getItemData(ctrlUsers.GetNextItem(-1, LVNI_SELECTED));
		if(ui != NULL) {
			if( ui->getText(COLUMN_NICK).find(nick) != tstring::npos ) {
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

bool HubFrame::resolve(const wstring& aDns) {
	HANDLE res;
	bool ret = false;
	if(resolveBuffer == NULL) {
        
		resolveBuffer = new char[MAXGETHOSTSTRUCT];
		
		PME regexp("\\b(([01]?\\d?\\d|2[0-4]\\d|25[0-5])\\.){3}([01]?\\d?\\d|2[0-4]\\d|25[0-5])\\b");
		
		if(regexp.match(aDns)) {
			unsigned long l = inet_addr(Text::wideToAcp(aDns).c_str());

			res = WSAAsyncGetHostByAddr(m_hWnd, RESOLVE_IP, (char*)&l, 4, AF_INET, resolveBuffer, MAXGETHOSTSTRUCT);
			isIP = true;
		} else {
			res = WSAAsyncGetHostByName(m_hWnd, RESOLVE_IP, Text::wideToAcp(aDns).c_str(), resolveBuffer, MAXGETHOSTSTRUCT);
			isIP = false;
		}

		if( res == 0 ) {
			delete[] resolveBuffer;
			resolveBuffer = NULL;
		} else
			ret = true;
	}

	return ret;
}

LRESULT HubFrame::onResolvedIP(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/){
	if( resolveBuffer && WSAGETASYNCERROR(lParam) == 0 ) {
		hostent *h = (hostent*)resolveBuffer;
		in_addr a;
		
		memcpy(&a.S_un.S_addr, h->h_addr_list[0], 4);
		char * c = inet_ntoa(a);
		if( isIP )
			addClientLine(Text::acpToWide(c) + _T(" ") +  TSTRING(RESOLVES_TO) + _T(" ") + Text::acpToWide(h->h_name), BOOLSETTING(STATUS_IN_CHAT));
		else
			addClientLine(Text::acpToWide(h->h_name) + _T(" ") +  TSTRING(RESOLVES_TO) + _T(" ") + Text::acpToWide(c), BOOLSETTING(STATUS_IN_CHAT));
	
	//since the user can't do anything about these errors avoid showing them
	} else if(WSAGETASYNCERROR(lParam)) {
		if(WSAGETASYNCERROR(lParam) != WSAENOBUFS && WSAGETASYNCERROR(lParam) != WSAEFAULT) {
			addClientLine(Text::acpToWide( Util::translateError(WSAGETASYNCERROR(lParam))), BOOLSETTING(STATUS_IN_CHAT));
		}
	}
	
	if(resolveBuffer) {
		delete[] resolveBuffer;
		resolveBuffer = NULL;
	}
	return 0;
}


/**
 * @file
 * $Id: HubFrame.cpp,v 1.25 2004/02/21 15:15:28 trem Exp $
 */
