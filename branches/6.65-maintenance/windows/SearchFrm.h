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

#if !defined(AFX_SEARCHFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_)
#define AFX_SEARCHFRM_H__A7078724_FD85_4F39_8463_5A08A5F45E33__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "FlatTabCtrl.h"
#include "TypedListViewCtrl.h"
#include "WinUtil.h"
#include "FulComboBox.h"

#include "../client/pme.h"

#include "../client/Client.h"
#include "../client/SearchManager.h"
#include "../client/CriticalSection.h"
#include "../client/ClientManagerListener.h"
#include "../client/TimerManager.h"
#include "../client/ShareManager.h"

#include "UCHandler.h"

#define SEARCH_MESSAGE_MAP 6		// This could be any number, really...
#define SHOWUI_MESSAGE_MAP 7

class SearchFrame : public MDITabChildWindowImpl<SearchFrame>,
	private SearchManagerListener, private ClientManagerListener, 
	public UCHandler<SearchFrame>, public UserInfoBaseHandler<SearchFrame>,
	private TimerManagerListener
{
public:
	static void openWindow(const tstring& str = Util::emptyStringW, LONGLONG size = 0, SearchManager::SizeModes mode = SearchManager::SIZE_ATLEAST, SearchManager::TypeModes type = SearchManager::TYPE_ANY);
	static void closeAll();

	DECLARE_FRAME_WND_CLASS_EX(_T("SearchFrame"), IDR_SEARCH, 0, COLOR_3DFACE)

	typedef MDITabChildWindowImpl<SearchFrame> baseClass;
	typedef UCHandler<SearchFrame> ucBase;
	typedef UserInfoBaseHandler<SearchFrame> uicBase;

	BEGIN_MSG_MAP(SearchFrame)
		NOTIFY_HANDLER(IDC_RESULTS, LVN_GETDISPINFO, ctrlResults.onGetDispInfo)
		NOTIFY_HANDLER(IDC_RESULTS, LVN_COLUMNCLICK, ctrlResults.onColumnClick)
		NOTIFY_HANDLER(IDC_RESULTS, NM_CUSTOMDRAW, onCustomDraw)
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
		MESSAGE_HANDLER(WM_TIMER, onTimer)
		//crap seems like the first message is sent to the parent
		//so lets direct it where it should
		MESSAGE_HANDLER(WM_DRAWITEM, ctrlFiletype.onDrawItem)
		MESSAGE_HANDLER(WM_MEASUREITEM, ctrlFiletype.onMeasureItem)
		COMMAND_ID_HANDLER(IDC_DOWNLOAD, onDownload)
		COMMAND_ID_HANDLER(IDC_DOWNLOADTO, onDownloadTo)
		COMMAND_ID_HANDLER(IDC_DOWNLOADDIR, onDownloadWhole)
		COMMAND_ID_HANDLER(IDC_DOWNLOADDIRTO, onDownloadWholeTo)
		COMMAND_ID_HANDLER(IDC_VIEW_AS_TEXT, onViewAsText)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDC_SEARCH, onSearch)
		COMMAND_ID_HANDLER(IDC_FREESLOTS, onFreeSlots)
		COMMAND_ID_HANDLER(IDC_ONLYTTH, onTTH)
		COMMAND_ID_HANDLER(IDC_GETLIST, onGetList)
		COMMAND_ID_HANDLER(IDC_BROWSELIST, onBrowseList)
		COMMAND_RANGE_HANDLER(IDC_DOWNLOAD_TARGET, IDC_DOWNLOAD_TARGET + downloadPaths.size() + targets.size() + WinUtil::lastDirs.size(), onDownloadTarget)
		COMMAND_RANGE_HANDLER(IDC_DOWNLOAD_WHOLE_TARGET, IDC_DOWNLOAD_WHOLE_TARGET + downloadPaths.size() + WinUtil::lastDirs.size(), onDownloadWholeTarget)
		COMMAND_RANGE_HANDLER(IDC_COPY, IDC_COPY+COLUMN_LAST+2, onCopy)
		COMMAND_ID_HANDLER(IDC_SEARCH_ALTERNATES, onSearchByTTH)
		COMMAND_ID_HANDLER(IDC_PURGE, onPurge)
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
	searchBoxContainer(WC_COMBOBOX, this, SEARCH_MESSAGE_MAP),
		searchContainer(WC_EDIT, this, SEARCH_MESSAGE_MAP), 
		purgeContainer(WC_EDIT, this, SEARCH_MESSAGE_MAP), 
		sizeContainer(WC_EDIT, this, SEARCH_MESSAGE_MAP), 
		modeContainer(WC_COMBOBOX, this, SEARCH_MESSAGE_MAP),
		sizeModeContainer(WC_COMBOBOX, this, SEARCH_MESSAGE_MAP),
		fileTypeContainer(WC_COMBOBOX, this, SEARCH_MESSAGE_MAP),
		showUIContainer(WC_COMBOBOX, this, SHOWUI_MESSAGE_MAP),
		slotsContainer(WC_COMBOBOX, this, SEARCH_MESSAGE_MAP),
		doSearchContainer(WC_COMBOBOX, this, SEARCH_MESSAGE_MAP),
		resultsContainer(WC_LISTVIEW, this, SEARCH_MESSAGE_MAP),
		hubsContainer(WC_LISTVIEW, this, SEARCH_MESSAGE_MAP),
		filterBoxContainer(WC_COMBOBOX, this, SEARCH_MESSAGE_MAP),
		filterContainer(WC_EDIT, this, SEARCH_MESSAGE_MAP),
		tthContainer(WC_COMBOBOX, this, SEARCH_MESSAGE_MAP),
		initialSize(0), initialMode(SearchManager::SIZE_ATLEAST), initialType(SearchManager::TYPE_ANY),
		showUI(true), onlyFree(false), closed(false), isHash(false), useRegExp(false), results(0), filtered(0),
		onlyTTH(BOOLSETTING(SEARCH_ONLY_TTH)), timerID(0)
	{	
		SearchManager::getInstance()->addListener(this);
		downloadPaths = SettingsManager::getInstance()->getDownloadPaths();
	}

	virtual ~SearchFrame() {
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
	LRESULT onTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onPurge(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onGetList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onBrowseList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	
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
		ctrlResults.forEachSelectedT(SearchInfo::Download(Text::toT(SETTING(DOWNLOAD_DIRECTORY))));
		return 0;
	}

	LRESULT onViewAsText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		ctrlResults.forEachSelected(&SearchInfo::view);
		return 0;
	}

	LRESULT onDownloadWhole(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		ctrlResults.forEachSelectedT(SearchInfo::DownloadWhole(Text::toT(SETTING(DOWNLOAD_DIRECTORY))));
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

	LRESULT onTTH(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		onlyTTH = (ctrlTTH.GetCheck() == BST_CHECKED);
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

	void SearchFrame::setInitial(const tstring& str, LONGLONG size, SearchManager::SizeModes mode, SearchManager::TypeModes type) {
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

			if( sr->getTTH() )
				dupe = ShareManager::getInstance()->isTTHShared(*sr->getTTH());
			else
				dupe = false;
		};
		~SearchInfo() { 
			sr->decRef(); 
		};

		void getList();
		void browseList();

		void view();
		struct Download {
			Download(const tstring& aTarget) : tgt(aTarget) { };
			void operator()(SearchInfo* si);
			const tstring& tgt;
		};
		struct DownloadWhole {
			DownloadWhole(const tstring& aTarget) : tgt(aTarget) { };
			void operator()(SearchInfo* si);
			const tstring& tgt;
		};
		struct DownloadTarget {
			DownloadTarget(const tstring& aTarget) : tgt(aTarget) { };
			void operator()(SearchInfo* si);
			const tstring& tgt;
		};
		struct CheckSize {
			CheckSize() : size(-1), op(true), oneHub(true), hasTTH(false), firstTTH(true) { };
			void operator()(SearchInfo* si);
			tstring ext;
			int64_t size;
			bool oneHub;
			tstring hub;
			bool op;
			bool hasTTH;
			bool firstTTH;
			tstring tth;
		};

		const tstring& getText(int col) const {
			switch(col) {
				case COLUMN_NICK: return nick;
				case COLUMN_FILENAME: return fileName;
				case COLUMN_TYPE: return type;
				case COLUMN_SIZE: return size;
				case COLUMN_PATH: return path;
				case COLUMN_SLOTS: return slots;
				case COLUMN_CONNECTION: return connection;
				case COLUMN_HUB: return hubName;
				case COLUMN_EXACT_SIZE: return exactSize;
				case COLUMN_IP: return ip;
				case COLUMN_TTH: return tth;
				default: return Util::emptyStringT;
			}
		}

		static int compareItems(SearchInfo* a, SearchInfo* b, int col) {

			switch(col) {
				case COLUMN_FILENAME: 
					if(a->sr->getType() == b->sr->getType())
						return lstrcmpi(a->fileName.c_str(), b->fileName.c_str());
					else 
						return ( a->sr->getType() == SearchResult::TYPE_DIRECTORY ) ? -1 : 1;
				case COLUMN_TYPE: 
					if(a->sr->getType() == b->sr->getType())
						return lstrcmpi(a->type.c_str(), b->type.c_str());
					else
						return(a->sr->getType() == SearchResult::TYPE_DIRECTORY) ? -1 : 1;
				case COLUMN_SLOTS: 
					if(a->sr->getFreeSlots() == b->sr->getFreeSlots())
						return compare(a->sr->getSlots(), b->sr->getSlots());
					else
						return compare(a->sr->getFreeSlots(), b->sr->getFreeSlots());
				case COLUMN_SIZE:
				case COLUMN_EXACT_SIZE: return compare(a->sr->getSize(), b->sr->getSize());
				default: return lstrcmpi(a->getText(col).c_str(), b->getText(col).c_str());
			}
		}

		void update() { 
			if(sr->getType() == SearchResult::TYPE_FILE) {
				if(sr->getFile().rfind(_T('\\')) == tstring::npos) {
					fileName = Text::toT(sr->getUtf8() ? sr->getFile() : Text::acpToUtf8(sr->getFile()));
				} else {
					fileName = Text::toT(Util::getFileName(sr->getUtf8() ? sr->getFile() : Text::acpToUtf8(sr->getFile())));
					path = Text::toT(Util::getFilePath(sr->getUtf8() ? sr->getFile() : Text::acpToUtf8(sr->getFile())));
				}

				type = Text::toT(Util::getFileExt(Text::fromT(fileName)));
				if(!type.empty() && type[0] == _T('.'))
					type.erase(0, 1);
				size = Text::toT(Util::formatBytes(sr->getSize()));
				exactSize = Util::formatExactSize(sr->getSize());
			} else {
				fileName = Text::toT(sr->getUtf8() ? sr->getFileName() : Text::acpToUtf8(sr->getFileName()));
				path = Text::toT(sr->getUtf8() ? sr->getFile() : Text::acpToUtf8(sr->getFile()));
				type = TSTRING(DIRECTORY);
			}
			nick = Text::toT(sr->getUser()->getNick());
			connection = Text::toT(sr->getUser()->getConnection());
			hubName = Text::toT(sr->getHubName());
			slots = Text::toT(sr->getSlotString());
			ip = Text::toT(sr->getIP());
			if(sr->getTTH() != NULL)
				setTTH(Text::toT(sr->getTTH()->toBase32()));
		}

		GETSET(tstring, nick, Nick);
		GETSET(tstring, connection, Connection)
		GETSET(tstring, fileName, FileName);
		GETSET(tstring, path, Path);
		GETSET(tstring, type, Type);
		GETSET(tstring, hubName, HubName);
		GETSET(tstring, size, Size);
		GETSET(tstring, slots, Slots);
		GETSET(tstring, exactSize, ExactSize);
		GETSET(tstring, ip, IP);
		GETSET(tstring, tth, TTH);
		
		bool isDupe() const { return dupe; }
	private:
		bool dupe;
	};

	struct HubInfo : public FastAlloc<HubInfo> {
		HubInfo(const tstring& aIpPort, const tstring& aName, bool aOp) : ipPort(aIpPort),
			name(aName), op(aOp) { };

		const tstring& getText(int col) const {
			return (col == 0) ? name : Util::emptyStringT;
		}
		static int compareItems(HubInfo* a, HubInfo* b, int col) {
			return (col == 0) ? lstrcmpi(a->name.c_str(), b->name.c_str()) : 0;
		}
		tstring ipPort;
		tstring name;
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

	tstring initialString;
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
	CFulComboBox ctrlFiletype;
	CButton ctrlDoSearch;
	CButton ctrlPurge;
	
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
	CContainedWindow tthContainer;
	CContainedWindow purgeContainer;
	CContainedWindow filterContainer;
	CContainedWindow filterBoxContainer;

	CStatic searchLabel, sizeLabel, optionLabel, typeLabel, hubsLabel, filterLabel;
	CButton ctrlSlots, ctrlShowUI, ctrlTTH;
	bool showUI;

	TypedListViewCtrl<SearchInfo, IDC_RESULTS> ctrlResults;
	TypedListViewCtrl<HubInfo, IDC_HUB> ctrlHubs;

	CMenu resultsMenu;
	CMenu targetMenu;
	CMenu targetDirMenu;
	CMenu copyMenu;
	
	TStringList search;
	StringList targets;
	StringList wholeTargets;
	StringPairList downloadPaths;
	TStringList filterList;

	PME filterRegExp;
	bool useRegExp;

	/** Parameter map for user commands */
	StringMap ucParams;

	bool onlyFree;
	bool isHash;
	bool onlyTTH;

	int results;
	int filtered;

	CriticalSection cs;

	static TStringList lastSearches;
	static TStringList lastFilters;

	DWORD lastSearch;
	bool closed;

	static int columnIndexes[];
	static int columnSizes[];

	// Timer ID, needed to turn off timer
	UINT timerID;

	typedef map<HWND, SearchFrame*> FrameMap;
	typedef FrameMap::iterator FrameIter;
	typedef pair<HWND, SearchFrame*> FramePair;

	static FrameMap frames;

	void downloadSelected(const tstring& aDir, bool view = false); 
	void downloadWholeSelected(const tstring& aDir);
	void onEnter();
	void onTab(bool shift);

	void download(SearchResult* aSR, const tstring& aDir, bool view);
	
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
		HubInfo* hubInfo = new HubInfo(Text::toT(aClient->getIpPort()), Text::toT(aClient->getName()), aClient->getOp());
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
