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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "FinishedFrame.h"
#include "WinUtil.h"
#include "TextFrame.h"

#include "../client/ClientManager.h"
#include "../client/StringTokenizer.h"

int FinishedFrame::columnIndexes[] = { COLUMN_DONE, COLUMN_FILE, COLUMN_PATH, COLUMN_NICK, COLUMN_HUB, COLUMN_SIZE, COLUMN_SPEED, COLUMN_CRC32 };
int FinishedFrame::columnSizes[] = { 100, 110, 290, 125, 80, 80, 80, 80 };
static ResourceManager::Strings columnNames[] = { ResourceManager::FILENAME, ResourceManager::TIME, ResourceManager::PATH, 
ResourceManager::NICK, ResourceManager::HUB, ResourceManager::SIZE, ResourceManager::SPEED, ResourceManager::CRC_CHECKED
};

LRESULT FinishedFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	
	ctrlList.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_FINISHED);
	ctrlList.SetExtendedListViewStyle(LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);

	ctrlList.SetImageList(WinUtil::fileImages, LVSIL_SMALL);
	ctrlList.SetBkColor(WinUtil::bgColor);
	ctrlList.SetTextBkColor(WinUtil::bgColor);
	ctrlList.SetTextColor(WinUtil::textColor);
	
	// Create listview columns
	WinUtil::splitTokens(columnIndexes, SETTING(FINISHED_ORDER), COLUMN_LAST);
	WinUtil::splitTokens(columnSizes, SETTING(FINISHED_WIDTHS), COLUMN_LAST);
	
	for(int j=0; j<COLUMN_LAST; j++) {
		int fmt = (j == COLUMN_SIZE || j == COLUMN_SPEED) ? LVCFMT_RIGHT : LVCFMT_LEFT;
		ctrlList.InsertColumn(j, CTSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
	}
	
	ctrlList.SetColumnOrderArray(COLUMN_LAST, columnIndexes);
	ctrlList.setVisible(SETTING(FINISHED_VISIBLE));
	ctrlList.setSortColumn(COLUMN_DONE);
	
	
	UpdateLayout();
	
	FinishedManager::getInstance()->addListener(this);
	updateList(FinishedManager::getInstance()->lockList());
	FinishedManager::getInstance()->unlockList();
	
	copyMenu.CreatePopupMenu();
	ctrlList.buildCopyMenu(copyMenu);

	ctxMenu.CreatePopupMenu();
	ctxMenu.AppendMenu(MF_STRING, IDC_VIEW_AS_TEXT, CTSTRING(VIEW_AS_TEXT));
	ctxMenu.AppendMenu(MF_STRING, IDC_OPEN_FILE, CTSTRING(OPEN));
	ctxMenu.AppendMenu(MF_STRING, IDC_OPEN_FOLDER, CTSTRING(OPEN_FOLDER));
	ctxMenu.AppendMenu(MF_STRING, IDC_GRANTSLOT, CTSTRING(GRANT_EXTRA_SLOT));
	ctxMenu.AppendMenu(MF_STRING, IDC_GETLIST, CTSTRING(GET_FILE_LIST));
	ctxMenu.AppendMenu(MF_POPUP, copyMenu, CTSTRING(COPY));
	ctxMenu.AppendMenu(MF_SEPARATOR);
	ctxMenu.AppendMenu(MF_STRING, IDC_REMOVE, CTSTRING(REMOVE));
	ctxMenu.AppendMenu(MF_STRING, IDC_TOTAL, CTSTRING(REMOVE_ALL));
	ctxMenu.SetMenuDefaultItem(IDC_OPEN_FILE);

	WinUtil::SetIcon(m_hWnd, _T("FinishedDL.ico"));

	bHandled = FALSE;
	return TRUE;
}

LRESULT FinishedFrame::onContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (reinterpret_cast<HWND>(wParam) == ctrlList && ctrlList.GetSelectedCount() > 0) { 
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		
		if(pt.x == -1 && pt.y == -1) {
			WinUtil::getContextMenuPos(ctrlList, pt);
		}

		ctxMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);			
		return TRUE; 
	}
	bHandled = FALSE;
	return FALSE; 
}

void FinishedFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */) {
	RECT rect;
	GetClientRect(&rect);

	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);

	if(ctrlStatus.IsWindow()) {
		CRect sr;
		int w[4];
		ctrlStatus.GetClientRect(sr);
		w[3] = sr.right - 16;
		w[2] = max(w[3] - 100, 0);
		w[1] = max(w[2] - 100, 0);
		w[0] = max(w[1] - 100, 0);

		ctrlStatus.SetParts(4, w);
	}

	CRect rc(rect);
	ctrlList.MoveWindow(rc);
}

LRESULT FinishedFrame::onDoubleClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	
	NMITEMACTIVATE * const item = (NMITEMACTIVATE*) pnmh;

	if(item->iItem != -1) {
		ItemInfo *ii = ctrlList.getItemData(item->iItem);
		WinUtil::openFile(Text::toT(ii->entry->getTarget()));
	}
	return 0;
}

