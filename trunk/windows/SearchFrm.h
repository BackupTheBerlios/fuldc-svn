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

#if !defined(AFX_SEARCHFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)
#define AFX_SEARCHFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "FlatTabCtrl.h"
#include "TypedListViewCtrl.h"
#include "WinUtil.h"

#include "../regex/pme.h"

#include "../client/Client.h"
#include "../client/SearchManager.h"
#include "../client/CriticalSection.h"
#include "../client/ClientManagerListener.h"
#include "../client/TimerManager.h"

#include "UCHandler.h"

#define SEARCH_MESSAGE_MAP 6		// This could be any number, really...
#define SHOWUI_MESSAGE_MAP 7

class SearchFrame : public MDITabChildWindowImpl<SearchFrame>,
	private SearchManagerListener, private ClientManagerListener, 
	public UCHandler<SearchFrame>, public UserInfoBaseHandler<SearchFrame>,
	private TimerManagerListener
{
public:
	static void openWindow(const string& str = Util::emptyString, LONGLONG size = 0, SearchManager::SizeModes mode = SearchManager::SIZE_ATLEAST, SearchManager::TypeModes type = SearchManager::TYPE_ANY);
	
	DECLARE_FRAME_WND_CLASS_EX("SearchFrame", 0, 0, COLOR_3DFACE)

	typedef MDITabChildWindowImpl<SearchFrame> baseClass;
	typedef UCHandler<SearchFrame> ucBase;
	typedef UserInfoBaseHandler<SearchFrame> uicBase;

	BEGIN_MSG_MAP(SearchFrame)
		NOTIFY_HANDLER(IDC_RESULTS, LVN_GETDISPINFO, ctrlResults.onGetDispInfo)
		NOTIFY_HANDLER(IDC_RESULTS, LVN_COLUMNCLICK, ctrlResults.onColumnClick)
		NOTIFY_HANDLER(IDC_HUB, LVN_GETDISPINFO, ctrlHubs.onGetDispInfo)
		NOTIFY_HANDLER(IDC_RESULTS, NM_DBLCLK, onDoubleClickResults)
		NOTIFY_HANDLER(IDC_RESULTS, LVN_KEYDOWN, onKeyDown)
		NOTIFY_HANDLER(IDC_HUB, LVN_ITEMCHANGED, onItemChangedHub)
		MESSAGE_HANDLER(WM_CREATE, onCreate)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		MESSAGE_HANDLER(WM_CONTEXTMENU, onContextMenu)
		MESSAGE_HANDLER(WM_SPEAKER, onSpeaker)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, onCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORLISTBOX, onCtlColor)
		MESSAGE_HANDLER(WM_CLOSE, onClose)
		COMMAND_ID_HANDLER(IDC_DOWNLOAD, onDownload)
		COMMAND_ID_HANDLER(IDC_DOWNLOADTO, onDownloadTo)
		COMMAND_ID_HANDLER(IDC_DOWNLOADDIR, onDownloadWhole)
		COMMAND_ID_HANDLER(IDC_DOWNLOADDIRTO, onDownloadWholeTo)
		COMMAND_ID_HANDLER(IDC_VIEW_AS_TEXT, onViewAsText)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDC_SEARCH, onSearch)
		COMMAND_ID_HANDLER(IDC_FREESLOTS, onFreeSlots)
		COMMAND_ID_HANDLER(IDC_GETLIST, onGetList)
		COMMAND_RANGE_HANDLER(IDC_DOWNLOAD_TARGET, IDC_DOWNLOAD_TARGET + downloadPaths.size() + targets.size() + WinUtil::lastDirs.size(), onDownloadTarget)
		COMMAND_RANGE_HANDLER(IDC_DOWNLOAD_WHOLE_TARGET, IDC_DOWNLOAD_WHOLE_TARGET + downloadPaths.size() + WinUtil::lastDirs.size(), onDownloadWholeTarget)
		COMMAND_RANGE_HANDLER(IDC_COPY, IDC_COPY+COLUMN_LAST, onCopy)
		COMMAND_ID_HANDLER(IDC_SEARCH_BY_TTH, onSearchByTTH)
		COMMAND_ID_HANDLER(IDC_BITZI_LOOKUP, onBitziLookup)
		COMMAND_ID_HANDLER(IDC_COPY_MAGNET, onCopyMagnet)
		COMMAND_RANGE_HANDLER(IDC_DOWNLOAD_TARGET, IDC_DOWNLOAD_TARGET + targets.size() + WinUtil::lastDirs.size(), onDownloadTarget)
		COMMAND_RANGE_HANDLER(IDC_DOWNLOAD_WHOLE_TARGET, IDC_DOWNLOAD_WHOLE_TARGET + WinUtil::lastDirs.size(), onDownloadWholeTarget)
		CHAIN_COMMANDS(ucBase)
		CHAIN_COMMANDS(uicBase)
		CHAIN_MSG_MAP(baseClass)
	ALT_MSG_MAP(SEARCH_MESSAGE_MAP)
		MESSAGE_HANDLER(WM_CHAR, onChar)
		MESSAGE_HANDLER(WM_KEYDOWN, onChar)
		MESSAGE_HANDLER(WM_KEYUP, onChar)
	ALT_MSG_MAP(SHOWUI_MESSAGE_MAP)
		MESSAGE_HANDLER(BM_SETCHECK, onShowUI)
	END_MSG_MAP()

	SearchFrame() : 
	searchBoxContainer("COMBOBOX", this, SEARCH_MESSAGE_MAP),
		searchContainer("edit", this, SEARCH_MESSAGE_MAP), 
		sizeContainer("edit", this, SEARCH_MESSAGE_MAP), 
		modeContainer("COMBOBOX", this, SEARCH_MESSAGE_MAP),
		sizeModeContainer("COMBOBOX", this, SEARCH_MESSAGE_MAP),
		fileTypeContainer("COMBOBOX", this, SEARCH_MESSAGE_MAP),
		showUIContainer("BUTTON", this, SHOWUI_MESSAGE_MAP),
		slotsContainer("BUTTON", this, SEARCH_MESSAGE_MAP),
		doSearchContainer("BUTTON", this, SEARCH_MESSAGE_MAP),
		resultsContainer(WC_LISTVIEW, this, SEARCH_MESSAGE_MAP),
		hubsContainer(WC_LISTVIEW, this, SEARCH_MESSAGE_MAP),
		filterBoxContainer("COMBOBOX", this, SEARCH_MESSAGE_MAP),
		filterContainer("edit", this, SEARCH_MESSAGE_MAP),
		lastSearch(0), initialSize(0), initialMode(SearchManager::SIZE_ATLEAST), initialType(SearchManager::TYPE_ANY),
		showUI(true), onlyFree(false), closed(false), isHash(false), useRegExp(false), results(0), filtered(0)
	{	
		SearchManager::getInstance()->addListener(this);
		TimerManager::getInstance()->addListener(this);
		StringTokenizer token(SETTING(DOWNLOAD_TO_PATHS), "|");
		downloadPaths = token.getTokens();
	}

	virtual ~SearchFrame() {
		TimerManager::getInstance()->removeListener(this);
	}
	virtual void OnFinalMessage(HWND /*hWnd*/) { delete this; }

	LRESULT onChar(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onClose(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT onCtlColor(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onDoubleClickResults(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT onDownloadTarget(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadWholeTarget(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onDownloadWholeTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT onCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		
	LRESULT onSearchByTTH(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onBitziLookup(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onCopyMagnet(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
	void UpdateLayout(BOOL bResizeBars = TRUE);
	void runUserCommand(UserCommand& uc);

	void removeSelected() {
		int i = -1;
		while( (i = ctrlResults.GetNextItem(-1, LVNI_SELECTED)) != -1) {
			delete ctrlResults.getItemData(i);
			ctrlResults.DeleteItem(i);
		}
	}
	
	LRESULT onDownload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		ctrlResults.forEachSelectedT(SearchInfo::Download(SETTING(DOWNLOAD_DIRECTORY)));
		return 0;
	}

	LRESULT onViewAsText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		ctrlResults.forEachSelected(&SearchInfo::view);
		return 0;
	}

	LRESULT onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT onDownloadWhole(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		ctrlResults.forEachSelectedT(SearchInfo::DownloadWhole(SETTING(DOWNLOAD_DIRECTORY)));
		return 0;
	}
	
	LRESULT onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		removeSelected();
		return 0;
	}

	LRESULT onFreeSlots(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		onlyFree = (ctrlSlots.GetCheck() == 1);
		return 0;
	}

	LRESULT onSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		onEnter();
		return 0;
	}

	LRESULT onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
		
		if(kd->wVKey == VK_DELETE) {
			removeSelected();
		} 
		return 0;
	}

	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		if(::IsWindow(ctrlSearch))
			ctrlSearch.SetFocus();
		return 0;
	}

	LRESULT onShowUI(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled) {
		bHandled = FALSE;
		showUI = (wParam == BST_CHECKED);
		UpdateLayout(FALSE);
		return 0;
	}

	void SearchFrame::setInitial(const string& str, LONGLONG size, SearchManager::SizeModes mode, SearchManager::TypeModes type) {
		initialString = str; initialSize = size; initialMode = mode; initialType = type;
	}
	
private:
	class SearchInfo;
public:
	TypedListViewCtrl<SearchInfo, IDC_RESULTS>& getUserList() { return ctrlResults; };

private:
	enum {
		COLUMN_FIRST,
		COLUMN_FILENAME = COLUMN_FIRST,
		COLUMN_NICK,
		COLUMN_TYPE,
		COLUMN_SIZE,
		COLUMN_PATH,
		COLUMN_SLOTS,
		COLUMN_CONNECTION,
		COLUMN_HUB,
		COLUMN_EXACT_SIZE,
		COLUMN_IP,
		COLUMN_TTH,
		COLUMN_LAST
	};

	class SearchInfo : public UserInfoBase {
	public:
		SearchResult* sr;

		SearchInfo(SearchResult* aSR) : UserInfoBase(aSR->getUser()), sr(aSR) { 
			sr->incRef(); update();
		};
		~SearchInfo() { 
			sr->decRef(); 
		};

		void getList();
		void view();
		struct Download {
			Download(const string& aTarget) : tgt(aTarget) { };
			void operator()(SearchInfo* si);
			const string& tgt;
		};
		struct DownloadWhole {
			DownloadWhole(const string& aTarget) : tgt(aTarget) { };
			void operator()(SearchInfo* si);
			const string& tgt;
		};
		struct DownloadTarget {
			DownloadTarget(const string& aTarget) : tgt(aTarget) { };
			void operator()(SearchInfo* si);
			const string& tgt;
		};
		struct CheckSize {
			CheckSize() : size(-1), op(true), oneHub(true) { };
			void operator()(SearchInfo* si);
			string ext;
			int64_t size;
			bool oneHub;
			string hub;
			bool op;
		};

		const string& getText(int col) const {
			switch(col) {
				case COLUMN_NICK: return sr->getUser()->getNick();
				case COLUMN_FILENAME: return fileName;
				case COLUMN_TYPE: return type;
				case COLUMN_SIZE: return size;
				case COLUMN_PATH: return path;
				case COLUMN_SLOTS: return slots;
				case COLUMN_CONNECTION: return sr->getUser()->getConnection();
				case COLUMN_HUB: return sr->getHubName();
				case COLUMN_EXACT_SIZE: return exactSize;
				case COLUMN_IP: return ip;
				case COLUMN_TTH: return tth;
				default: return Util::emptyString;
			}
		}

		static int compareItems(SearchInfo* a, SearchInfo* b, int col) {

			switch(col) {
				case COLUMN_NICK: return Util::stricmp(a->sr->getUser()->getNick(), b->sr->getUser()->getNick());
				case COLUMN_FILENAME: return Util::stricmp(a->fileName, b->fileName);
				case COLUMN_TYPE: 
					if(a->sr->getType() == b->sr->getType())
						return Util::stricmp(a->type, b->type);
					else
						return(a->sr->getType() == SearchResult::TYPE_DIRECTORY) ? -1 : 1;
				case COLUMN_SIZE: return compare(a->sr->getSize(), b->sr->getSize());
				case COLUMN_PATH: return Util::stricmp(a->path, b->path);
				case COLUMN_SLOTS: 
					if(a->sr->getFreeSlots() == b->sr->getFreeSlots())
						return compare(a->sr->getSlots(), b->sr->getSlots());
					else
						return compare(a->sr->getFreeSlots(), b->sr->getFreeSlots());
				case COLUMN_CONNECTION: return Util::stricmp(a->sr->getUser()->getConnection(), b->sr->getUser()->getConnection());
				case COLUMN_HUB: return Util::stricmp(a->sr->getHubName(), b->sr->getHubName());
				case COLUMN_EXACT_SIZE: return compare(a->sr->getSize(), b->sr->getSize());
				case COLUMN_IP: return Util::stricmp(a->getIP(), b->getIP());
				case COLUMN_TTH: return Util::stricmp(a->getTTH(), b->getTTH());
				default: return 0;
			}
		}

		void update() { 
			if(sr->getType() == SearchResult::TYPE_FILE) {
				if(sr->getFile().rfind('\\') == string::npos) {
					fileName = sr->getFile();
				} else {
					fileName = Util::getFileName(sr->getFile());
					path = Util::getFilePath(sr->getFile());
				}

				type = Util::getFileExt(fileName);
				if(!type.empty() && type[0] == '.')
					type.erase(0, 1);
				size = Util::formatBytes(sr->getSize());
				exactSize = Util::formatExactSize(sr->getSize());
			} else {
				fileName = sr->getFileName();
				path = sr->getFile();
				type = STRING(DIRECTORY);
			}
			slots = sr->getSlotString();
			ip = sr->getIP();
			if(sr->getTTH() != NULL)
				setTTH(sr->getTTH()->toBase32());
		}

		GETSET(string, fileName, FileName);
		GETSET(string, path, Path);
		GETSET(string, type, Type);
		GETSET(string, size, Size);
		GETSET(string, slots, Slots);
		GETSET(string, exactSize, ExactSize);
		GETSET(string, ip, IP);
		GETSET(string, tth, TTH);
	};

	struct HubInfo : public FastAlloc<HubInfo> {
		HubInfo(const string& aIpPort, const string& aName, bool aOp) : ipPort(aIpPort),
			name(aName), op(aOp) { };

		const string& getText(int col) const {
			return (col == 0) ? name : Util::emptyString;
		}
		static int compareItems(HubInfo* a, HubInfo* b, int col) {
			return (col == 0) ? Util::stricmp(a->name, b->name) : 0;
		}
		string ipPort;
		string name;
		bool op;
	};

	// WM_SPEAKER
	enum Speakers {
		ADD_RESULT,
		HUB_ADDED,
		HUB_CHANGED,
		HUB_REMOVED,
		STATS
	};

	string initialString;
	int64_t initialSize;
	SearchManager::SizeModes initialMode;
	SearchManager::TypeModes initialType;

	CStatusBarCtrl ctrlStatus;
	CEdit ctrlSearch;
	CComboBox ctrlSearchBox;
	CEdit ctrlFilter;
	CComboBox ctrlFilterBox;
	CEdit ctrlSize;
	CComboBox ctrlMode;
	CComboBox ctrlSizeMode;
	CComboBox ctrlFiletype;
	CButton ctrlDoSearch;
	
	CContainedWindow searchContainer;
	CContainedWindow searchBoxContainer;
	CContainedWindow sizeContainer;
	CContainedWindow modeContainer;
	CContainedWindow sizeModeContainer;
	CContainedWindow fileTypeContainer;
	CContainedWindow slotsContainer;
	CContainedWindow showUIContainer;
	CContainedWindow doSearchContainer;
	CContainedWindow resultsContainer;
	CContainedWindow hubsContainer;
	CContainedWindow filterContainer;
	CContainedWindow filterBoxContainer;

 	CStatic searchLabel, sizeLabel, optionLabel, typeLabel, hubsLabel, filterLabel;
	CButton ctrlSlots, ctrlShowUI;
	bool showUI;

	TypedListViewCtrl<SearchInfo, IDC_RESULTS> ctrlResults;
	TypedListViewCtrl<HubInfo, IDC_HUB> ctrlHubs;

	CMenu resultsMenu;
	CMenu targetMenu;
	CMenu targetDirMenu;
	CMenu copyMenu;
	
	StringList search;
	StringList targets;
	StringList wholeTargets;
	StringList downloadPaths;
	StringList filterList;

	PME filterRegExp;
	bool useRegExp;

	/** Parameter map for user commands */
	StringMap ucParams;

	bool onlyFree;
	bool isHash;

	int results;
	int filtered;

	CriticalSection cs;

	static StringList lastSearches;
	static StringList lastFilters;

	DWORD lastSearch;
	bool closed;

	static int columnIndexes[];
	static int columnSizes[];

	void downloadSelected(const string& aDir, bool view = false); 
	void downloadWholeSelected(const string& aDir);
	void onEnter();
	void onTab(bool shift);

	void download(SearchResult* aSR, const string& aDir, bool view);
	
	virtual void on(SearchManagerListener::SR, SearchResult* aResult) throw();

	// ClientManagerListener
	virtual void on(ClientConnected, Client* c) throw() { speak(HUB_ADDED, c); }
	virtual void on(ClientUpdated, Client* c) throw() { speak(HUB_CHANGED, c); }
	virtual void on(ClientDisconnected, Client* c) throw() { speak(HUB_REMOVED, c); }

	virtual void on(TimerManagerListener::Second, DWORD /*aTick*/) throw(){
		PostMessage(WM_SPEAKER, STATS);
	}
	
	void initHubs();
	void onHubAdded(HubInfo* info);
	void onHubChanged(HubInfo* info);
	void onHubRemoved(HubInfo* info);

	LRESULT onItemChangedHub(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	void speak(Speakers s, Client* aClient) {
		HubInfo* hubInfo = new HubInfo(aClient->getIpPort(), aClient->getName(), aClient->getOp());
		PostMessage(WM_SPEAKER, WPARAM(s), LPARAM(hubInfo)); 
	};
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)

/**
 * @file
 * $Id: SearchFrm.h,v 1.6 2004/01/06 01:52:15 trem Exp $
 */

