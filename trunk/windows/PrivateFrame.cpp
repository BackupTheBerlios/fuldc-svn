/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
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
#include "../client/File.h"
#include "../client/StringTokenizer.h"

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
	tabMenu.AppendMenu(MF_STRING, IDC_GETLIST, CTSTRING(GET_FILE_LIST));
	tabMenu.AppendMenu(MF_STRING, IDC_MATCH_QUEUE, CTSTRING(MATCH_QUEUE));
	tabMenu.AppendMenu(MF_STRING, IDC_GRANTSLOT, CTSTRING(GRANT_EXTRA_SLOT));
	tabMenu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CTSTRING(ADD_TO_FAVORITES));
	tabMenu.AppendMenu(MF_STRING, IDC_COPY_NICK, CTSTRING(COPY_NICK));
	tabMenu.AppendMenu(MF_STRING, IDC_SHOWLOG, CTSTRING(SHOW_LOG));


	PostMessage(WM_SPEAKER, USER_UPDATED);
	created = true;

	ClientManager::getInstance()->addListener(this);

	WinUtil::SetIcon(m_hWnd, _T("User.ico"));
	
	if(BOOLSETTING(STRIP_ISP_PM))
		ctrlClient.setFlag(CFulEditCtrl::STRIP_ISP);

	bHandled = FALSE;
	return 1;
}

