#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "../client/SettingsManager.h"
#include "../client/Util.h"

#include "WinUtil.h"
#include "PopupManager.h"


PopupManager* Singleton< PopupManager >::instance = NULL;

void PopupManager::Show(HWND &hWnd, const string& aMsg ) {
	Lock l(cs);

	if(!activated)
		return;

	if (!Util::getAway() && BOOLSETTING(POPUP_AWAY)) {
		return;
	}
	
	CRect rcDesktop;
	
	//get desktop rect so we know where to place the popup
	::SystemParametersInfo(SPI_GETWORKAREA,0,&rcDesktop,0);
	
	int screenHeight = rcDesktop.bottom;
	int screenWidth = rcDesktop.right;

	//if we have popups all the way up to the top of the screen do not create a new one
	if( (offset + height) > screenHeight)
		return;
	
	//get the handle of the window that has focus
	HWND gotFocus = ::SetFocus(hWnd);
	
	//compute the window position
	CRect rc(screenWidth - width , screenHeight - height - offset, screenWidth, screenHeight - offset);
	
	//Create a new popup
	PopupWnd *p = new PopupWnd(hWnd, aMsg, rc, hBitmap);
			
	//Set the correct font and display the window
	p->ShowWindow(SW_SHOW);
		
	//restore focus to window
	::SetFocus(gotFocus);

	
	
	//increase offset so we know where to place the next popup
	offset += height;

	popups.push_back(p);
}

void PopupManager::onAction(TimerManagerListener::Types type, u_int32_t tick) {
	if(TimerManagerListener::Types::SECOND == type) {
		//we got nothing to do here
		if(popups.empty()) {
			return;
		}

		if(!BOOLSETTING(REMOVE_POPUPS))
			return;

		//check all popups and see if we need to remove anyone
		PopupList::iterator i = popups.begin();
		for(; i != popups.end(); ++i) {

			if((*i)->visible + SETTING(POPUP_TIMEOUT) * 1000 < tick) {
				//okay remove the first popup
				Remove();

				//if list is empty there is nothing more to do
				if(popups.empty())
					return;
				
				//start over from the beginning
				i = popups.begin();
			}
		}



	}
}

void PopupManager::onAction(QueueManagerListener::Types type, string msg) {
	//we can't create the window in this thread, then the client will crash
	//so post a message and let the main window thread create it
	string *s = new string(msg);
	::PostMessage(hWnd, WM_SPEAKER, DOWNLOAD_COMPLETE, (LPARAM)s);
}

void PopupManager::Remove(int pos) {
	Lock l(cs);

	CRect rcDesktop;

	//get desktop rect so we know where to place the popup
	::SystemParametersInfo(SPI_GETWORKAREA,0,&rcDesktop,0);
	if(pos == 0)
		pos = rcDesktop.bottom;

	//find the correct window
	int end = (rcDesktop.bottom - pos) / height;
	PopupList::iterator i = popups.begin();
	for(int j = 0; j < end; ++j, ++i);
	
	//remove the window from the list
	PopupWnd *p = (*i);
	popups.erase(i);
	
	//close the window and delete it
	if(p == NULL){
		return;
	}
	
	p->SendMessage(WM_CLOSE, 0, 0);
	delete p;
	p = NULL;
	
	    
	//set offset one window position lower
	offset -= height;

	//nothing to do
	if(popups.empty())
		return;

	CRect rc;

	//move down all windows
	for(i = popups.begin(); i != popups.end(); ++i) {
		(*i)->GetWindowRect(rc);
		if(rc.bottom <= pos){
			rc.top += height;
			rc.bottom += height;
			(*i)->MoveWindow(rc);
		}
	}
}

void PopupManager::ShowPm(const string& nick, const string& msg, HWND hWnd /* = NULL */){
	dcdebug("%s\n", msg);
	int pos = msg.find_first_of(">")+1;
	if(pos == string::npos )
		pos = 0;

	string s = "New PM from: " + nick + " Msg:\r\n" + msg.substr(pos);
	if(hWnd == NULL)
		Show(this->hWnd, s);
	else
		Show(hWnd, s);
}

void PopupManager::ShowMC(const string& nick, const string& msg, HWND hWnd /* = NULL */){
	string s = nick + " says:\r\n" + msg;
	if(hWnd == NULL)
		Show(this->hWnd, s);
	else
		Show(hWnd, s);
}

void PopupManager::ShowMC(const string& msg, HWND hWnd /* = NULL */){
	int pos1 = msg.find_first_of("<");
	int pos2 = msg.find_first_of(">");
	
	//something wrong with the string, return
	if(pos1 == string::npos || pos2 == string::npos)
		return;

	ShowMC(msg.substr(pos1+1, pos2-pos1-1), msg.substr(pos2+1), hWnd);
	
}

void PopupManager::ShowDisconnected(const string& hub, HWND hWnd /* = NULL */) {
	string s = "you've been disconnected from:\r\n " + hub;
	if(hWnd == NULL)
		Show(this->hWnd, s);
	else
		Show(hWnd, s);
}

void PopupManager::ShowDownloadComplete(string *msg){
	string s = "Download complete:\r\n";
	
	//if we have a whole path remove everything but the last directory
	if(msg->at(msg->length()-1) == '\\'){
		int i = msg->rfind('\\', msg->length()-2);
		if( i != string::npos)
			s = s + msg->substr(i+1, msg->length() - i - 2);
	}else {
		s = s+ (*msg);
	}
	
	delete msg;
	Show(hWnd, s);
}