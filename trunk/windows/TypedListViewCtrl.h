/* 
* Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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
	ColumnInfo(string aName, int aPos, int aFormat, int aWidth): name(aName), pos(aPos), width(aWidth), 
		format(aFormat), visible(true){}
		string name;
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
	TypedListViewCtrl() : sortColumn(-1), sortAscending(true) { };
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
			CHAR *buf = new CHAR[512];
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
				di->item.pszText = const_cast<char*>(((T*)di->item.lParam)->getText(j).c_str());

			delete[] buf;
		}
		return 0;
	}

	// Sorting
	LRESULT onColumnClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLISTVIEW* l = (NMLISTVIEW*)pnmh;
		int column = findColumn(l->iSubItem);
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
			return Util::stricmp(a.getText(0), b) == -1;
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
			} else if(comp == -1) {
				high = mid - 1;
			} else if(comp == 1) {
				low = mid + 1;
			}
		}

		comp = T::compareItems(a, b, sortColumn);
		if(!sortAscending)
			comp = -comp;
		if(comp == 1)
			mid++;

		return mid;
	}

	void setSortColumn(int aSortColumn) {
		sortColumn = aSortColumn;
		updateArrow();
	}
	int getSortColumn() { return sortColumn; }
	bool isAscending() { return sortAscending; }

	iterator begin() { return iterator(this); }
	iterator end() { return iterator(this, GetItemCount()); }

	int insertColumn(int nCol, string columnHeading, int nFormat = LVCFMT_LEFT, int nWidth = -1, int nSubItem = -1 ){
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

	LRESULT onHeaderMenu(UINT /*msg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ColumnInfo * ci = columnList[wParam];
		ci->visible = ! ci->visible;

		SetRedraw(FALSE);

		if(!ci->visible){
			removeColumn(ci);
		} else {
			int pos = GetHeader().GetItemCount();
			//InsertColumn(wParam, ci->name.c_str(), ci->format, ci->width, -1);
			InsertColumn(pos, ci->name.c_str(), ci->format, ci->width, wParam);
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
		char *buf = new char[128];
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
		StringTokenizer tok(vis, ',');
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


private:
	CMenu headerMenu;

	int sortColumn;
	bool sortAscending;
	
	static int CALLBACK compareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
		thisClass* t = (thisClass*)lParamSort;
		int result = T::compareItems((T*)lParam1, (T*)lParam2, t->sortColumn);
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

	//find the current position for the column that was inserted at the specified pos
	int findColumn(int col){
		CHAR *buf = new CHAR[512];
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

	int findColumn(ColumnInfo* ci){
		CHAR *buf = new CHAR[512];
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
