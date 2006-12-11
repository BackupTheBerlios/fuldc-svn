/*
 * Copyright (C) 2001-2006 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(TRANSFER_VIEW_H)
#define TRANSFER_VIEW_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/DownloadManager.h"
#include "../client/UploadManager.h"
#include "../client/CriticalSection.h"
#include "../client/ConnectionManagerListener.h"

#include "UCHandler.h"
#include "TypedListViewCtrl.h"
#include "ExListViewCtrl.h"
#include "WinUtil.h"
#include "FlatTabCtrl.h"
#include "TransfersManager.h"

class TransfersFrame : public MDITabChildWindowImpl<TransfersFrame>, 
	public CSplitterImpl<TransfersFrame, true>,
	public StaticFrame<TransfersFrame, ResourceManager::TRANSFERS, IDC_TRANSFERS>,
	public UserInfoBaseHandler<TransfersFrame>, 
	public UCHandler<TransfersFrame>,
	private TransfersManagerListener
{
public:
	DECLARE_FRAME_WND_CLASS_EX(_T("TransfersFrame"), IDR_TRANSFERS, 0, COLOR_3DFACE);

	TransfersFrame() : category(0) { };
	virtual ~TransfersFrame() { arrows.Destroy(); }

	typedef MDITabChildWindowImpl<TransfersFrame> baseClass;
	typedef CSplitterImpl<TransfersFrame, true> splitBase;
	typedef UserInfoBaseHandler<TransfersFrame> uibBase;
	typedef UCHandler<TransfersFrame> ucBase;

	BEGIN_MSG_MAP(TransfersFrame)
		NOTIFY_HANDLER(IDC_TRANSFERS, LVN_GETDISPINFO, ctrlTransfers.onGetDispInfo)
		NOTIFY_HANDLER(IDC_TRANSFERS, LVN_COLUMNCLICK, ctrlTransfers.onColumnClick)
		NOTIFY_HANDLER(IDC_TRANSFERS, LVN_KEYDOWN, onKeyDownTransfers)
		NOTIFY_HANDLER(IDC_TRANSFERS, NM_CUSTOMDRAW, onCustomDraw)
		NOTIFY_HANDLER(IDC_TRANSFERS, NM_DBLCLK, onDoubleClickTransfers)
		NOTIFY_HANDLER(IDC_CATEGORIES, LVN_ITEMCHANGED, onItemChanged)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		COMMAND_ID_HANDLER(IDC_FORCE, onForce)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDC_PM_UP, onPmAll)
		COMMAND_ID_HANDLER(IDC_PM_DOWN, onPmAll)
		COMMAND_ID_HANDLER(IDC_OPEN, onOpen)
		COMMAND_ID_HANDLER(IDC_OPEN_FOLDER, onOpen)
		COMMAND_ID_HANDLER(IDC_REMOVE_FILE, onRemoveFile)
		COMMAND_ID_HANDLER(IDC_SEARCH_ALTERNATES, onSearchAlternates)
		COMMAND_RANGE_HANDLER(IDC_COPY, IDC_COPY + COLUMN_LAST+2, onCopy)
		CHAIN_COMMANDS(ucBase)
		CHAIN_COMMANDS(uibBase)
		CHAIN_MSG_MAP(baseClass)
		CHAIN_MSG_MAP(splitBase)
	END_MSG_MAP()

	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	LRESULT onForce(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);			
	LRESULT onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onPmAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);			
	LRESULT onOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemoveFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onSearchAlternates(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDoubleClickTransfers(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT onItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	
	void runUserCommand(UserCommand& uc);

	LRESULT onKeyDownTransfers(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
		if(kd->wVKey == VK_DELETE) {
			ctrlTransfers.forEachSelected(&TransferInfo::disconnect);
		}
		return 0;
	}

	LRESULT onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		ctrlTransfers.forEachSelected(&TransferInfo::disconnect);
		return 0;
	}

	void UpdateLayout(BOOL bResizeBars = TRUE);

private:

public:
	TypedListViewCtrl<TransferInfo, IDC_TRANSFERS>& getUserList() { return ctrlTransfers; }
private:
	enum {
		ADD_ITEM,
		REMOVE_ITEM,
		UPDATE_ITEM
	};

	enum {
		ALL,
		DOWNLOADS,
		UPLOADS
	};

	enum {
		COLUMN_FIRST,
		COLUMN_USER = COLUMN_FIRST,
		COLUMN_HUB,
		COLUMN_STATUS,
		COLUMN_TIMELEFT,
		COLUMN_TOTALTIMELEFT,
		COLUMN_SPEED,
		COLUMN_FILE,
		COLUMN_SIZE,
		COLUMN_PATH,
		COLUMN_IP,
		COLUMN_RATIO,
		COLUMN_LAST
	};

	enum {
		IMAGE_DOWNLOAD = 0,
		IMAGE_UPLOAD
	};

	void speak(int event, TransferInfo* ti) { PostMessage(WM_SPEAKER, event, reinterpret_cast<LPARAM>(ti)); }

	TypedListViewCtrl<TransferInfo, IDC_TRANSFERS> ctrlTransfers;
	ExListViewCtrl ctrlCategories;

	static int columnIndexes[];
	static int columnSizes[];

	CMenu transferMenu;
	CMenu pmMenu;
	CMenu openMenu;
	CMenu copyMenu;
	CMenu ucMenu;

	CImageList arrows;
	CImageList categories;

	int category;

	typedef vector<TransferInfo*> Transfers;
	typedef Transfers::iterator TransferIter;

	Transfers transfers;

	virtual void on(Added, TransferInfo* ti) { speak(ADD_ITEM, ti); }
	virtual void on(Updated, TransferInfo* ti) { speak(UPDATE_ITEM, ti); }
	virtual void on(Removed, TransferInfo* ti) { speak(REMOVE_ITEM, ti); }
};

#endif // !defined(TRANSFER_VIEW_H)
