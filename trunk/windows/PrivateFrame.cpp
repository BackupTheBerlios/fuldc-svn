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

#include "PrivateFrame.h"
#include "SearchFrm.h"
#include "PopupManager.h"

#include "../client/Client.h"
#include "../client/ClientManager.h"
#include "../client/Util.h"
#include "../client/LogManager.h"
#include "../client/UploadManager.h"
#include "../client/ShareManager.h"
#include "../client/HubManager.h"
#include "../client/QueueManager.h"

#include <MMSystem.h>

CriticalSection PrivateFrame::cs;
PrivateFrame::FrameMap PrivateFrame::frames;

LRESULT PrivateFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	
	ctrlClient.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_READONLY, WS_EX_CLIENTEDGE);
	
	ctrlClient.LimitText(0);
	ctrlClient.SetFont(WinUtil::font);
	ctrlClient.SetBackgroundColor(WinUtil::bgColor);
	ctrlClient.SetTextColor(WinUtil::textColor);
	ctrlMessage.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		ES_AUTOHSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, WS_EX_CLIENTEDGE);
	
	// koppla containern till ctrlClient så vi kan snappa upp WM_CONTEXTMENU
	ctrlClientContainer.SubclassWindow(ctrlClient.m_hWnd);
	ctrlMessageContainer.SubclassWindow(ctrlMessage.m_hWnd);
	
	ctrlMessage.SetFont(WinUtil::font);

	tabMenu.CreatePopupMenu();
	tabMenu.AppendMenu(MF_STRING, IDC_GETLIST, CSTRING(GET_FILE_LIST));
	tabMenu.AppendMenu(MF_STRING, IDC_MATCH_QUEUE, CSTRING(MATCH_QUEUE));
	tabMenu.AppendMenu(MF_STRING, IDC_GRANTSLOT, CSTRING(GRANT_EXTRA_SLOT));
	tabMenu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CSTRING(ADD_TO_FAVORITES));
	tabMenu.AppendMenu(MF_STRING, IDC_COPY_NICK, CSTRING(COPY_NICK));
	tabMenu.AppendMenu(MF_STRING, IDC_SHOWLOG, CSTRING(SHOW_LOG));


	userMenu.CreatePopupMenu();
	userMenu.AppendMenu(MF_STRING, IDC_GETLIST, CSTRING(GET_FILE_LIST));
	userMenu.AppendMenu(MF_STRING, IDC_MATCH_QUEUE, CSTRING(MATCH_QUEUE));
	userMenu.AppendMenu(MF_STRING, IDC_GRANTSLOT, CSTRING(GRANT_EXTRA_SLOT));
	userMenu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CSTRING(ADD_TO_FAVORITES));
	userMenu.AppendMenu(MF_STRING, IDC_COPY_NICK, CSTRING(COPY_NICK));
	userMenu.AppendMenu(MF_STRING, IDC_SHOWLOG, CSTRING(SHOW_LOG));

	searchMenu.CreatePopupMenu();
	
	mcMenu.CreatePopupMenu();
	mcMenu.AppendMenu(MF_STRING, IDC_COPY, CSTRING(COPY));
	mcMenu.AppendMenu(MF_STRING, IDC_SEARCH, CSTRING(SEARCH));
	mcMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)searchMenu, CSTRING(SEARCH_SITES));

	//Set the MNS_NOTIFYBYPOS flag to receive WM_MENUCOMMAND
	MENUINFO inf;
	inf.cbSize = sizeof(MENUINFO);
	inf.fMask = MIM_STYLE | MIM_APPLYTOSUBMENUS;
	inf.dwStyle = MNS_NOTIFYBYPOS;
	mcMenu.SetMenuInfo(&inf);
	
	PostMessage(WM_SPEAKER, USER_UPDATED);
	created = true;

	ClientManager::getInstance()->addListener(this);

	WinUtil::SetIcon(m_hWnd, "User.ico");
	
	if(BOOLSETTING(STRIP_ISP_PM))
		ctrlClient.setFlag(CFulEditCtrl::STRIP_ISP);

	bHandled = FALSE;
	return 1;
}