LRESULT FinishedFrame::onViewAsText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int i;
	if((i = ctrlList.GetNextItem(-1, LVNI_SELECTED)) != -1) {
		ItemInfo *ii = ctrlList.getItemData(i);
		TextFrame::openWindow(Text::toT(ii->entry->getTarget()));
	}
	return 0;
}

LRESULT FinishedFrame::onOpenFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int i;
	if((i = ctrlList.GetNextItem(-1, LVNI_SELECTED)) != -1) {
		ItemInfo *ii = ctrlList.getItemData(i);
		WinUtil::openFile(Text::toT(ii->entry->getTarget()));
	}
	return 0;
}

LRESULT FinishedFrame::onOpenFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int i;
	if((i = ctrlList.GetNextItem(-1, LVNI_SELECTED)) != -1) {
		ItemInfo *ii = ctrlList.getItemData(i);
		::ShellExecute(NULL, NULL, Text::toT(Util::getFilePath(ii->entry->getTarget())).c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
	return 0;
}

LRESULT FinishedFrame::onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i;
	if((i = ctrlList.GetNextItem(-1, LVNI_SELECTED)) != -1) {
		ItemInfo *ii = ctrlList.getItemData(i);
		User::Ptr u = ClientManager::getInstance()->getUser(ii->entry->getUser());
		QueueManager::getInstance()->addList(u, QueueItem::FLAG_CLIENT_VIEW);
	}
	return 0;
}

LRESULT FinishedFrame::onGrant(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i;
	if((i = ctrlList.GetNextItem(-1, LVNI_SELECTED)) != -1) {
		ItemInfo *ii = ctrlList.getItemData(i);
		User::Ptr u = ClientManager::getInstance()->getUser(ii->entry->getUser());
		UploadManager::getInstance()->reserveSlot(u);
	}
	return 0;
}

LRESULT FinishedFrame::onRemove(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	switch(wID)
	{
	case IDC_REMOVE:
		{
			int i = -1;
			while((i = ctrlList.GetNextItem(-1, LVNI_SELECTED)) != -1) {
				ItemInfo *ii = ctrlList.getItemData(i);
				FinishedManager::getInstance()->remove(ii->entry);
				ctrlList.DeleteItem(i);
				delete ii;
			}
			break;
		}
	case IDC_TOTAL:
		FinishedManager::getInstance()->removeAll();
		break;
	}
	return 0;
}

LRESULT FinishedFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	if(!closed) {
		FinishedManager::getInstance()->removeListener(this);

		closed = true;
        PostMessage(WM_CLOSE);
		return 0;
	} else {
		//@todo switch to typedlist style save function
		ctrlList.saveHeaderOrder(SettingsManager::FINISHED_ORDER, 
			SettingsManager::FINISHED_WIDTHS, SettingsManager::FINISHED_VISIBLE);

		//cleanup to avoid memory leak
		for(int i = 0; i < ctrlList.GetItemCount(); ++i) {
			delete ctrlList.getItemData(i);
		}
		ctrlList.DeleteAllItems();
		
		checkButton(false);

		bHandled = FALSE;
		return 0;
	}
}

LRESULT FinishedFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if(wParam == SPEAK_ADD_LINE) {
		FinishedItem* entry = reinterpret_cast<FinishedItem*>(lParam);
		addEntry(entry);
		if(BOOLSETTING(FINISHED_DIRTY))
			setDirty();
		updateStatus();
	} else if(wParam == SPEAK_REMOVE) {
		updateStatus();
	} else if(wParam == SPEAK_REMOVE_ALL) {
		for(int i = 0; i < ctrlList.GetItemCount(); ++i) {
			delete ctrlList.getItemData(i);
		}
		ctrlList.DeleteAllItems();
		updateStatus();
	}
	return 0;
}

void FinishedFrame::addEntry(FinishedItem* entry) {
	ItemInfo *ii = new ItemInfo(entry);

	totalBytes += entry->getChunkSize();
	totalTime += entry->getMilliSeconds();

	int image = WinUtil::getIconIndex(Text::toT(entry->getTarget()));
	int loc = ctrlList.insertItem(ii, image);
	ctrlList.EnsureVisible(loc, FALSE);
}

LRESULT FinishedFrame::onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMLVKEYDOWN* kd = reinterpret_cast<NMLVKEYDOWN*>(pnmh);

	if(kd->wVKey == VK_DELETE) {
		PostMessage(WM_COMMAND, IDC_REMOVE);
	} 
	return 0;
}

LRESULT FinishedFrame::onCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ctrlList.copy(wID - IDC_COPY);
	return 0;
}


/**
 * @file
 * $Id: FinishedFrame.cpp,v 1.2 2004/01/06 01:52:09 trem Exp $
 */
