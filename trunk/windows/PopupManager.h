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
#include "../client/CriticalSection.h"

#include "PopupDlg.h"

#define DOWNLOAD_COMPLETE 6

class Popup
{
public:
	Popup(const string& aMsg, u_int32_t aVisible, HWND hWnd, CRect rc, HBITMAP hBitmap) : visible(aVisible) {
		dialog = new PopupWnd(hWnd, aMsg, rc, hBitmap);
	}
	Popup() {}

	~Popup() {
		if(dialog != NULL){
			//destroy the window before deleting the object
			//::SendMessageTimeout(dialog->m_hWnd, WM_CLOSE, 0, 0, SMTO_BLOCK, 1000, NULL);
			SendMessage(dialog->m_hWnd, WM_CLOSE, 0, 0);
			delete dialog;
			dialog = NULL;
		}
	}

	//popup and time of creation, no need to save position
	//as that can be retrieved with GetWindowRect()
	PopupWnd *dialog;
	u_int32_t visible;
};

class PopupManager : public Singleton< PopupManager >, private TimerManagerListener, 
	private QueueManagerListener
{
public:
	PopupManager() : height(90), width(200), offset(0), activated(true) {
		TimerManager::getInstance()->addListener(this);
		QueueManager::getInstance()->addListener(this);

		hBitmap = NULL;
		hBitmap = (HBITMAP)::LoadImage(NULL, "icons\\popup.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		if(hBitmap != NULL){
			BITMAP bm;
			GetObject(hBitmap,sizeof(bm),&bm);
			height = bm.bmHeight;
			width = bm.bmWidth;
		}
	}
	~PopupManager() {
		TimerManager::getInstance()->removeListener(this);
		QueueManager::getInstance()->removeListener(this);

		//destroy the bitmap
		::DeleteObject(hBitmap);
	}
	
	void SetHwnd(HWND &hWnd) {
		this->hWnd = hWnd;
	}

	//performs some formatting of the string, strips <nick>
	void ShowPm(const string& nick, const string& msg, HWND hWnd = NULL);
	
	//retrieves nick from <nick> and removes <nick> from msg
	void ShowMC(const string& msg, HWND hWnd = NULL );

	//the caller have to separate nick and format msg neatly
	void ShowMC(const string& nick, const string& msg, HWND hWnd = NULL);
	
	//maybe change this in the future, couldn't think of any other
	//important status messages
	void ShowDisconnected(const string& hub, HWND hWnd = NULL);

	//since we create a new string in onAction use a pointer here
	//and take care of cleanup
	void ShowDownloadComplete(string *msg);

	void Mute(bool mute) {
		activated = !mute;
	}

private:
		
	typedef deque< Popup* > PopupList;
	PopupList popups;
	
	//size of the popup window
	u_int16_t height;
	u_int16_t width;

	//if we have multiple windows displayed, 
	//keep track of where the new one will be displayed
	u_int16_t offset;
	
	//store handle to mainframe window
	HWND hWnd;

	//remove first popup in list and move everyone else
	void Remove();

	//used for thread safety
	CriticalSection cs;

	//turn on/off popups completely
	bool activated;

	//this is the background of the popup window
	HBITMAP hBitmap;
    	
	// TimerManagerListener
	virtual void onAction(TimerManagerListener::Types type, u_int32_t tick) throw();

	//QueueManagerListener
	virtual void onAction(QueueManagerListener::Types type, QueueItem* aQi) throw() {}
	virtual void onAction(QueueManagerListener::Types type, string msg) throw();

	//call this with a preformatted message
	void Show(HWND &hWnd, const string &aMsg);
	
};

#endif