void PrivateFrame::gotMessage(const User::Ptr& aUser, const string& aMessage) {
	PrivateFrame* p = NULL;
	Lock l(cs);
	FrameIter i = frames.find(aUser);
	if(i == frames.end()) {
		bool found = false;
		for(i = frames.begin(); i != frames.end(); ++i) {
			if( (!i->first->isOnline()) && 
				(i->first->getNick() == aUser->getNick()) &&
				(i->first->getLastHubAddress() == aUser->getLastHubAddress()) ) {
				
				found = true;
				p = i->second;
				frames.erase(i);
				frames[aUser] = p;
				p->setUser(aUser);
				p->addLine(aMessage);
				if(BOOLSETTING(PRIVATE_MESSAGE_BEEP) && !p->muted) {
					if(BOOLSETTING(CUSTOM_SOUND))
						PlaySound("PM.wav", NULL, SND_ASYNC | SND_FILENAME | SND_NOWAIT);
					else
						MessageBeep(MB_OK);
				}
				if (BOOLSETTING(POPUP_ON_PM) && !BOOLSETTING(POPUP_ON_NEW_PM) && p->doPopups) {
					PopupManager::getInstance()->ShowPm(aUser->getNick(), aMessage);
				}
				break;
			}
		}
		if(!found) {
			p = new PrivateFrame(aUser);
			frames[aUser] = p;
			p->addLine(aMessage);
			if(Util::getAway()) {
				// if no_awaymsg_to_bots is set, and aUser has an empty connection type (i.e. probably is a bot), then don't send
				if(!(BOOLSETTING(NO_AWAYMSG_TO_BOTS) && aUser->getConnection().empty()))
					p->sendMessage(Util::getAwayMessage());
			}

			if(BOOLSETTING(PRIVATE_MESSAGE_BEEP) || BOOLSETTING(PRIVATE_MESSAGE_BEEP_OPEN)) {
				if(BOOLSETTING(CUSTOM_SOUND))
					PlaySound("newPM.wav", NULL, SND_ASYNC | SND_FILENAME | SND_NOWAIT);
				else
					MessageBeep(MB_OK);
			}

			if (BOOLSETTING(POPUP_ON_PM) && p->doPopups) {
				PopupManager::getInstance()->ShowPm(aUser->getNick(), aMessage);
			}
		}
	} else {
		if(BOOLSETTING(PRIVATE_MESSAGE_BEEP) && !i->second->muted) {
			if(BOOLSETTING(CUSTOM_SOUND))
				PlaySound("PM.wav", NULL, SND_ASYNC | SND_FILENAME | SND_NOWAIT);
			else
				MessageBeep(MB_OK);
		}
		if (BOOLSETTING(POPUP_ON_PM) && !BOOLSETTING(POPUP_ON_NEW_PM) && i->second->doPopups) {
			PopupManager::getInstance()->ShowPm(aUser->getNick(), aMessage);
		}
		i->second->addLine(aMessage);

	}
}

void PrivateFrame::openWindow(const User::Ptr& aUser, const string& msg) {
	PrivateFrame* p = NULL;
	Lock l(cs);
	FrameIter i = frames.find(aUser);
	if(i == frames.end()) {
		p = new PrivateFrame(aUser);
		frames[aUser] = p;
		p->CreateEx(WinUtil::mdiClient);
	} else {
		p = i->second;
		p->MDIActivate(p->m_hWnd);
	}
	if(!msg.empty())
		p->sendMessage(msg);
}

