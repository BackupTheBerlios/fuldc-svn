/* 
* Copyright (C) 2003-2005 P�r Bj�rklund, per.bjorklund@gmail.com
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

#ifndef POPUPMANAGER_H
#define POPUPMANAGER_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "../client/Singleton.h"
#include "../client/TimerManager.h"
#include "../client/QueueManagerListener.h"
#include "../client/QueueManager.h"

#include "PopupDlg.h"
#include "WinUtil.h"

class PopupManager : public Singleton< PopupManager >, private TimerManagerListener, 
	private QueueManagerListener
{
public:
	PopupManager() : height(90), width(200), offset(0), activated(true), minimized(false), id(0) {
		TimerManager::getInstance()->addListener(this);
		QueueManager::getInstance()->addListener(this);

		hBitmap = (HBITMAP)::LoadImage(NULL, WinUtil::getIconPath(_T("popup.bmp")).c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		if(hBitmap != NULL){
			BITMAP bm;
			GetObject(hBitmap,sizeof(bm),&bm);
			height = (uint16_t)bm.bmHeight;
			width = (uint16_t)bm.bmWidth;
		}
	}
	~PopupManager() {
		TimerManager::getInstance()->removeListener(this);
		QueueManager::getInstance()->removeListener(this);

		//destroy the bitmap
		::DeleteObject(hBitmap);
	}
	
	//performs some formatting of the string, strips <nick>
	void ShowPm(const tstring& nick, const tstring& msg, HWND owner);
	
	//retrieves nick from <nick> and removes <nick> from msg
	void ShowMC(const tstring& msg, HWND owner);

	//the caller have to separate nick and format msg neatly
	void ShowMC(const tstring& nick, const tstring& msg, HWND owner);
	
	//maybe change this in the future, couldn't think of any other
	//important status messages
	void ShowDisconnected(const tstring& hub, HWND owner);

	//since we create a new string in onAction use a pointer here
	//and take care of cleanup
	void ShowDownloadComplete(tstring *msg);

	//remove the specified popup in list and move everyone else
	void Remove(int pos, bool clicked = false);

	//remove the popups that are scheduled to be removed
	void AutoRemove();

	void Mute(bool mute) {
		activated = !mute;
	}

	void Minimized(bool mini){
		minimized = mini;
	}

	bool isMuted() {
		return !activated;
	}

	bool isMinimised() {
		return minimized;
	}

private:
		
	typedef list< PopupWnd* > PopupList;
	typedef PopupList::iterator PopupIter;
	PopupList popups;
	
	//size of the popup window
	uint16_t height;
	uint16_t width;

	//if we have multiple windows displayed, 
	//keep track of where the new one will be displayed
	uint16_t offset;
	
	//turn on/off popups completely
	bool activated;

	//keep track of window state
	bool minimized;

	//this is the background of the popup window
	HBITMAP hBitmap;

	//id of the popup to keep track of them
	uint32_t id;
    	
	// TimerManagerListener
	virtual void on(TimerManagerListener::Second, time_t tick) throw();

	//QueueManagerListener
	virtual void on(QueueManagerListener::ReleaseDone, string msg) throw();

	//call this with a preformatted message
	void Show(const tstring &aMsg, HWND owner = NULL);
	
};

#endif