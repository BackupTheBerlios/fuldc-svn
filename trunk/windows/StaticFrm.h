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

#ifndef __STATICFRAME_H
#define __STATICFRAME_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"
#include "../client/DCPlusPlus.h"

template<class T, int title, int ID>
class StaticFrame {
public:
	~StaticFrame() { frame = NULL; };

	static T* frame;
	static void openWindow() {
		if(frame == NULL) {
			frame = new T();
			frame->CreateEx(WinUtil::mdiClient, frame->rcDefault, CTSTRING_I(ResourceManager::Strings(title)));
			::SendMessage(WinUtil::mainWnd, WM_USER, ID, TRUE);
		} else {
			
			if( (HWND)::SendMessage(WinUtil::mdiClient, WM_MDIGETACTIVE, 0, 0) == frame->m_hWnd){
				frame->PostMessage(WM_CLOSE);
			} else {
				frame->SendMessage(WM_MDIACTIVATE, (WPARAM)frame->m_hWnd, 0);
				::SetWindowPos(frame->m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
				::SendMessage(WinUtil::mainWnd, WM_USER, ID, TRUE );
				
			}
		}
	}

	void checkButton(bool check = true){
		::SendMessage(WinUtil::mainWnd, WM_USER, ID, check ? TRUE : FALSE );
	}
};

template<class T, int title, int ID>
T* StaticFrame<T, title, ID>::frame = NULL;

#endif