LRESULT PrivateFrame::onChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
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
				if (curCommandPosition > 0 && !prevCommands.empty()) {
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

void PrivateFrame::onEnter()
{
	char* message;
	bool resetText = true;

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

		// Process special commands
		if(s[0] == '/') {
			string param;
			string message;
			string status;
			if(WinUtil::checkCommand(s, param, message, status)) {
				if(!message.empty()) {
					sendMessage(message);
				}
				if(!status.empty()) {
					addClientLine(status);
				}
			} else if(Util::stricmp(s.c_str(), "clear") == 0) {
				ctrlClient.SetWindowText("");
			} else if(Util::stricmp(s.c_str(), "grant") == 0) {
				UploadManager::getInstance()->reserveSlot(getUser());
				addClientLine(STRING(SLOT_GRANTED));
			} else if(Util::stricmp(s.c_str(), "close") == 0) {
				PostMessage(WM_CLOSE);
			} else if((Util::stricmp(s.c_str(), "favorite") == 0) || (Util::stricmp(s.c_str(), "fav") == 0)) {
				HubManager::getInstance()->addFavoriteUser(getUser());
				addLine(STRING(FAVORITE_USER_ADDED));
			} else if(Util::stricmp(s.c_str(), "getlist") == 0) {
				BOOL bTmp;
				onGetList(0,0,0,bTmp);
			} else if(Util::stricmp(s.c_str(), "help") == 0) {
				addLine("*** " + WinUtil::commands + ", /getlist, /clear, /grant, /close, /favorite, /mute, /unmute, /pop on|off");
			} else if(Util::stricmp(s.c_str(), "me") == 0) {
				sendMessage("/" + s + " " + param);
			} else if(Util::stricmp(s.c_str(), "mute") == 0) {
				muted = true;
				addClientLine(STRING(MUTED));
			} else if(Util::stricmp(s.c_str(), "unmute") == 0){
				muted = false;
				addClientLine(STRING(UNMUTED));
			} else if(Util::stricmp(s.c_str(), "pop") == 0) {
				if( Util::stricmp(param, "on") == 0){
					doPopups = true;
					addClientLine(STRING(POPUPS_ACTIVATED));
				}else if( Util::stricmp(param, "off") == 0){
					doPopups = false;
					addClientLine(STRING(POPUPS_DEACTIVATED));
				}
			} else {
				if(user->isOnline()) {
					sendMessage(s);
				} else {
					ctrlStatus.SetText(0, CSTRING(USER_WENT_OFFLINE));
					resetText = false;
				}
			}
		} else {
			if(user->isOnline()) {
				sendMessage(s);
			} else {
				ctrlStatus.SetText(0, CSTRING(USER_WENT_OFFLINE));
				resetText = false;
			}
		}
		if(resetText)
			ctrlMessage.SetWindowText("");
	} 
}

LRESULT PrivateFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	if(!closed) {
		ClientManager::getInstance()->removeListener(this);

		closed = true;
		PostMessage(WM_CLOSE);
		return 0;
	} else {
		Lock l(cs);
		frames.erase(user);

		bHandled = FALSE;
		return 0;
	}
}

void PrivateFrame::addLine(const string& aLine, bool bold) {
	if(!created) {
		if(BOOLSETTING(POPUNDER_PM))
			WinUtil::hiddenCreateEx(this);
		else
			CreateEx(WinUtil::mdiClient);
	}

	if(BOOLSETTING(LOG_PRIVATE_CHAT)) {
		StringMap params;
		params["message"] = aLine;
		LOG(user->getNick(), Util::formatParams(SETTING(LOG_FORMAT_PRIVATE_CHAT), params));
	}

	if(ctrlClient.AddLine(aLine, BOOLSETTING(TIME_STAMPS)))
		setNotify();
	
	addClientLine(CSTRING(LAST_CHANGE) + Util::getTimeString());

	if(bold)
		setDirty();
}

LRESULT PrivateFrame::onTabContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
	prepareMenu(tabMenu, UserCommand::CONTEXT_CHAT, user->getClientAddressPort(), user->isClientOp());
	tabMenu.AppendMenu(MF_SEPARATOR);
	tabMenu.AppendMenu(MF_STRING, IDC_CLOSE_WINDOW, CSTRING(CLOSE));
	tabMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
	tabMenu.DeleteMenu(tabMenu.GetMenuItemCount()-1, MF_BYPOSITION);
	tabMenu.DeleteMenu(tabMenu.GetMenuItemCount()-1, MF_BYPOSITION);
	cleanMenu(tabMenu);
	return TRUE;
}

void PrivateFrame::runUserCommand(UserCommand& uc) {
	if(!WinUtil::getUCParams(m_hWnd, uc, ucParams))
		return;

	ucParams["mynick"] = user->getClientNick();

	user->getParams(ucParams);

	user->send(Util::formatParams(uc.getCommand(), ucParams));
	return;
};



LRESULT PrivateFrame::onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	try {
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	} catch(const Exception& e) {
		addClientLine(e.getError());
	}
	return 0;
}

