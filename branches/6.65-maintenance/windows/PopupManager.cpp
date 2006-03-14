/* 
* Copyright (C) 2003-2005 Pär Björklund, per.bjorklund@gmail.com
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
#include "../client/SettingsManager.h"
#include "../client/Util.h"

#include "WinUtil.h"
#include "PopupManager.h"
#include "MainFrm.h"


PopupManager* Singleton< PopupManager >::instance = NULL;

void PopupManager::Show(const tstring& aMsg, HWND owner) {
	if(!activated)
		return;

	if (!Util::getAway() && BOOLSETTING(POPUP_AWAY))
		return;

	if(!minimized && BOOLSETTING(POPUP_MINIMIZED)) 
		return;

	
	if( BOOLSETTING(POPUP_DONT_SHOW_ON_ACTIVE) && ( (HWND)::SendMessage(WinUtil::mdiClient, WM_MDIGETACTIVE, NULL, NULL) == owner ) )
		return;
	
	
	CRect rcDesktop;
	
	//get desktop rect so we know where to place the popup
	::SystemParametersInfo(SPI_GETWORKAREA,0,&rcDesktop,0);
	
	int screenHeight = rcDesktop.bottom;
	int screenWidth = rcDesktop.right;

	//if we have popups all the way up to the top of the screen do not create a new one
	if( (offset + height) > screenHeight)
		return;
	
	//compute the window position
	CRect rc(screenWidth - width , screenHeight - height - offset, screenWidth, screenHeight - offset);
	
	//Create a new popup
	PopupWnd *p = new PopupWnd(aMsg, rc, hBitmap, id++);
	p->owner = owner;
			
	//move the window to the top of the z-order and display it
	p->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_SHOWWINDOW);

	//increase offset so we know where to place the next popup
	offset = offset + height;

	popups.push_back(p);
}

void PopupManager::on(TimerManagerListener::Second /*type*/, u_int32_t /*tick*/ ) {
	if(!BOOLSETTING(REMOVE_POPUPS))
		return;

	//post a message and let the main window thread take care of the window
	::PostMessage(WinUtil::mainWnd, WM_SPEAKER, MainFrame::REMOVE_POPUP, 0);

}

void PopupManager::on(QueueManagerListener::ReleaseDone, string msg) {
	//we can't create the window in this thread, then the client will crash
	//so post a message and let the main window thread create it
	::PostMessage(WinUtil::mainWnd, WM_SPEAKER, MainFrame::DOWNLOAD_COMPLETE, (LPARAM)new tstring(Text::toT(msg)));
}

void PopupManager::AutoRemove(){
	//we got nothing to do here
	if(popups.empty()) {
		return;
	}

	//check all popups and see if we need to remove anyone
	PopupIter i = popups.begin();
	for(; i != popups.end(); ++i) {

		if((*i)->visible + SETTING(POPUP_TIMEOUT) * 1000 < GET_TICK()) {
			//okay remove the first popup
			Remove((*i)->id);

			//if list is empty there is nothing more to do
			if(popups.empty())
				return;

			//start over from the beginning
			i = popups.begin();
		}
	}
}

void PopupManager::Remove(int pos, bool	clicked /* = false*/) {
	bool rButton = pos & 0x10000 > 0;
	pos = pos & 0xFFFF; // remove the right mouse button bit.
	
	if(popups.empty()) { //seems like we hit a bit of a race condition here
		return;			 //with the autoremove function, ohh well return and no harm done.
	}
	
	//find the correct window
	PopupIter i = popups.begin();

	for(; i != popups.end(); ++i) {
		if((*i)->id == pos)
			break;
	}

	dcassert(i != popups.end());

	//remove the window from the list
	PopupWnd *p = (*i);
	i = popups.erase(i);

	if(p == NULL){
		return;
	}

	//close the window and delete it
	HWND w = p->owner;
	p->SendMessage(WM_CLOSE, 0, 0);
	delete p;

	if( clicked && BOOLSETTING(POPUP_ACTIVATE_ON_CLICK) && !rButton ) {
		SetForegroundWindow(WinUtil::mainWnd);
		if( IsIconic(WinUtil::mainWnd) )
			ShowWindow(WinUtil::mainWnd, SW_RESTORE);
		
		::SendMessage(WinUtil::mdiClient, WM_MDIACTIVATE, (WPARAM)w, NULL);
	}

	//set offset one window position lower
	dcassert(offset > 0);
	offset = offset - height;

	//nothing to do
	if(popups.empty())
		return;

	CRect rc;

	//move down all windows
	for(; i != popups.end(); ++i) {
		(*i)->GetWindowRect(rc);
		rc.top += height;
		rc.bottom += height;
		(*i)->MoveWindow(rc);
	}
}

void PopupManager::ShowPm(const tstring& nick, const tstring& msg, HWND owner){
	int pos = msg.find_first_of(_T(">"))+1;
	if(pos == tstring::npos )
		pos = 0;

	tstring s = TSTRING(POPUP_NEW_PM) + _T(" ") + nick + _T(" ") + TSTRING(POPUP_MSG) + msg.substr(pos);
	Show(s, owner);
}

void PopupManager::ShowMC(const tstring& nick, const tstring& msg, HWND owner){
	tstring s = nick + _T(" ") + TSTRING(POPUP_SAYS) + msg;
	Show(s, owner);
}

void PopupManager::ShowMC(const tstring& msg, HWND owner){
	int pos1 = msg.find_first_of(_T("<"));
	int pos2 = msg.find_first_of(_T(">"));
	
	//something wrong with the string, return
	if(pos1 == tstring::npos || pos2 == tstring::npos)
		return;

	ShowMC(msg.substr(pos1+1, pos2-pos1-1), msg.substr(pos2+1), owner);
	
}

void PopupManager::ShowDisconnected(const tstring& hub, HWND owner) {
	tstring s = TSTRING(POPUP_DISCONNECTED) + hub;
	Show(s, owner);
}

void PopupManager::ShowDownloadComplete(tstring *msg){
	tstring s = TSTRING(POPUP_DOWNLOAD_COMPLETE);
	
	//if we have a whole path remove everything but the last directory
	if(msg->at(msg->length()-1) == _T('\\')){
		int i = msg->rfind(_T('\\'), msg->length()-2);
		if( i != tstring::npos)
			s = s + msg->substr(i+1, msg->length() - i - 2);
	}else {
		s = s+ (*msg);
	}
	
	delete msg;
	Show(s);
}