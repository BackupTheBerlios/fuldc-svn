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
#include "Resource.h"

#include "FulComboBox.h"
#include "WinUtil.h"


BOOL CFulComboBox::LoadImageList(tstring path, int cx, int nGrow, COLORREF crMask, UINT uType, UINT uFlags) {
	return imageList.CreateFromImage(path.c_str(), cx, nGrow, crMask, uType, uFlags);
}

void CFulComboBox::SetImageList(CImageList &image) {
	imageList = image;
}

CImageList CFulComboBox::GetImageList() const {
	return imageList;
}

LRESULT CFulComboBox::onMeasureItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	LPMEASUREITEMSTRUCT mi = reinterpret_cast<LPMEASUREITEMSTRUCT>(lParam);

	int h = WinUtil::getTextHeight(m_hWnd, WinUtil::font);

	if(fontSize == 0)
		fontSize = h;

	if(h > height)
		mi->itemHeight = h;
	else 
		mi->itemHeight = height;
	
	mi->itemWidth = 0;

	return TRUE;
}

LRESULT CFulComboBox::onDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	DRAWITEMSTRUCT* di = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);

	int state = ::SaveDC(di->hDC);

	switch(di->itemAction) {
		case ODA_FOCUS:
			if(di->itemState & ODS_FOCUS) {
				if(!(di->itemState & ODS_NOFOCUSRECT))
					::DrawFocusRect(di->hDC, &di->rcItem);
			}
			break;
		case ODA_SELECT:
		case ODA_DRAWENTIRE:
			
			DWORD oldTextColor;
			DWORD oldBkColor;

			if(di->itemState & ODS_SELECTED) {
				oldTextColor = ::SetTextColor(di->hDC, ::GetSysColor(COLOR_HIGHLIGHTTEXT));
				oldBkColor = ::SetBkColor(di->hDC, ::GetSysColor(COLOR_HIGHLIGHT));
			} else {
				oldTextColor = ::SetTextColor(di->hDC, WinUtil::textColor);
				oldBkColor = ::SetBkColor(di->hDC, WinUtil::bgColor);
			}

			TCHAR* buf = NULL;
			int length = SendMessage(CB_GETLBTEXTLEN, di->itemID, 0);
			if(length != CB_ERR) {
				buf = new TCHAR[length+1];
				if(CB_ERR == SendMessage(CB_GETLBTEXT, di->itemID, reinterpret_cast<LPARAM>(buf))) {
					delete[] buf;
					buf = NULL;
				}
			}


			int size = di->rcItem.bottom - di->rcItem.top;

			int spacing = iconSpacing;

			if(!imageList.IsNull()) {
				//id of windows internally created controls, this is
				//what the listbox uses
				if(di->CtlID == 1000) 
					spacing = 16 + iconSpacing * 2;
				else 
					spacing = 16 + iconSpacing * 3;
			}

			if(buf) {
				::ExtTextOut(di->hDC, 
						spacing,
						di->rcItem.top + (size / 2) - (fontSize / 2),
						ETO_OPAQUE,
						&di->rcItem, 
						buf,
						length,
						NULL);
			}

			if(!imageList.IsNull() && di->itemID < imageList.GetImageCount()) {
				ImageList_Draw(	imageList.m_hImageList, 
							di->itemID, di->hDC, 
							di->rcItem.left + iconSpacing, 
							di->rcItem.top + (size / 2) - 8, 
							ILD_TRANSPARENT);
			}

			if(di->itemState & ODS_FOCUS) {
				if(!(di->itemState & ODS_NOFOCUSRECT))
					::DrawFocusRect(di->hDC, &di->rcItem);
			}

			::SetTextColor(di->hDC, oldTextColor);
			::SetBkColor(di->hDC, oldBkColor);

			if(buf)
				delete[] buf;
	}

	::RestoreDC(di->hDC, state);

	return TRUE;
}
