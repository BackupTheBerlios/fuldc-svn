#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "../client/SettingsManager.h"
#include "../client/Util.h"

#include "WinUtil.h"
#include "PopupManager.h"
#include "MainFrm.h"


PopupManager* Singleton< PopupManager >::instance = NULL;

void PopupManager::Show(const tstring& aMsg ) {
	if(!activated)
		return;

	if (!Util::getAway() && BOOLSETTING(POPUP_AWAY)) {
		return;
	}

	if(!minimized && BOOLSETTING(POPUP_MINIMIZED)) 
		return;
	
	
	CRect rcDesktop;
	
	//get desktop rect so we know where to place the popup
	::SystemParametersInfo(SPI_GETWORKAREA,0,&rcDesktop,0);
	
	int screenHeight = rcDesktop.bottom;
	int screenWidth = rcDesktop.right;

	Lock l(cs);

	//if we have popups all the way up to the top of the screen do not create a new one
	if( (offset + height) > screenHeight)
		return;
	
	//get the handle of the window that has focus
	HWND gotFocus = ::SetFocus(WinUtil::mainWnd);
	
	//compute the window position
	CRect rc(screenWidth - width , screenHeight - height - offset, screenWidth, screenHeight - offset);
	
	//Create a new popup
	PopupWnd *p = new PopupWnd(aMsg, rc, hBitmap);
			
	//move the window to the top of the z-order and display it
	p->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_SHOWWINDOW);

	//restore focus to window
	::SetFocus(gotFocus);
	
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
	Lock l(cs);

	//we got nothing to do here
	if(popups.empty()) {
		return;
	}

	//check all popups and see if we need to remove anyone
	PopupList::iterator i = popups.begin();
	for(; i != popups.end(); ++i) {

		if((*i)->visible + SETTING(POPUP_TIMEOUT) * 1000 < GET_TICK()) {
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

	if(p == NULL){
		return;
	}

	//close the window and delete it
	p->SendMessage(WM_CLOSE, 0, 0);
	delete p;
	p = NULL;


	//set offset one window position lower
	dcassert(offset > 0);
	offset = offset - height;

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

void PopupManager::ShowPm(const tstring& nick, const tstring& msg){
	int pos = msg.find_first_of(_T(">"))+1;
	if(pos == tstring::npos )
		pos = 0;

	tstring s = TSTRING(POPUP_NEW_PM) + _T(" ") + nick + _T(" ") + TSTRING(POPUP_MSG) + msg.substr(pos);
	Show(s);
}

void PopupManager::ShowMC(const tstring& nick, const tstring& msg){
	tstring s = nick + _T(" ") + TSTRING(POPUP_SAYS) + msg;
	Show(s);
}

void PopupManager::ShowMC(const tstring& msg){
	int pos1 = msg.find_first_of(_T("<"));
	int pos2 = msg.find_first_of(_T(">"));
	
	//something wrong with the string, return
	if(pos1 == tstring::npos || pos2 == tstring::npos)
		return;

	ShowMC(msg.substr(pos1+1, pos2-pos1-1), msg.substr(pos2+1));
	
}

void PopupManager::ShowDisconnected(const tstring& hub) {
	tstring s = TSTRING(POPUP_DISCONNECTED) + hub;
	Show(s);
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