LRESULT PrivateFrame::onMatchQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	try {
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_MATCH_QUEUE);
	} catch(const Exception& e) {
		addClientLine(e.getError());
	}
	return 0;
}

LRESULT PrivateFrame::onGrantSlot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	UploadManager::getInstance()->reserveSlot(user);
	return 0;
}

LRESULT PrivateFrame::onAddToFavorites(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HubManager::getInstance()->addFavoriteUser(user);
	return 0;
}

void PrivateFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */) {
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);
	
	if(ctrlStatus.IsWindow()) {
		CRect sr;
		int w[1];
		ctrlStatus.GetClientRect(sr);
		
		w[0] = sr.right - 16;

		ctrlStatus.SetParts(1, w);
	}
	
	int h = WinUtil::fontHeight + 4;

	CRect rc = rect;
	rc.bottom -= h + 10;
	ctrlClient.MoveWindow(rc);
	
	rc = rect;
	rc.bottom -= 2;
	rc.top = rc.bottom - h - 5;
	rc.left +=2;
	rc.right -=2;
	ctrlMessage.MoveWindow(rc);
	
}

LRESULT PrivateFrame::onContextMenu(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
	RECT rc;
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

	ctrlClient.GetClientRect(&rc);
	if (uMsg == WM_CONTEXTMENU)
		ctrlClient.ScreenToClient(&pt);

	WinUtil::AppendSearchMenu(searchMenu);

	if(PtInRect(&rc, pt)) {
		//hämta raden som vi klickade på
		string tmp;
		string::size_type start = ctrlClient.TextUnderCursor(pt, tmp);

		// kontrollera ifall musen var över nicket
		string::size_type end = tmp.find_first_of(" >\t", start+1);
		if (end != string::npos && end != start+1 && tmp[start-1] == '<') {
			
			// konvertera tillbaka positionen för musen så menyn hamnar rätt
			ctrlClient.ClientToScreen(&pt);
			userMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		}else {
			RECT rc2;
			POINT pt2 = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) }; // location of mouse click 
			ctrlClient.ClientToScreen(&pt);
			ctrlClient.GetWindowRect(&rc);
			ctrlMessage.GetWindowRect(&rc2);
			ctrlMessage.ClientToScreen(&pt2);

			// Not in client area. In message are. Not both at the same time
			if (!PtInRect(&rc, pt2) && PtInRect(&rc2, pt2) && pt2.y > rc.bottom) {
				// In message box
				bHandled = FALSE;
				return FALSE;
			} else {
				ctrlClient.ScreenToClient(&pt);
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
		}
	}else {
		bHandled = false;
	}
	return 0;
}

LRESULT PrivateFrame::onCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(searchTerm.empty()){
		ctrlClient.Copy();
	} else {
		WinUtil::copyToClipboard(searchTerm);
		searchTerm = Util::emptyString;
	}

	return 0;
}

LRESULT PrivateFrame::onSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	WinUtil::search(searchTerm, 0);
	searchTerm = Util::emptyString;
	return 0;
}

LRESULT PrivateFrame::onMenuCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if(searchMenu.m_hMenu == (HMENU)lParam) {
		WinUtil::search(searchTerm, wParam+1);
		searchTerm = Util::emptyString;
	} else {
		MENUITEMINFO inf;
		inf.cbSize = sizeof(MENUITEMINFO);
		inf.fMask = MIIM_ID;
		mcMenu.GetMenuItemInfo(wParam, TRUE, &inf);
		PostMessage(WM_COMMAND, inf.wID, 0);
	}
	return 0;
}

LRESULT PrivateFrame::onCopyNick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	WinUtil::copyToClipboard(user->getNick());

	return 0;
}

LRESULT PrivateFrame::onViewLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	string path = SETTING(LOG_DIRECTORY) + user->getNick() + ".log";
	ShellExecute(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
	return 0;
}
LRESULT PrivateFrame::onLButton(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
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
		} 
	}
	return 0;
}

void PrivateFrame::closeAll(){
	FrameMap::iterator i = frames.begin();
	for(; i != frames.end(); ++i)
		i->second->PostMessage(WM_CLOSE, 0, 0);

	frames.clear();
}

/**
 * @file
 * $Id: PrivateFrame.cpp,v 1.14 2004/02/21 15:15:02 trem Exp $
 */


