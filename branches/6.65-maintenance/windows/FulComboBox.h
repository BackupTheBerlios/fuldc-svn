/* 
* Copyright (C) 2003-2006 Pär Björklund, per.bjorklund@gmail.com
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

#if !defined(CFULCOMBOBOX_H)
#define CFULCOMBOBOX_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CFulComboBox : public CWindowImpl< CFulComboBox, CComboBox >
{
public:
	BEGIN_MSG_MAP(CFulComboBox)
		MESSAGE_HANDLER(WM_DRAWITEM, onDrawItem)
		MESSAGE_HANDLER(WM_MEASUREITEM, onMeasureItem)
	END_MSG_MAP()

	CFulComboBox() : fontSize(0), iconSpacing(4), height(16) { }
	~CFulComboBox() { }

	BOOL LoadImageList(tstring path, int cx, int nGrow, COLORREF crMask = CLR_DEFAULT, UINT uType = IMAGE_BITMAP, UINT uFlags = LR_LOADFROMFILE | LR_CREATEDIBSECTION);

	void SetImageList(CImageList &image);
	CImageList GetImageList() const;

	//message handlers
	LRESULT onDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onMeasureItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	
private:
	CImageList imageList;
	
	//this will be set in onMeasureItem and since we don't switch font while running it won't
	//be a problem if it's not updated
	int fontSize;

	const int height;
	const int iconSpacing;
};

#endif // _CFULCOMBOBOX_H
