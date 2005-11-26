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

#if !defined(FINISHED_FRAME_H)
#define FINISHED_FRAME_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlatTabCtrl.h"
#include "TypedListViewCtrl.h"

#include "../client/FinishedManager.h"

class FinishedFrame : public MDITabChildWindowImpl<FinishedFrame>, public StaticFrame<FinishedFrame, ResourceManager::FINISHED_DOWNLOADS, IDC_FINISHED>,
	private FinishedManagerListener
{
public:
	FinishedFrame() : totalBytes(0), totalTime(0), closed(false) { };
	virtual ~FinishedFrame() { };

	DECLARE_FRAME_WND_CLASS_EX(_T("FinishedFrame"), IDR_FINISHED_DL, 0, COLOR_3DFACE);
		
	BEGIN_MSG_MAP(FinishedFrame)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_SETFOCUS, onSetFocus)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDC_TOTAL, onRemove)
		COMMAND_ID_HANDLER(IDC_VIEW_AS_TEXT, onViewAsText)
		COMMAND_ID_HANDLER(IDC_OPEN_FILE, onOpenFile)
		COMMAND_ID_HANDLER(IDC_OPEN_FOLDER, onOpenFolder)
		COMMAND_ID_HANDLER(IDC_GETLIST, onGetList)
		COMMAND_ID_HANDLER(IDC_GRANTSLOT, onGrant)
		COMMAND_RANGE_HANDLER(IDC_COPY, IDC_COPY+COLUMN_LAST+2, onCopy)
		NOTIFY_HANDLER(IDC_FINISHED, LVN_GETDISPINFO, ctrlList.onGetDispInfo)
		NOTIFY_HANDLER(IDC_FINISHED, LVN_COLUMNCLICK, ctrlList.onColumnClick)
		NOTIFY_HANDLER(IDC_FINISHED, LVN_KEYDOWN, onKeyDown)
		NOTIFY_HANDLER(IDC_FINISHED, NM_DBLCLK, onDoubleClick)
		CHAIN_MSG_MAP(MDITabChildWindowImpl<FinishedFrame>)
	END_MSG_MAP()
		
	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onDoubleClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onViewAsText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onOpenFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onOpenFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT onCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onGrant(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void UpdateLayout(BOOL bResizeBars = TRUE);

	LRESULT onSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /* bHandled */) {
		ctrlList.SetFocus();
		return 0;
	}

private:
	enum {
		SPEAK_ADD_LINE,
		SPEAK_REMOVE,
		SPEAK_REMOVE_ALL
	};

	enum {
		COLUMN_FIRST,
		COLUMN_FILE = COLUMN_FIRST,
		COLUMN_DONE,
		COLUMN_PATH,
		COLUMN_NICK,
		COLUMN_HUB,
		COLUMN_SIZE,
		COLUMN_SPEED,
		COLUMN_CRC32,
		COLUMN_LAST
	};

	class ItemInfo {
	public:
		ItemInfo(FinishedItem* fi) {
			entry = fi;

			columns[COLUMN_FILE]  = Text::toT(Util::getFileName(entry->getTarget()));
			columns[COLUMN_DONE]  = Text::toT(Util::formatTime("%Y-%m-%d %H:%M:%S", entry->getTime()));
			columns[COLUMN_PATH]  = Text::toT(Util::getFilePath(entry->getTarget()));
			columns[COLUMN_NICK]  = Text::toT(entry->getUser());
			columns[COLUMN_HUB]   = Text::toT(entry->getHub());
			columns[COLUMN_SIZE]  = Text::toT(Util::formatBytes(entry->getSize()));
			columns[COLUMN_SPEED] = Text::toT(Util::formatBytes(entry->getAvgSpeed()) + "/s");
			columns[COLUMN_CRC32] = entry->getCrc32Checked() ? TSTRING(YES_STR) : TSTRING(NO_STR);
		}
		tstring columns[COLUMN_LAST];

		tstring& getText(int col) {
			dcassert(col >= 0 && col < COLUMN_LAST);
			return columns[col];
		}

		static int compareItems(ItemInfo* a, ItemInfo* b, int col) {
			switch(col) {
				case COLUMN_SPEED:	return compare(a->entry->getAvgSpeed(), b->entry->getAvgSpeed());
				case COLUMN_SIZE:	return compare(a->entry->getSize(), b->entry->getSize());
				default:			return lstrcmpi(a->columns[col].c_str(), b->columns[col].c_str());
			}
			
		}

		FinishedItem* entry;
	};
	
	CStatusBarCtrl ctrlStatus;
	CMenu ctxMenu;
	CMenu copyMenu;
	
	TypedListViewCtrl<ItemInfo, IDC_FINISHED> ctrlList;
	
	int64_t totalBytes;
	int64_t totalTime;

	bool closed;

	static int columnSizes[COLUMN_LAST];
	static int columnIndexes[COLUMN_LAST];
	
	void updateStatus() {
		ctrlStatus.SetText(1, Text::toT(Util::toString(ctrlList.GetItemCount()) + ' ' + STRING(ITEMS)).c_str());
		ctrlStatus.SetText(2, Text::toT(Util::formatBytes(totalBytes)).c_str());
		ctrlStatus.SetText(3, Text::toT(Util::formatBytes((totalTime > 0) ? totalBytes * ((int64_t)1000) / totalTime : 0) + "/s").c_str());
	}

	void updateList(const FinishedItem::List& fl) {
		ctrlList.SetRedraw(FALSE);
		for(FinishedItem::List::const_iterator i = fl.begin(); i != fl.end(); ++i) {
			addEntry(*i);
		}
		ctrlList.SetRedraw(TRUE);
		ctrlList.Invalidate();
		updateStatus();
	}

	void addEntry(FinishedItem* entry);

	virtual void on(AddedDl, FinishedItem* entry) throw() {
		PostMessage(WM_SPEAKER, SPEAK_ADD_LINE, (WPARAM)entry);
	}
	virtual void on(RemovedDl, FinishedItem* entry) throw() { 
		totalBytes -= entry->getChunkSize();
		totalTime -= entry->getMilliSeconds();
		PostMessage(WM_SPEAKER, SPEAK_REMOVE);
	}
	virtual void on(RemovedAllDl) throw() { 
		PostMessage(WM_SPEAKER, SPEAK_REMOVE_ALL);
		totalBytes = 0;
		totalTime = 0;
	}
};

#endif // !defined(FINISHED_FRAME_H)

/**
 * @file
 * $Id: FinishedFrame.h,v 1.2 2004/01/06 01:52:10 trem Exp $
 */