void PrivateFrame::gotMessage(const User::Ptr& aUser, const tstring& aMessage) {
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
						PlaySound(_T("PM.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_NOWAIT);
					else
						MessageBeep(MB_OK);
				}
				if(BOOLSETTING(POPUP_ON_PM) && !BOOLSETTING(POPUP_ON_NEW_PM) && p->doPopups) {
					PopupManager::getInstance()->ShowPm(Text::toT(aUser->getNick()), aMessage, p->m_hWnd);
				}

				if(BOOLSETTING(FLASH_WINDOW_ON_PM) && !BOOLSETTING(FLASH_WINDOW_ON_NEW_PM)) {
					p->FlashWindow();
				}
				break;
			}
		}
		if(!found) {
			p = new PrivateFrame(aUser);
			frames[aUser] = p;
			p->ReadLog();
			p->addLine(aMessage);
			if(Util::getAway()) {
				// if no_awaymsg_to_bots is set, and aUser has an empty connection type (i.e. probably is a bot), then don't send
				if(!(BOOLSETTING(NO_AWAYMSG_TO_BOTS) && aUser->getConnection().empty()))
					p->sendMessage(Text::toT(Util::getAwayMessage()));
			}

			if(BOOLSETTING(PRIVATE_MESSAGE_BEEP) || BOOLSETTING(PRIVATE_MESSAGE_BEEP_OPEN)) {
				if(BOOLSETTING(CUSTOM_SOUND))
					PlaySound(_T("newPM.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_NOWAIT);
				else
					MessageBeep(MB_OK);
			}

			if(BOOLSETTING(POPUP_ON_PM) && p->doPopups) {
				PopupManager::getInstance()->ShowPm(Text::toT(aUser->getNick()), aMessage, p->m_hWnd);
			}

			if(BOOLSETTING(FLASH_WINDOW_ON_PM)){
				p->FlashWindow();
			}
		}
	} else {
		if(BOOLSETTING(PRIVATE_MESSAGE_BEEP) && !i->second->muted) {
			if(BOOLSETTING(CUSTOM_SOUND))
				PlaySound(_T("PM.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_NOWAIT);
			else
				MessageBeep(MB_OK);
		}
		if (BOOLSETTING(POPUP_ON_PM) && !BOOLSETTING(POPUP_ON_NEW_PM) && i->second->doPopups) {
			PopupManager::getInstance()->ShowPm(Text::toT(aUser->getNick()), aMessage, i->second->m_hWnd);
		}

		if(BOOLSETTING(FLASH_WINDOW_ON_PM) && !BOOLSETTING(FLASH_WINDOW_ON_NEW_PM)) {
			p->FlashWindow();
		}

		i->second->addLine(aMessage);

	}
}

void PrivateFrame::openWindow(const User::Ptr& aUser, const tstring& msg) {
	PrivateFrame* p = NULL;
	Lock l(cs);
	FrameIter i = frames.find(aUser);
	if(i == frames.end()) {
		p = new PrivateFrame(aUser);
		frames[aUser] = p;
		p->CreateEx(WinUtil::mdiClient);
	} else {
		p = i->second;
		if(::IsIconic(p->m_hWnd))
			::ShowWindow(p->m_hWnd, SW_RESTORE);
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
						auto_ptr<TCHAR> messageContents(new TCHAR[ctrlMessage.GetWindowTextLength()+2]);
						ctrlMessage.GetWindowText(messageContents.get(), ctrlMessage.GetWindowTextLength()+1);
						currentCommand = tstring(messageContents.get());
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

				auto_ptr<TCHAR> messageContents(new TCHAR[ctrlMessage.GetWindowTextLength()+2]);
				ctrlMessage.GetWindowText(messageContents.get(), ctrlMessage.GetWindowTextLength()+1);
				currentCommand = tstring(messageContents.get());

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
	bool resetText = true;

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

		// Process special commands
		if(s[0] == _T('/')) {
			tstring param;
			tstring message;
			tstring status;
			if(WinUtil::checkCommand(s, param, message, status)) {
				if(!message.empty()) {
					sendMessage(message);
				}
				if(!status.empty()) {
					addClientLine(status);
				}
			} else if(Util::stricmp(s.c_str(), _T("clear")) == 0) {
				ctrlClient.SetWindowText(_T(""));
			} else if(Util::stricmp(s.c_str(), _T("grant")) == 0) {
				UploadManager::getInstance()->reserveSlot(getUser());
				addClientLine(TSTRING(SLOT_GRANTED));
			} else if(Util::stricmp(s.c_str(), _T("close")) == 0) {
				PostMessage(WM_CLOSE);
			} else if((Util::stricmp(s.c_str(), _T("favorite")) == 0) || (Util::stricmp(s.c_str(), _T("fav")) == 0)) {
				HubManager::getInstance()->addFavoriteUser(getUser());
				addLine(TSTRING(FAVORITE_USER_ADDED));
			} else if(Util::stricmp(s.c_str(), _T("getlist")) == 0) {
				BOOL bTmp;
				onGetList(0,0,0,bTmp);
			} else if(Util::stricmp(s.c_str(), _T("showlog")) == 0) {
				StringMap params;
				params["user"] = user->getNick();
				params["hub"] = user->getClientName();
				params["mynick"] = user->getClientNick(); 
				params["mycid"] = user->getClientCID().toBase32(); 
				params["cid"] = user->getCID().toBase32(); 
				params["hubaddr"] = user->getClientAddressPort();
				WinUtil::openFile(Text::toT(Util::validateFileName(SETTING(LOG_DIRECTORY) + Util::formatParams(SETTING(LOG_FILE_PRIVATE_CHAT), params))));
			} else if(Util::stricmp(s.c_str(), _T("help")) == 0) {
				addLine(_T("*** ") + WinUtil::commands + _T(", /getlist, /clear, /grant, /close, /favorite, /mute, /unmute, /pop on|off"));
			} else if(Util::stricmp(s.c_str(), _T("me")) == 0) {
				sendMessage(_T("/") + s + _T(" ") + param);
			} else if(Util::stricmp(s.c_str(), _T("mute")) == 0) {
				int res = WinUtil::checkParam(param);
				if(param.empty() || 1 == res ){
					muted = true;
					addClientLine(TSTRING(MUTED));
				} else if( 0 == res ){
					muted = false;
					addClientLine(TSTRING(UNMUTED));
				}
			} else if(Util::stricmp(s.c_str(), _T("unmute")) == 0){
				muted = false;
				addClientLine(TSTRING(UNMUTED));
			} else if(Util::stricmp(s.c_str(), _T("pop")) == 0) {
				int res = WinUtil::checkParam(param);
				if( 1 == res || (param.empty() && !doPopups) ){
					doPopups = true;
					addClientLine(TSTRING(POPUPS_ACTIVATED));
				}else if( 0 == res || (param.empty() && doPopups) ){
					doPopups = false;
					addClientLine(TSTRING(POPUPS_DEACTIVATED));
				}
			} else {
				if(user->isOnline()) {
					sendMessage(s);
				} else {
					ctrlStatus.SetText(0, CTSTRING(USER_WENT_OFFLINE));
					resetText = false;
				}
			}
		} else {
			if(user->isOnline()) {
				sendMessage(s);
			} else {
				ctrlStatus.SetText(0, CTSTRING(USER_WENT_OFFLINE));
				resetText = false;
			}
		}
		if(resetText)
			ctrlMessage.SetWindowText(_T(""));
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

void PrivateFrame::addLine(const tstring& aLine, bool bold) {
	if(!created) {
		if(BOOLSETTING(POPUNDER_PM))
			WinUtil::hiddenCreateEx(this);
		else
			CreateEx(WinUtil::mdiClient);
	}

	if(BOOLSETTING(LOG_PRIVATE_CHAT)) {
		StringMap params;
		params["message"] = Text::fromT(aLine);
		params["user"] = user->getNick();
		params["hub"] = user->getClientName();
		params["hubaddr"] = user->getClientAddressPort();
		params["mynick"] = user->getClientNick(); 
		params["mycid"] = user->getClientCID().toBase32(); 
		params["cid"] = user->getCID().toBase32(); 
		LOG(LogManager::PM, params);
	}

	if(ctrlClient.AddLine(aLine, BOOLSETTING(TIME_STAMPS)))
		setNotify();
	
	addClientLine(TSTRING(LAST_CHANGE) + Util::getTimeStringW());

	if(bold)
		setDirty();
}

LRESULT PrivateFrame::onTabContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
	prepareMenu(tabMenu, UserCommand::CONTEXT_CHAT, Text::toT(user->getClientAddressPort()), user->isClientOp());
	tabMenu.AppendMenu(MF_SEPARATOR);
	tabMenu.AppendMenu(MF_STRING, IDC_CLOSE_WINDOW, CTSTRING(CLOSE));
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
	ucParams["mycid"] = user->getClientCID().toBase32();

	user->getParams(ucParams);
	user->clientEscapeParams(ucParams);

	user->sendUserCmd(Util::formatParams(uc.getCommand(), ucParams));
	return;
};



LRESULT PrivateFrame::onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	try {
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_CLIENT_VIEW);
	} catch(const Exception& e) {
		addClientLine(Text::toT(e.getError()));
	}
	return 0;
}

LRESULT PrivateFrame::onMatchQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	try {
		QueueManager::getInstance()->addList(user, QueueItem::FLAG_MATCH_QUEUE);
	} catch(const Exception& e) {
		addClientLine(Text::toT(e.getError()));
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
	
	if( pt.x == -1 && pt.y == -1 )
		pt.x = pt.y = 0;

	ctrlClient.GetClientRect(&rc);
	if (uMsg == WM_CONTEXTMENU)
		ctrlClient.ScreenToClient(&pt);

	if(PtInRect(&rc, pt)) {
		//hämta raden som vi klickade på
		tstring tmp;
		tstring::size_type start = ctrlClient.TextUnderCursor(pt, tmp);

		// kontrollera ifall musen var över nicket
		tstring::size_type end = tmp.find_first_of(_T(" >\t"), start+1);
		if (end != tstring::npos && end != start+1 && tmp[start-1] == _T('<')) {
			
			// konvertera tillbaka positionen för musen så menyn hamnar rätt
			ctrlClient.ClientToScreen(&pt);
			tabMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
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
				ctrlClient.ShowMenu(m_hWnd, pt);
				bHandled = TRUE;
				return TRUE;
			}
		}
	}else {
		bHandled = FALSE;
	}
	return 0;
}

