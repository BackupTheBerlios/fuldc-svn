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

#if !defined(AFX_TYPEDLISTVIEWCTRL_H__45847002_68C2_4C8A_9C2D_C4D8F65DA841__INCLUDED_)
#define AFX_TYPEDLISTVIEWCTRL_H__45847002_68C2_4C8A_9C2D_C4D8F65DA841__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ListViewArrows.h"
#include "memdc.h"

class ColumnInfo {
public:
	ColumnInfo(const tstring &aName, int aPos, int aFormat, int aWidth): name(aName), pos(aPos), width(aWidth), 
		format(aFormat), visible(true){}
		tstring name;
		bool visible;
		int pos;
		int width;
		int format;
};

template<class T, int ctrlId>
class TypedListViewCtrl : public CWindowImpl<TypedListViewCtrl, CListViewCtrl, CControlWinTraits>,
	ListViewArrows<TypedListViewCtrl<T, ctrlId> >
{
public:
	TypedListViewCtrl() : sortColumn(-1), sortAscending(true), ownerDraw(false) { };
	~TypedListViewCtrl() {
		for(ColumnIter i = columnList.begin(); i != columnList.end(); ++i){
			delete (*i);
		}
	}

	typedef TypedListViewCtrl<T, ctrlId> thisClass;
	typedef CListViewCtrl baseClass;
	typedef ListViewArrows<thisClass> arrowBase;

	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_MENUCOMMAND, onHeaderMenu)
		MESSAGE_HANDLER(WM_CHAR, onChar)
		MESSAGE_HANDLER(WM_ERASEBKGND, onEraseBkgnd)
		MESSAGE_HANDLER(WM_PAINT, onPaint)
		CHAIN_MSG_MAP(arrowBase)
	END_MSG_MAP();

	class iterator : public ::iterator<random_access_iterator_tag, T*> {
	public:
		iterator() : typedList(NULL), cur(0), cnt(0) { }
		iterator(const iterator& rhs) : typedList(rhs.typedList), cur(rhs.cur), cnt(rhs.cnt) { }
		iterator& operator=(const iterator& rhs) { typedList = rhs.typedList; cur = rhs.cur; cnt = rhs.cnt; return *this; }

		bool operator==(const iterator& rhs) const { return cur == rhs.cur; }
		bool operator!=(const iterator& rhs) const { return !(*this == rhs); }
		bool operator<(const iterator& rhs) const { return cur < rhs.cur; }

		int operator-(const iterator& rhs) const { 
			return cur - rhs.cur;
		}

		iterator& operator+=(int n) { cur += n; return *this; };
		iterator& operator-=(int n) { return (cur += -n); };
		
		T& operator*() { return *typedList->getItemData(cur); }
		T* operator->() { return &(*(*this)); }
		T& operator[](int n) { return *typedList->getItemData(cur + n); }
		
		iterator operator++(int) {
			iterator tmp(*this);
			operator++();
			return tmp;
		}
		iterator& operator++() {
			++cur;
			return *this;
		}

	private:
		iterator(thisClass* aTypedList) : typedList(aTypedList), cur(aTypedList->GetNextItem(-1, LVNI_ALL)), cnt(aTypedList->GetItemCount()) { 
			if(cur == -1)
				cur = cnt;
		}
		iterator(thisClass* aTypedList, int first) : typedList(aTypedList), cur(first), cnt(aTypedList->GetItemCount()) { 
			if(cur == -1)
				cur = cnt;
		}
		friend class thisClass;
		thisClass* typedList;
		int cur;
		int cnt;
	};

	LRESULT onGetDispInfo(int /* idCtrl */, LPNMHDR pnmh, BOOL& /* bHandled */) {
		NMLVDISPINFO* di = (NMLVDISPINFO*)pnmh;
		if(di->item.mask & LVIF_TEXT) {
			di->item.mask |= LVIF_DI_SETITEM;
			int pos = di->item.iSubItem;
			bool insert = false;
			int j = 0;
			TCHAR *buf = new TCHAR[512];
			LVCOLUMN lvc;
			lvc.mask = LVCF_TEXT;
			lvc.pszText = buf;
			lvc.cchTextMax = 512;
			GetColumn(pos, &lvc);
			if(columnList.size() > 0){
				for(ColumnIter i = columnList.begin(); i != columnList.end(); ++i, ++j){
					if((Util::stricmp(buf, (*i)->name.c_str()) == 0)){
						if((*i)->visible == true)
							insert = true;
						break;
					}
				}
			} else {
				insert = true;
			}
			if(insert == true)
				di->item.pszText = const_cast<TCHAR*>(((T*)di->item.lParam)->getText(j).c_str());

			delete[] buf;
		}
		return 0;
	}

	// Sorting
	LRESULT onColumnClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
		int column = l->iSubItem;//findColumn(l->iSubItem);
		if(column != sortColumn) {
			sortAscending = true;
			sortColumn = column;
		} else if(sortAscending) {
			sortAscending = false;
		} else {
			sortColumn = -1;
		}
		updateArrow();
		resort();
		return 0;
	}
	void resort() {
		if(sortColumn != -1) {
			SortItems(&compareFunc, (LPARAM)this);
		}
	}

	int insertItem(T* item, int image) {
		return insertItem(getSortPos(item), item, image);
	}
	int insertItem(int i, T* item, int image) {
		return InsertItem(LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE, i, 
			LPSTR_TEXTCALLBACK, 0, 0, image, (LPARAM)item);
	}
	T* getItemData(int iItem) { return (T*)GetItemData(iItem); }
	T* getSelectedItem() { return (GetSelectedCount() > 0 ? getItemData(GetNextItem(-1, LVNI_SELECTED)) : NULL); }

	int findItem(T* item) { 
		LVFINDINFO fi = { LVFI_PARAM, NULL, (LPARAM)item };
		return FindItem(&fi, -1);
	}
	struct CompFirst {
		CompFirst() { } 
		bool operator()(T& a, const tstring& b) {
			return Util::stricmp(a.getText(0), b) < 0;
		}
	};
	int findItem(const tstring& b, int start = -1, bool aPartial = false) {
		LVFINDINFO fi = { aPartial ? LVFI_PARTIAL : LVFI_STRING, b.c_str() };
		return FindItem(&fi, start);
	}

	void forEach(void (T::*func)()) {
		int n = GetItemCount();
		for(int i = 0; i < n; ++i)
			(getItemData(i)->*func)();
	}
	void forEachSelected(void (T::*func)()) {
		int i = -1;
		while( (i = GetNextItem(i, LVNI_SELECTED)) != -1)
			(getItemData(i)->*func)();
	}
	template<class _Function>
	_Function forEachT(_Function pred) {
		int n = GetItemCount();
		for(int i = 0; i < n; ++i)
			pred(getItemData(i));
		return pred;
	}
	template<class _Function>
	_Function forEachSelectedT(_Function pred) {
		int i = -1;
		while( (i = GetNextItem(i, LVNI_SELECTED)) != -1)
			pred(getItemData(i));
		return pred;
	}
	
	void updateItem(int i) {
		int k = GetHeader().GetItemCount();
		for(int j = 0; j < k; ++j)
			SetItemText(i, j, LPSTR_TEXTCALLBACK);
	}
	void updateItem(T* item) { int i = findItem(item); if(i != -1) updateItem(i); };
	void deleteItem(T* item) { int i = findItem(item); if(i != -1) DeleteItem(i); };

	int getSortPos(T* a) {
		int high = GetItemCount();
		if((sortColumn == -1) || (high == 0))
			return high;

		high--;

		int low = 0;
		int mid = 0;
		T* b = NULL;
		int comp = 0;
		while( low <= high ) {
			mid = (low + high) / 2;
			b = getItemData(mid);
			comp = T::compareItems(a, b, sortColumn);
			
			if(!sortAscending)
				comp = -comp;

			if(comp == 0) {
				return mid;
			} else if(comp < 0) {
				high = mid - 1;
			} else if(comp > 0) {
				low = mid + 1;
			}
		}

		comp = T::compareItems(a, b, sortColumn);
		if(!sortAscending)
			comp = -comp;
		if(comp > 0)
			mid++;

		return mid;
	}

	void setSortColumn(int aSortColumn) {
		sortColumn = aSortColumn;
		updateArrow();
	}
	int getSortColumn() { return sortColumn; }
	int getRealSortColumn() { return findColumn(sortColumn); }
	bool isAscending() { return sortAscending; }

	iterator begin() { return iterator(this); }
	iterator end() { return iterator(this, GetItemCount()); }

	int insertColumn(int nCol, const tstring &columnHeading, int nFormat = LVCFMT_LEFT, int nWidth = -1, int nSubItem = -1 ){
		columnList.push_back(new ColumnInfo(columnHeading, nCol, nFormat, nWidth));
		return InsertColumn(nCol, columnHeading.c_str(), nFormat, nWidth, nSubItem);
	}

	void showMenu(POINT &pt){
		headerMenu.DestroyMenu();
		headerMenu.CreatePopupMenu();
		MENUINFO inf;
		inf.cbSize = sizeof(MENUINFO);
		inf.fMask = MIM_STYLE;
		inf.dwStyle = MNS_NOTIFYBYPOS;
		headerMenu.SetMenuInfo(&inf);

		int j = 0;
		for(ColumnIter i = columnList.begin(); i != columnList.end(); ++i, ++j) {
			headerMenu.AppendMenu(MF_STRING, IDC_HEADER_MENU, (*i)->name.c_str());
			if((*i)->visible)
				headerMenu.CheckMenuItem(j, MF_BYPOSITION | MF_CHECKED);
		}
		headerMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
	}

	/////////////////////////////////////////////////////////////////
	//this is a beginning of an owner drawn flicker free list control
	//doesn't work yet, i'll see if it's worth finishing one day
	/////////////////////////////////////////////////////////////////
	LRESULT onEraseBkgnd(UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		if(!ownerDraw) {
			bHandled = FALSE;
			return 1;
		}
		
		return 0;
	}
	
	LRESULT onPaint(UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		if(!ownerDraw) {
			bHandled = FALSE;
			return 1;
		}
		
		CRect rc;
		CRect crc;
		GetClientRect(&crc);

		if(GetUpdateRect(&rc, FALSE)) {
			CPaintDC dc(m_hWnd);
			dc.SetBkColor(WinUtil::bgColor);
			CMemDC memDC(&dc, &crc);

			memDC.SetBkMode(TRANSPARENT);
			memDC.SelectFont(WinUtil::font);
			memDC.SetMapMode(MM_TEXT);

			DRAWITEMSTRUCT dd;
			dd.hwndItem = m_hWnd;
			dd.hDC = memDC.m_hDC;
			
			int nVertPos = GetScrollPos(SB_VERT);

			for (int v=0; v<crc.Height()/16; v++) {
				dd.itemID = v+nVertPos;    
				DrawItem(&dd);
			}

			memDC.Paint();
		}

		return 0;
	}

	void DrawItem(LPDRAWITEMSTRUCT ld) 
	{
		int nCols = GetHeader().GetItemCount( );
		CRect rc;

		T* item = (T*)GetItemData(ld->itemID);

		if(!item)
			return;

		//////////////////////////
		//stuff for NM_CUSTOMDRAW
		/////////////////////////
		NMLVCUSTOMDRAW customDraw;
		customDraw

		LVITEM lvItem;
		lvItem.iItem = ld->itemID;
		lvItem.iSubItem = 0;
		lvItem.mask = LVIF_IMAGE | LVIF_STATE;
		lvItem.stateMask = LVIS_SELECTED;
		GetItem(&lvItem);

		COLORREF oldTextColor = 0;
		
		if(lvItem.state & LVIS_SELECTED) {
			CRect tmp;
			CRect itemRect;

			GetItemRect(ld->itemID, &itemRect, LVIR_BOUNDS);
			GetItemRect(ld->itemID, &tmp, LVIR_ICON);

			//this is sort of ugly but it will have to do for now
			//just want to get it working =)
			if(tmp.left < 10) {
				itemRect.left = tmp.right;

				::FillRect(ld->hDC, &itemRect, ::GetSysColorBrush(COLOR_HIGHLIGHT));
			} else {
				CRect bgRect = itemRect;
				bgRect.right = tmp.left;
				::FillRect(ld->hDC, &bgRect, ::GetSysColorBrush(COLOR_HIGHLIGHT));
				bgRect.left = tmp.right;
				bgRect.right = itemRect.right;
				::FillRect(ld->hDC, &bgRect, ::GetSysColorBrush(COLOR_HIGHLIGHT));
			}
			
			oldTextColor = ::SetTextColor(ld->hDC, ::GetSysColor(COLOR_HIGHLIGHTTEXT));
		}

		for (int nIndex=0; nIndex<nCols; nIndex++) 
		{
			//find the correct column, the col id might not
			//map to the correct column if some of them are hidden
			int col = findColumn(nIndex);

			LVCOLUMN lvColumn;
			lvColumn.mask = LVCF_ORDER | LVCF_FMT;
			GetColumn(nIndex, &lvColumn);

			//hmm the docs says that the index is one based
			//but the rect gets messed up if i use it, maybe i'm
			//doing something wrong
			GetSubItemRect(ld->itemID, nIndex, LVIR_LABEL , rc);
			
			if(nIndex == 0){
				if(imageList.m_hImageList) {
					HICON icon = imageList.GetIcon(lvItem.iImage);
					if(icon) {
						CRect tmp;
						GetSubItemRect(ld->itemID, nIndex, LVIR_ICON, tmp);
						
						::DrawIconEx(ld->hDC, tmp.left, tmp.top, icon, 16, 16, 0, NULL, DI_NORMAL | DI_COMPAT);
						
						//rc.left = tmp.right;
						DestroyIcon(icon);
					}
				}
			}

			//rc.DeflateRect(2, 2);
			::ExtTextOut(ld->hDC, rc.left, rc.top, ETO_CLIPPED, rc, item->getText(col).c_str(), item->getText(col).length(), NULL);
		}

		if(lvItem.state & LVIS_SELECTED) {
			::SetTextColor(ld->hDC, oldTextColor);
		}
	}

	CImageList SetImageList(HIMAGELIST hImageList, int nImageList)
	{
		imageList.Attach(hImageList);
		return CListViewCtrl::SetImageList(hImageList, nImageList);
		//return CImageList((HIMAGELIST)::SendMessage(m_hWnd, LVM_SETIMAGELIST, nImageList, (LPARAM)hImageList));
	}

	LRESULT onHeaderMenu(UINT /*msg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ColumnInfo * ci = columnList[wParam];
		ci->visible = ! ci->visible;

		SetRedraw(FALSE);

		if(!ci->visible){
			removeColumn(ci);
		} else {
			int pos = GetHeader().GetItemCount();
			InsertColumn(pos, ci->name.c_str(), ci->format, ci->width, static_cast<int>(wParam));
			LVCOLUMN lvcl = { 0 };
			lvcl.mask = LVCF_ORDER;
			lvcl.iOrder = ci->pos;
			SetColumn(pos, &lvcl);
		}

		SetRedraw();
		Invalidate();
		UpdateWindow();

		return 0;
	}

	void saveHeaderOrder(SettingsManager::StrSetting order, SettingsManager::StrSetting widths, 
		SettingsManager::StrSetting visible) throw() {
		string tmp, tmp2, tmp3;
		TCHAR *buf = new TCHAR[128];
		int size = GetHeader().GetItemCount();
		for(int i = 0; i < size; ++i){
			LVCOLUMN lvc;
			lvc.mask = LVCF_TEXT | LVCF_ORDER | LVCF_WIDTH;
			lvc.cchTextMax = 128;
			lvc.pszText = buf;
			GetColumn(i, &lvc);
			for(ColumnIter j = columnList.begin(); j != columnList.end(); ++j){
				if(Util::stricmp(buf, (*j)->name.c_str()) == 0){
					(*j)->pos = lvc.iOrder;
					(*j)->width = lvc.cx;
				}
			}
		}

		for(ColumnIter i = columnList.begin(); i != columnList.end(); ++i){
			ColumnInfo* ci = *i;

			if(ci->visible){
				tmp3 += "1,";
			} else {
				ci->pos = size++;
				tmp3 += "0,";
			}

			tmp += Util::toString(ci->pos);
			tmp += ',';

			tmp2 += Util::toString(ci->width);
			tmp2 += ',';
		}

		tmp.erase(tmp.size()-1, 1);
		tmp2.erase(tmp2.size()-1, 1);
		tmp3.erase(tmp3.size()-1, 1);
		SettingsManager::getInstance()->set(order, tmp);
		SettingsManager::getInstance()->set(widths, tmp2);
		SettingsManager::getInstance()->set(visible, tmp3);

	}

	void setVisible(string vis){
		StringTokenizer<string> tok(vis, ',');
		StringList l = tok.getTokens();

		StringIter i = l.begin();
		ColumnIter j = columnList.begin();
		for(; j != columnList.end() && i != l.end(); ++i, ++j){

			if(Util::toInt(*i) == 0){
				(*j)->visible = false;
				removeColumn(*j);
			}
		}
	}

	void setColumnOrderArray(int iCount, LPINT piArray ) {
		LVCOLUMN lvc;
		lvc.mask = LVCF_ORDER;
		for(int i = 0; i < iCount; ++i) {
			lvc.iOrder = columnList[i]->pos = piArray[i];
			SetColumn(i, &lvc);
		}
	}

	LRESULT onChar(UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		if((GetKeyState(VkKeyScan('A') & 0xFF) & 0xFF00) > 0 && (GetKeyState(VK_CONTROL) & 0xFF00) > 0){
			int count = GetItemCount();
			for(int i = 0; i < count; ++i)
				ListView_SetItemState(m_hWnd, i, LVIS_SELECTED, LVIS_SELECTED);

			return 0;
		}
		
		bHandled = FALSE;
		return 1;
	}

	//find the current position for the column that was inserted at the specified pos
	int findColumn(int col){
		TCHAR *buf = new TCHAR[512];
		LVCOLUMN lvcl;
		lvcl.mask = LVCF_TEXT;
		lvcl.pszText = buf;
		lvcl.cchTextMax = 512;

		GetColumn(col, &lvcl);

		int result = -1;

		int i = 0;
		for(ColumnIter j = columnList.begin(); j != columnList.end(); ++i, ++j){
			if(Util::stricmp((*j)->name.c_str(), buf) == 0){
				result = i;
				break;
			}
		}

		delete[] buf;

		return result;
	}					

	setOwnerDraw(bool handleDrawing) { ownerDraw = handleDrawing; }

	setStatusbarColumn(int col = -1) { drawStatusbarColumn = col; }

private:
	CMenu headerMenu;
	CImageList imageList;

	//should we handle the drawing?
	bool ownerDraw;

	//this is to keep track on custom draw operations
	int state;

	int sortColumn;
	bool sortAscending;
	
	static int CALLBACK compareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
		thisClass* t = (thisClass*)lParamSort;
		int result = T::compareItems((T*)lParam1, (T*)lParam2, t->getRealSortColumn());
		return (t->sortAscending ? result : -result);
	}

	typedef vector< ColumnInfo* > ColumnList;
	typedef ColumnList::iterator ColumnIter;

	ColumnList columnList;

	void removeColumn(ColumnInfo* ci){
		
		int column = findColumn(ci);

		if(column > -1){
			ci->width = GetColumnWidth(column);

			HDITEM hd;
			hd.mask = HDI_ORDER;
			GetHeader().GetItem(column, &hd);
			ci->pos = hd.iOrder;
			
			DeleteColumn(column);
			if(sortColumn == ci->pos)
				sortColumn = 0;
		}
		
	}

	int findColumn(ColumnInfo* ci){
		TCHAR *buf = new TCHAR[512];
		LVCOLUMN lvcl;
		lvcl.mask = LVCF_TEXT;
		lvcl.pszText = buf;
		lvcl.cchTextMax = 512;

		int columns = GetHeader().GetItemCount();

		int result = -1;

		for(int k = 0; k < columns; ++k){

			GetColumn(k, &lvcl);
			if(Util::stricmp(ci->name.c_str(), lvcl.pszText) == 0){
				result = k;
				break;
			}
		}

		delete[] buf;

		return result;
	}
};

#endif

/**
* @file
* $Id: TypedListViewCtrl.h,v 1.1 2003/12/15 16:52:08 trem Exp $
*/
