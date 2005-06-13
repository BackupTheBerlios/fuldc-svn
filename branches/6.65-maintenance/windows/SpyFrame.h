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

#if !defined(AFX_SPYFRAME_H__19A67830_B811_4672_BBC2_3D793E0342E8__INCLUDED_)
#define AFX_SPYFRAME_H__19A67830_B811_4672_BBC2_3D793E0342E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/ClientManager.h"
#include "../client/TimerManager.h"

#include "FlatTabCtrl.h"
#include "ExListViewCtrl.h"

#define IGNORETTH_MESSAGE_MAP 7

class SpyFrame : public MDITabChildWindowImpl<SpyFrame>, public StaticFrame<SpyFrame, ResourceManager::SEARCH_SPY, IDC_SEARCH_SPY>,
	private ClientManagerListener, private TimerManagerListener
{
public:
	SpyFrame() : total(0), cur(0), closed(false), ignoreTth(false), ignoreTthContainer(WC_BUTTON, this, IGNORETTH_MESSAGE_MAP) {
		ZeroMemory(perSecond, sizeof(perSecond));
		ClientManager::getInstance()->addListener(this);
		TimerManager::getInstance()->addListener(this);
	}

	virtual ~SpyFrame() {
	}

	enum {
		COLUMN_FIRST,
		COLUMN_STRING = COLUMN_FIRST,
		COLUMN_COUNT,
		COLUMN_TIME,
		COLUMN_LAST
	};

	static int columnIndexes[COLUMN_LAST];
	static int columnSizes[COLUMN_LAST];

	DECLARE_FRAME_WND_CLASS_EX(_T("SpyFrame"), IDR_SPY, 0, COLOR_3DFACE)

	virtual void OnFinalMessage(HWND /*hWnd*/) { delete this; }

	typedef MDITabChildWindowImpl<SpyFrame> baseClass;
	BEGIN_MSG_MAP(SpyFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		COMMAND_ID_HANDLER(IDC_SEARCH, onSearch)
		NOTIFY_HANDLER(IDC_RESULTS, LVN_COLUMNCLICK, onColumnClickResults)
		CHAIN_MSG_MAP(baseClass)
	ALT_MSG_MAP(IGNORETTH_MESSAGE_MAP)
		MESSAGE_HANDLER(BM_SETCHECK, onIgnoreTth)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onColumnClickResults(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);

	void UpdateLayout(BOOL bResizeBars = TRUE);

	LRESULT onIgnoreTth(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
		bHandled = FALSE;
		ignoreTth = (wParam == BST_CHECKED);
		return 0;
	}
	
private:

	enum { AVG_TIME = 60 };
	enum {
		SEARCH,
		TICK_AVG
	};

	ExListViewCtrl ctrlSearches;
	CStatusBarCtrl ctrlStatus;
	CContainedWindow ignoreTthContainer;
	CButton ctrlIgnoreTth;
	int total;
	int perSecond[AVG_TIME];
	int cur;
	tstring searchString;

	bool closed;
	bool ignoreTth;
	
	// ClientManagerListener
	virtual void on(ClientManagerListener::IncomingSearch, const string& s) throw();
	
	// TimerManagerListener
	virtual void on(TimerManagerListener::Second, u_int32_t) throw();
};

#endif // !defined(AFX_SPYFRAME_H__19A67830_B811_4672_BBC2_3D793E0342E8__INCLUDED_)

/**
 * @file
 * $Id: SpyFrame.h,v 1.2 2004/01/06 01:52:16 trem Exp $
 */