LRESULT PrivateFrame::onCopyNick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	WinUtil::setClipboard(Text::toT(user->getNick()));

	return 0;
}

LRESULT PrivateFrame::onViewLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	StringMap params;
	params["user"] = user->getNick();
	params["hub"] = user->getClientName();
	params["hubaddr"] = user->getClientAddressPort();
	params["mynick"] = user->getClientNick(); 
	params["mycid"] = user->getClientCID().toBase32(); 
	params["cid"] = user->getCID().toBase32(); 
	
	tstring path = Text::toT(LogManager::getInstance()->getLogFilename(LogManager::PM, params));
	if(!path.empty())
		ShellExecute(NULL, _T("open"), Util::validateFileName(path).c_str(), NULL, NULL, SW_SHOWNORMAL);
	return 0;
}

void PrivateFrame::closeAll(){
	FrameMap::iterator i = frames.begin();
	for(; i != frames.end(); ++i)
		i->second->PostMessage(WM_CLOSE, 0, 0);

	frames.clear();
}

void PrivateFrame::FlashWindow() {
	if( GetForegroundWindow() != WinUtil::mainWnd ) {
		DWORD flashCount;
		SystemParametersInfo(SPI_GETFOREGROUNDFLASHCOUNT, 0, &flashCount, 0);
		FLASHWINFO flash;
		flash.cbSize = sizeof(FLASHWINFO);
		flash.dwFlags = FLASHW_ALL;
		flash.uCount = flashCount;
		flash.hwnd = WinUtil::mainWnd;
		flash.dwTimeout = 0;

		FlashWindowEx(&flash);
	}
}

void PrivateFrame::ReadLog() {
	string path = SETTING(LOG_DIRECTORY) + Util::validateFileName(user->getNick()) + ".log";

	try {
        File f(path, File::READ, File::OPEN);
		
		int64_t size = f.getSize();

		if(size > 32*1024) {
			f.setPos(size - 32*1024);
		}

		StringList lines = StringTokenizer<string>(f.read(32*1024), "\r\n").getTokens();

		int linesCount = lines.size();

		int i = linesCount > 10 ? linesCount - 10 : 0;

		for(; i < linesCount; ++i){
			addLine(Text::acpToWide(lines[i]));
		}
        
		if(linesCount > 0)
			addLine(TSTRING(END_LOG));

		f.close();
	} catch(FileException & /*e*/){
	}
}

/**
 * @file
 * $Id: PrivateFrame.cpp,v 1.14 2004/02/21 15:15:02 trem Exp $
 */


