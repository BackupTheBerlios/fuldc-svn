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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"

#include "Resource.h"

#include "DirectoryListingFrm.h"
#include "SearchFrm.h"
#include "WinUtil.h"
#include "LineDlg.h"

#include "../client/File.h"
#include "../client/QueueManager.h"
#include "../client/StringTokenizer.h"
#include "../client/ADLSearch.h"
#include "../client/MerkleTree.h"
#include "../client/User.h"
#include "../client/ClientManager.h"

DirectoryListingFrame::FrameMap DirectoryListingFrame::frames;
int DirectoryListingFrame::columnIndexes[] = { COLUMN_FILENAME, COLUMN_TYPE, COLUMN_EXACTSIZE, COLUMN_SIZE, COLUMN_TTH };
int DirectoryListingFrame::columnSizes[] = { 300, 60, 100, 100, 200 };

static ResourceManager::Strings columnNames[] = { ResourceManager::FILE, ResourceManager::TYPE, ResourceManager::EXACT_SIZE, ResourceManager::SIZE, ResourceManager::TTH_ROOT };

DirectoryListingFrame::UserMap DirectoryListingFrame::lists;

void DirectoryListingFrame::openWindow(const tstring& aFile, const User::Ptr& aUser) {
	UserIter i = lists.find(aUser);
	if(i != lists.end()) {
		i->second->rebuild(aUser);
		i->second->loadFile(aFile);
		if(!BOOLSETTING(POPUNDER_FILELIST)) {
			i->second->MDIActivate(i->second->m_hWnd);
		}
	} else {
		DirectoryListingFrame* frame = new DirectoryListingFrame(aUser);
		if(BOOLSETTING(POPUNDER_FILELIST)) {
			WinUtil::hiddenCreateEx(frame);
		} else {
			frame->CreateEx(WinUtil::mdiClient);
		}
		frame->loadFile(aFile);
		frames.insert( FramePair( frame->m_hWnd, frame ) );
	}
}

void DirectoryListingFrame::openWindow(const User::Ptr& aUser, const string& txt) {
	UserIter i = lists.find(aUser);
	if(i != lists.end()) {
		i->second->loadXML(txt);
	} else {
		DirectoryListingFrame* frame = new DirectoryListingFrame(aUser);
		if(BOOLSETTING(POPUNDER_FILELIST)) {
			WinUtil::hiddenCreateEx(frame);
		} else {
			frame->CreateEx(WinUtil::mdiClient);
		}
		frame->loadXML(txt);
		frames.insert( FramePair( frame->m_hWnd, frame ) );
	}
}

DirectoryListingFrame::DirectoryListingFrame(const User::Ptr& aUser) :
	statusContainer(STATUSCLASSNAME, this, STATUS_MESSAGE_MAP), treeContainer(WC_TREEVIEW, this, CONTROL_MESSAGE_MAP),
		listContainer(WC_LISTVIEW, this, CONTROL_MESSAGE_MAP), historyIndex(0),
		treeRoot(NULL), skipHits(0), updating(false), dl(NULL), searching(false),
		mylist(false), loading(true)
{
	tstring tmp;

	dl = new DirectoryListing(aUser);

	lists.insert(make_pair(aUser, this));
}

void DirectoryListingFrame::loadFile(const tstring& name) {
	ctrlStatus.SetText(0, CTSTRING(LOADING_FILE_LIST));
	//don't worry about cleanup, the object will delete itself once the thread has finished it's job
	ThreadedDirectoryListing* tdl = new ThreadedDirectoryListing(this, Text::fromT(name), Util::emptyString);
	loading = true;
	tdl->start();

	tstring filename = Util::getFileName(name);
	if( Util::stricmp(filename, _T("files.xml.bz2")) == 0 )
		mylist = true;
	else if ( Util::strnicmp(filename, _T("MyList"), 6) == 0 )
		mylist = true;
}

void DirectoryListingFrame::loadXML(const string& txt) {
	ctrlStatus.SetText(0, CTSTRING(LOADING_FILE_LIST));
	//don't worry about cleanup, the object will delete itself once the thread has finished it's job
	ThreadedDirectoryListing* tdl = new ThreadedDirectoryListing(this, Util::emptyString, txt);
	loading = true;
	tdl->start();
}

LRESULT DirectoryListingFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	statusContainer.SubclassWindow(ctrlStatus.m_hWnd);

	ctrlTree.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP, WS_EX_CLIENTEDGE, IDC_DIRECTORIES);
	treeContainer.SubclassWindow(ctrlTree);
	ctrlList.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_FILES);
	listContainer.SubclassWindow(ctrlList);
	ctrlList.SetExtendedListViewStyle(LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
	
	ctrlList.SetBkColor(WinUtil::bgColor);
	ctrlList.SetTextBkColor(WinUtil::bgColor);
	ctrlList.SetTextColor(WinUtil::textColor);
	
	ctrlTree.SetBkColor(WinUtil::bgColor);
	ctrlTree.SetTextColor(WinUtil::textColor);
	
	WinUtil::splitTokens(columnIndexes, SETTING(DIRECTORYLISTINGFRAME_ORDER), COLUMN_LAST);
	WinUtil::splitTokens(columnSizes, SETTING(DIRECTORYLISTINGFRAME_WIDTHS), COLUMN_LAST);
	for(int j = 0; j < COLUMN_LAST; j++) 
	{
		int fmt = ((j == COLUMN_SIZE) || (j == COLUMN_EXACTSIZE)) ? LVCFMT_RIGHT : LVCFMT_LEFT;
		ctrlList.InsertColumn(j, CTSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
	}
	ctrlList.SetColumnOrderArray(COLUMN_LAST, columnIndexes);

	ctrlList.setSortColumn(COLUMN_FILENAME);
	ctrlList.setVisible(SETTING(DIRECTORYLISTINGFRAME_VISIBLE));
	
	ctrlTree.SetImageList(WinUtil::fileImages, TVSIL_NORMAL);
	ctrlList.SetImageList(WinUtil::fileImages, LVSIL_SMALL);

	ctrlFind.Create(ctrlStatus.m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON, 0, IDC_FIND);
	ctrlFind.SetWindowText(CTSTRING(FIND));
	ctrlFind.SetFont(WinUtil::systemFont);

	ctrlFindNext.Create(ctrlStatus.m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON, 0, IDC_NEXT);
	ctrlFindNext.SetWindowText(CTSTRING(NEXT));
	ctrlFindNext.SetFont(WinUtil::systemFont);

	ctrlMatchQueue.Create(ctrlStatus.m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON, 0, IDC_MATCH_QUEUE);
	ctrlMatchQueue.SetWindowText(CTSTRING(MATCH_QUEUE));
	ctrlMatchQueue.SetFont(WinUtil::systemFont);

	ctrlListDiff.Create(ctrlStatus.m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		BS_PUSHBUTTON, 0, IDC_FILELIST_DIFF);
	ctrlListDiff.SetWindowText(CTSTRING(FILE_LIST_DIFF));
	ctrlListDiff.SetFont(WinUtil::systemFont);

	SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	SetSplitterPanes(ctrlTree.m_hWnd, ctrlList.m_hWnd);
	m_nProportionalPos = 2500;
	
	treeRoot = ctrlTree.InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, Text::toT(dl->getUser()->getNick()).c_str(), WinUtil::getDirIconIndex(), WinUtil::getDirIconIndex(), 0, 0, (LPARAM)dl->getRoot(), NULL, TVI_SORT);

	memset(statusSizes, 0, sizeof(statusSizes));
	statusSizes[4] = WinUtil::getTextWidth(TSTRING(FILE_LIST_DIFF), m_hWnd) + 8;
	statusSizes[5] = WinUtil::getTextWidth(TSTRING(MATCH_QUEUE), m_hWnd) + 8;
	statusSizes[6] = WinUtil::getTextWidth(TSTRING(FIND), m_hWnd) + 8;
	statusSizes[7] = WinUtil::getTextWidth(TSTRING(NEXT), m_hWnd) + 8;

	ctrlStatus.SetParts(9, statusSizes);

	fileMenu.CreatePopupMenu();
	targetMenu.CreatePopupMenu();
	directoryMenu.CreatePopupMenu();
	targetDirMenu.CreatePopupMenu();
	searchMenu.CreatePopupMenu();
	copyMenu.CreatePopupMenu();
	
	for(int i = 0; i < COLUMN_LAST; ++i) {
		copyMenu.AppendMenu(MF_STRING, IDC_COPY + i, CTSTRING_I(columnNames[i]));
	}
	copyMenu.AppendMenu(MF_STRING, IDC_COPY + COLUMN_LAST, CTSTRING(PATH));
	copyMenu.AppendMenu(MF_SEPARATOR);
	copyMenu.AppendMenu(MF_STRING, IDC_COPY+1+COLUMN_LAST, CTSTRING(ALL));
			
	fileMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD, CTSTRING(DOWNLOAD));
	fileMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)targetMenu, CTSTRING(DOWNLOAD_TO));
	fileMenu.AppendMenu(MF_STRING, IDC_VIEW_AS_TEXT, CTSTRING(VIEW_AS_TEXT));
	fileMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)copyMenu, CTSTRING(COPY_TO_CLIPBOARD));
	fileMenu.AppendMenu(MF_SEPARATOR);
	fileMenu.AppendMenu(MF_STRING, IDC_SEARCH_ALTERNATES, CTSTRING(SEARCH_FOR_ALTERNATES));
	fileMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)searchMenu, CTSTRING(SEARCH_SITES));
	fileMenu.SetMenuDefaultItem(IDC_DOWNLOAD);
	
	//Set the MNS_NOTIFYBYPOS flag to receive WM_MENUCOMMAND
	MENUINFO inf;
	inf.cbSize = sizeof(MENUINFO);
	inf.fMask = MIM_STYLE | MIM_APPLYTOSUBMENUS;
	inf.dwStyle = MNS_NOTIFYBYPOS;
	fileMenu.SetMenuInfo(&inf);

	directoryMenu.AppendMenu(MF_STRING, IDC_DOWNLOADDIR, CTSTRING(DOWNLOAD));
	directoryMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)targetDirMenu, CTSTRING(DOWNLOAD_TO));
	
	setWindowTitle();

	WinUtil::SetIcon(m_hWnd, _T("Directory.ico"));
	
	bHandled = FALSE;
	return 1;
}

void DirectoryListingFrame::updateTree(DirectoryListing::Directory* aTree, HTREEITEM aParent) {
	for(DirectoryListing::Directory::Iter i = aTree->directories.begin(); i != aTree->directories.end(); ++i) {
		if(!loading) {
			throw AbortException();
		}

		tstring name;
		name = Text::toT((*i)->getName());
		int index = (*i)->getComplete() ? WinUtil::getDirIconIndex() : WinUtil::getDirMaskedIndex();
		HTREEITEM ht = ctrlTree.InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, name.c_str(), index, index, 0, 0, (LPARAM)*i, aParent, TVI_SORT);
		if((*i)->getAdls())
			ctrlTree.SetItemState(ht, TVIS_BOLD, TVIS_BOLD);
		updateTree(*i, ht);
	}
}
void DirectoryListingFrame::refreshTree(const tstring& root) {
	if(!loading) {
		throw AbortException();
	}
	
	ctrlTree.SetRedraw(FALSE);

	HTREEITEM ht = findItem(treeRoot, root);
	if(ht == NULL) {
		ht = treeRoot;
	}

	DirectoryListing::Directory* d = (DirectoryListing::Directory*)ctrlTree.GetItemData(ht);

	HTREEITEM next = NULL;
	while((next = ctrlTree.GetChildItem(ht)) != NULL) {
		ctrlTree.DeleteItem(next);
	}

	updateTree(d, ht);
	
	ctrlTree.Expand(treeRoot);

	int index = d->getComplete() ? WinUtil::getDirIconIndex() : WinUtil::getDirMaskedIndex();
	ctrlTree.SetItemImage(ht, index, index);

	ctrlTree.SelectItem(NULL);
	selectItem(root);

	ctrlTree.SetRedraw(TRUE);
}

void DirectoryListingFrame::updateStatus() {
	if(!searching && !updating && ctrlStatus.IsWindow()) {
		int cnt = ctrlList.GetSelectedCount();
		int64_t total = 0;
		if(cnt == 0) {
			cnt = ctrlList.GetItemCount ();
			total = ctrlList.forEachT(ItemInfo::TotalSize()).total;
		} else {
			total = ctrlList.forEachSelectedT(ItemInfo::TotalSize()).total;
		}

		tstring tmp1 = Text::toT(STRING(ITEMS) + ": " + Util::toString(cnt));
		tstring tmp2 = Text::toT(STRING(SIZE) + ": " + Util::formatBytes(total));
		bool u = false;

		int w = WinUtil::getTextWidth(tmp1, ctrlStatus.m_hWnd);
		if(statusSizes[0] < w) {
			statusSizes[0] = w;
			u = true;
		}
		ctrlStatus.SetText(1, tmp1.c_str());
		w = WinUtil::getTextWidth(tmp2, ctrlStatus.m_hWnd);
		if(statusSizes[1] < w) {
			statusSizes[1] = w;
			u = true;
		}
		ctrlStatus.SetText(2, tmp2.c_str());

		if(u)
			UpdateLayout(TRUE);
	}
}

void DirectoryListingFrame::initStatus() {
	files = dl->getTotalFileCount();
	size = Util::formatBytes(dl->getTotalSize());

	tstring tmp1 = Text::toT(STRING(FILES) + ": " + Util::toString(dl->getTotalFileCount(true)));
	tstring tmp2 = Text::toT(STRING(SIZE) + ": " + Util::formatBytes(dl->getTotalSize(true)));
	statusSizes[2] = WinUtil::getTextWidth(tmp1, m_hWnd);
	statusSizes[3] = WinUtil::getTextWidth(tmp2, m_hWnd);

	ctrlStatus.SetParts(9, statusSizes);
	ctrlStatus.SetText(3, tmp1.c_str());
	ctrlStatus.SetText(4, tmp2.c_str());

	UpdateLayout(FALSE);
}

LRESULT DirectoryListingFrame::onSelChangedDirectories(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMTREEVIEW* p = (NMTREEVIEW*) pnmh;

	if(p->itemNew.state & TVIS_SELECTED) {
		DirectoryListing::Directory* d = (DirectoryListing::Directory*)p->itemNew.lParam;
		changeDir(d, TRUE);
		addHistory(dl->getPath(d));
	}
	return 0;
}


void DirectoryListingFrame::addHistory(const string& name) {
	history.erase(history.begin() + historyIndex, history.end());
	while(history.size() > 25)
		history.pop_front();
	history.push_back(name);
	historyIndex = history.size();
}

void DirectoryListingFrame::changeDir(DirectoryListing::Directory* d, BOOL enableRedraw)
{
	ctrlList.SetRedraw(FALSE);
	updating = true;
	clearList();

	for(DirectoryListing::Directory::Iter i = d->directories.begin(); i != d->directories.end(); ++i) {
		ctrlList.insertItem(ctrlList.GetItemCount(), new ItemInfo(*i), (*i)->getComplete() ? WinUtil::getDirIconIndex() : WinUtil::getDirMaskedIndex());
	}
	for(DirectoryListing::File::Iter j = d->files.begin(); j != d->files.end(); ++j) {
		ItemInfo* ii = new ItemInfo(*j);
		ctrlList.insertItem(ctrlList.GetItemCount(), ii, WinUtil::getIconIndex(ii->getText(COLUMN_FILENAME)));
	}
	ctrlList.resort();
	ctrlList.SetRedraw(enableRedraw);
	updating = false;
	updateStatus();
}

void DirectoryListingFrame::up() {
	HTREEITEM t = ctrlTree.GetSelectedItem();
	if(t == NULL)
		return;
	t = ctrlTree.GetParentItem(t);
	if(t == NULL)
		return;
	ctrlTree.SelectItem(t);
}

void DirectoryListingFrame::back() {
	if(history.size() > 1 && historyIndex > 1) {
		size_t n = min(historyIndex, history.size()) - 1;
		deque<string> tmp = history;
		selectItem(Text::toT(history[n - 1]));
		historyIndex = n;
		history = tmp;
	}
}
void DirectoryListingFrame::forward() {
	if(history.size() > 1 && historyIndex < history.size()) {
		size_t n = min(historyIndex, history.size() - 1);
		deque<string> tmp = history;
		selectItem(Text::toT(history[n]));
		historyIndex = n + 1;
		history = tmp;
	}
}

LRESULT DirectoryListingFrame::onDoubleClickFiles(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*) pnmh;

	HTREEITEM t = ctrlTree.GetSelectedItem();
	if(t != NULL && item->iItem != -1) {
		ItemInfo* ii = (ItemInfo*) ctrlList.GetItemData(item->iItem);

		if(ii->type == ItemInfo::FILE) {
			try {
				dl->download(ii->file, SETTING(DOWNLOAD_DIRECTORY) + Text::fromT(ii->getText(COLUMN_FILENAME)), false, WinUtil::isShift());
			} catch(const Exception& e) {
				ctrlStatus.SetText(0, Text::toT(e.getError()).c_str());
			}
		} else {
			HTREEITEM ht = ctrlTree.GetChildItem(t);
			while(ht != NULL) {
				if((DirectoryListing::Directory*)ctrlTree.GetItemData(ht) == ii->dir) {
					ctrlTree.SelectItem(ht);
					break;
				}
				ht = ctrlTree.GetNextSiblingItem(ht);
			}
		} 
	}
	return 0;
}

LRESULT DirectoryListingFrame::onDownloadDir(WORD , WORD , HWND , BOOL& ) {
	HTREEITEM t = ctrlTree.GetSelectedItem();
	if(t != NULL) {
		DirectoryListing::Directory* dir = (DirectoryListing::Directory*)ctrlTree.GetItemData(t);
		try {
			dl->download(dir, SETTING(DOWNLOAD_DIRECTORY), WinUtil::isShift());
		} catch(const Exception& e) {
			ctrlStatus.SetText(0, Text::toT(e.getError()).c_str());
		}
	}
	return 0;
}

LRESULT DirectoryListingFrame::onDownloadDirTo(WORD , WORD , HWND , BOOL& ) {
	HTREEITEM t = ctrlTree.GetSelectedItem();
	if(t != NULL) {
		DirectoryListing::Directory* dir = (DirectoryListing::Directory*)ctrlTree.GetItemData(t);
		tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY));
		if(WinUtil::browseDirectory(target, m_hWnd)) {
			WinUtil::addLastDir(target);
			
			try {
				dl->download(dir, Text::fromT(target), WinUtil::isShift());
			} catch(const Exception& e) {
				ctrlStatus.SetText(0, Text::toT(e.getError()).c_str());
			}
		}
	}
	return 0;
}

void DirectoryListingFrame::downloadList(const tstring& aTarget, bool view /* = false */) {
	int i=-1;
	while( (i = ctrlList.GetNextItem(i, LVNI_SELECTED)) != -1) {
		ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(i);

		tstring target = aTarget.empty() ? Text::toT(SETTING(DOWNLOAD_DIRECTORY)) : aTarget;

		try {
			if(ii->type == ItemInfo::FILE) {
				if(view) {
					File::deleteFile(Text::fromT(target) + Util::validateFileName(ii->file->getName()));
				}
				dl->download(ii->file, Text::fromT(target + ii->getText(COLUMN_FILENAME)), view, WinUtil::isShift() || view);
			} else if(!view) {
				dl->download(ii->dir, Text::fromT(target), WinUtil::isShift());
			} 
		} catch(const Exception& e) {
			ctrlStatus.SetText(0, Text::toT(e.getError()).c_str());
		}
	}
}

LRESULT DirectoryListingFrame::onDownload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	downloadList(Text::toT(SETTING(DOWNLOAD_DIRECTORY)));
	return 0;
}

LRESULT DirectoryListingFrame::onDownloadTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlList.GetSelectedCount() == 1) {
		ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));

		try {
			if(ii->type == ItemInfo::FILE) {
				tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY)) + ii->getText(COLUMN_FILENAME);
				if(WinUtil::browseFile(target, m_hWnd)) {
					WinUtil::addLastDir(Util::getFilePath(target));
					dl->download(ii->file, Text::fromT(target), false, WinUtil::isShift());
				}
			} else {
				tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY));
				if(WinUtil::browseDirectory(target, m_hWnd)) {
					WinUtil::addLastDir(target);
					dl->download(ii->dir, Text::fromT(target), WinUtil::isShift());
				}
			} 
		} catch(const Exception& e) {
			ctrlStatus.SetText(0, Text::toT(e.getError()).c_str());
		}
	} else {
		tstring target = Text::toT(SETTING(DOWNLOAD_DIRECTORY));
		if(WinUtil::browseDirectory(target, m_hWnd)) {
			WinUtil::addLastDir(target);			
			downloadList(target);
		}
	}
	return 0;
}

LRESULT DirectoryListingFrame::onViewAsText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	downloadList(Text::toT(Util::getTempPath()), true);
	return 0;
}

LRESULT DirectoryListingFrame::onSearchByTTH(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ItemInfo* ii = ctrlList.getSelectedItem();
	if(ii != NULL) {
		if(ii->type == ItemInfo::DIRECTORY) {
			SearchFrame::openWindow(ii->getText(COLUMN_FILENAME), 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_DIRECTORY);
		} else {
			if(ii->file->getTTH())
				WinUtil::searchHash(ii->file->getTTH());
		}
	}
	return 0;
}

LRESULT DirectoryListingFrame::onMatchQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int x = QueueManager::getInstance()->matchListing(*dl);
	AutoArray<TCHAR> buf(STRING(MATCHED_FILES).length() + 32);
	_stprintf(buf, CTSTRING(MATCHED_FILES), x);
	ctrlStatus.SetText(0, buf);
	return 0;
}

LRESULT DirectoryListingFrame::onListDiff(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	tstring file;
	if(WinUtil::browseFile(file, m_hWnd, false, Text::toT(Util::getListPath()), _T("File Lists\0*.xml.bz2\0All Files\0*.*\0"))) {
		DirectoryListing dirList(dl->getUser());
		dirList.loadFile(Text::fromT(file));
		dl->getRoot()->filterList(dirList);
		refreshTree(Util::emptyStringT);
		initStatus();
		updateStatus();
	}
	return 0;
}

LRESULT DirectoryListingFrame::onGoToDirectory(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlList.GetSelectedCount() != 1) 
		return 0;

	tstring fullPath;
	ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));
	if(ii->type == ItemInfo::FILE) {
		if(!ii->file->getAdls())
			return 0;
		fullPath = Text::toT(dl->getPath(ii->file));
		//DirectoryListing::Directory* pd = ii->file->getParent();
		//while(pd != NULL && pd != dl->getRoot()) {
		//	fullPath = _T("\\") + Text::toT(pd->getName()) + fullPath;
		//	pd = pd->getParent();
		//}
		//fullPath.erase(0, 1);
	} else if(ii->type == ItemInfo::DIRECTORY) {
		if(!(ii->dir->getAdls() && ii->dir->getParent() != dl->getRoot()))
			return 0;
		fullPath = Text::toT(((DirectoryListing::AdlDirectory*)ii->dir)->getFullPath());
		if(fullPath[0] == _T('\\')) {
			fullPath.erase(0,1);
		}
	}

	selectItem(fullPath);
	
	return 0;
}

HTREEITEM DirectoryListingFrame::findItem(HTREEITEM ht, const tstring& name) {
	string::size_type i = name.find('\\');
	if(i == string::npos)
		return ht;
	
	for(HTREEITEM child = ctrlTree.GetChildItem(ht); child != NULL; child = ctrlTree.GetNextSiblingItem(child)) {
		DirectoryListing::Directory* d = (DirectoryListing::Directory*)ctrlTree.GetItemData(child);
		if(Text::toT(d->getName()) == name.substr(0, i)) {
			return findItem(child, name.substr(i+1));
		}
	}
	return NULL;
}

void DirectoryListingFrame::selectItem(const tstring& name) {
	HTREEITEM ht = findItem(treeRoot, name);
	if(ht != NULL) {
		ctrlTree.EnsureVisible(ht);
		ctrlTree.SelectItem(ht);
	}
}

HRESULT DirectoryListingFrame::onContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if(TRUE == fileMenu.RemoveMenu(IDC_GO_TO_DIRECTORY, MF_BYCOMMAND)) {
		fileMenu.RemoveMenu(fileMenu.GetMenuItemCount()-1, MF_BYPOSITION);
	}
	if(fileMenu.GetMenuItemCount() > 7) {
		for(int i = 0; i < 3; ++i) {
			fileMenu.RemoveMenu(fileMenu.GetMenuItemCount()-1, MF_BYPOSITION);
		}
	}
	
	if (reinterpret_cast<HWND>(wParam) == ctrlList && ctrlList.GetSelectedCount() > 0) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		
		if(pt.x == -1 && pt.y == -1) {
			WinUtil::getContextMenuPos(ctrlList, pt);
		}

		int n = 0;

		WinUtil::AppendSearchMenu(searchMenu);

		ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));

		while(targetMenu.GetMenuItemCount() > 0) {
			targetMenu.DeleteMenu(0, MF_BYPOSITION);
		}

		if(ctrlList.GetSelectedCount() == 1 && ii->type == ItemInfo::FILE) {
			//Append Favorite download dirs
			StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
			if (spl.size() > 0) {
				for(StringPairIter i = spl.begin(); i != spl.end(); i++) {
					targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_FAVORITE_DIRS + n, Text::toT(i->second).c_str());
					n++;
				}
				targetMenu.AppendMenu(MF_SEPARATOR);
			}

			n = 0;
			targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOADTO, CTSTRING(BROWSE));
			targets.clear();
			if(ii->file->getTTH() != NULL) {
				QueueManager::getInstance()->getTargetsByRoot(targets, *ii->file->getTTH());
			}

			if(targets.size() > 0) {
				targetMenu.AppendMenu(MF_SEPARATOR);
				for(StringIter i = targets.begin(); i != targets.end(); ++i) {
					targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET + (++n), Text::toT(*i).c_str());
				}
			}
			if(WinUtil::lastDirs.size() > 0) {
				targetMenu.AppendMenu(MF_SEPARATOR);
				for(TStringIter i = WinUtil::lastDirs.begin(); i != WinUtil::lastDirs.end(); ++i) {
					targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET + (++n), i->c_str());
				}
			}

			if(ii->file->getDupe()) {
				fileMenu.AppendMenu(MF_SEPARATOR);
				fileMenu.AppendMenu(MF_STRING, IDC_OPEN, CTSTRING(OPEN));
				fileMenu.AppendMenu(MF_STRING, IDC_OPEN_FOLDER, CTSTRING(OPEN_FOLDER));
			}

			if(ii->file->getAdls())	{
				fileMenu.AppendMenu(MF_SEPARATOR);
				fileMenu.AppendMenu(MF_STRING, IDC_GO_TO_DIRECTORY, CTSTRING(GO_TO_DIRECTORY));
			}

			prepareMenu(fileMenu, UserCommand::CONTEXT_FILELIST, Text::toT(dl->getUser()->getClientAddressPort()), dl->getUser()->isClientOp());
			fileMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			cleanMenu(fileMenu);
		} else {
			//Append Favorite download dirs
			StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
			if (spl.size() > 0) {
				for(StringPairIter i = spl.begin(); i != spl.end(); i++) {
					targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_FAVORITE_DIRS + n, Text::toT(i->second).c_str());
					n++;
				}
				targetMenu.AppendMenu(MF_SEPARATOR);
			}

			n = 0;
			targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOADTO, CTSTRING(BROWSE));
			if(WinUtil::lastDirs.size() > 0) {
				targetMenu.AppendMenu(MF_SEPARATOR);
				for(TStringIter i = WinUtil::lastDirs.begin(); i != WinUtil::lastDirs.end(); ++i) {
					targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET + (++n), i->c_str());
				}
			}
			//don't care about partial dupes since their folders might not be the same so we can't find
			//them in our folder structure
			if(ctrlList.GetSelectedCount() == 1 && ii->dir->getDupe() == DirectoryListing::Directory::DUPE) {
				fileMenu.AppendMenu(MF_SEPARATOR);
				fileMenu.AppendMenu(MF_STRING, IDC_OPEN, CTSTRING(OPEN));
				fileMenu.AppendMenu(MF_STRING, IDC_OPEN_FOLDER, CTSTRING(OPEN_FOLDER));
			}
			if(ii->type == ItemInfo::DIRECTORY && ii->type == ItemInfo::DIRECTORY && 
			   ii->dir->getAdls() && ii->dir->getParent() != dl->getRoot()) {
			    fileMenu.AppendMenu(MF_SEPARATOR);
				fileMenu.AppendMenu(MF_STRING, IDC_GO_TO_DIRECTORY, CTSTRING(GO_TO_DIRECTORY));
			}

			prepareMenu(fileMenu, UserCommand::CONTEXT_FILELIST, Text::toT(dl->getUser()->getClientAddressPort()), dl->getUser()->isClientOp());
			fileMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			cleanMenu(fileMenu);
		}
		
		return TRUE; 
	} else if(reinterpret_cast<HWND>(wParam) == ctrlTree && ctrlTree.GetSelectedItem() != NULL) { 
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		
		if(pt.x == -1 && pt.y == -1) {
			WinUtil::getContextMenuPos(ctrlTree, pt);
		} else {
			ctrlTree.ScreenToClient(&pt);
			UINT a = 0;
			HTREEITEM ht = ctrlTree.HitTest(pt, &a);
			if(ht != NULL && ht != ctrlTree.GetSelectedItem())
				ctrlTree.SelectItem(ht);
			ctrlTree.ClientToScreen(&pt);
		}

		// Strange, windows doesn't change the selection on right-click... (!)
		
		while(targetDirMenu.GetMenuItemCount() > 0) {
			targetDirMenu.DeleteMenu(0, MF_BYPOSITION);
		}

		int n = 0;
		//Append Favorite download dirs
		StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
		if (spl.size() > 0) {
			for(StringPairIter i = spl.begin(); i != spl.end(); i++) {
				targetDirMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS + n, Text::toT(i->second).c_str());
				n++;
			}
			targetDirMenu.AppendMenu(MF_SEPARATOR);
		}

		n = 0;
		targetDirMenu.AppendMenu(MF_STRING, IDC_DOWNLOADDIRTO, CTSTRING(BROWSE));

		if(WinUtil::lastDirs.size() > 0) {
			targetDirMenu.AppendMenu(MF_SEPARATOR);
			for(TStringIter i = WinUtil::lastDirs.begin(); i != WinUtil::lastDirs.end(); ++i) {
				targetDirMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET_DIR + (++n), i->c_str());
			}
		}
		
		directoryMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		
		return TRUE; 
	} 

	bHandled = FALSE;
	return FALSE; 
}

HRESULT DirectoryListingFrame::onXButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM /* lParam */, BOOL& /* bHandled */) {
	if(GET_XBUTTON_WPARAM(wParam) & XBUTTON1) {
		back();
		return TRUE;
	} else if(GET_XBUTTON_WPARAM(wParam) & XBUTTON2) {
		forward();
		return TRUE;
	}

	return FALSE;
}

LRESULT DirectoryListingFrame::onDownloadTarget(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int newId = wID - IDC_DOWNLOAD_TARGET - 1;
	dcassert(newId >= 0);
	
	if(ctrlList.GetSelectedCount() == 1) {
		ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));

		if(ii->type == ItemInfo::FILE) {
			if(newId < (int)targets.size()) {
				try {
					dl->download(ii->file, targets[newId], false, WinUtil::isShift());
				} catch(const Exception& e) {
					ctrlStatus.SetText(0, Text::toT(e.getError()).c_str());
				}
			} else {
				newId -= (int)targets.size();
				dcassert(newId < (int)WinUtil::lastDirs.size());
				downloadList(WinUtil::lastDirs[newId]);
			}
		} else {
			dcassert(newId < (int)WinUtil::lastDirs.size());
			downloadList(WinUtil::lastDirs[newId]);
		}
	} else if(ctrlList.GetSelectedCount() > 1) {
		dcassert(newId < (int)WinUtil::lastDirs.size());
		downloadList(WinUtil::lastDirs[newId]);
	}
	return 0;
}

LRESULT DirectoryListingFrame::onDownloadTargetDir(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int newId = wID - IDC_DOWNLOAD_TARGET_DIR - 1;
	dcassert(newId >= 0);
	
	HTREEITEM t = ctrlTree.GetSelectedItem();
	if(t != NULL) {
		DirectoryListing::Directory* dir = (DirectoryListing::Directory*)ctrlTree.GetItemData(t);
		string target = SETTING(DOWNLOAD_DIRECTORY);
		try {
			dcassert(newId < (int)WinUtil::lastDirs.size());
			dl->download(dir, Text::fromT(WinUtil::lastDirs[newId]), (GetKeyState(VK_SHIFT) & 0x8000) > 0);
		} catch(const Exception& e) {
			ctrlStatus.SetText(0, Text::toT(e.getError()).c_str());
		}
	}
	return 0;
}
LRESULT DirectoryListingFrame::onDownloadFavoriteDirs(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int newId = wID - IDC_DOWNLOAD_FAVORITE_DIRS;
	dcassert(newId >= 0);
	StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
	
	if(ctrlList.GetSelectedCount() == 1) {
		ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));

		if(ii->type == ItemInfo::FILE) {
			if(newId < (int)targets.size()) {
				try {
					dl->download(ii->file, targets[newId], false, WinUtil::isShift());
				} catch(const Exception& e) {
					ctrlStatus.SetText(0, Text::toT(e.getError()).c_str());
				}
			} else {
				newId -= (int)targets.size();
				dcassert(newId < (int)spl.size());
				downloadList(Text::toT(spl[newId].first));
			}
		} else {
			dcassert(newId < (int)spl.size());
			downloadList(Text::toT(spl[newId].first));
		}
	} else if(ctrlList.GetSelectedCount() > 1) {
		dcassert(newId < (int)spl.size());
		downloadList(Text::toT(spl[newId].first));
	}
	return 0;
}

LRESULT DirectoryListingFrame::onDownloadWholeFavoriteDirs(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int newId = wID - IDC_DOWNLOAD_WHOLE_FAVORITE_DIRS;
	dcassert(newId >= 0);
	
	HTREEITEM t = ctrlTree.GetSelectedItem();
	if(t != NULL) {
		DirectoryListing::Directory* dir = (DirectoryListing::Directory*)ctrlTree.GetItemData(t);
		string target = SETTING(DOWNLOAD_DIRECTORY);
		try {
			StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
			dcassert(newId < (int)spl.size());
			dl->download(dir, spl[newId].first, WinUtil::isShift());
		} catch(const Exception& e) {
			ctrlStatus.SetText(0, Text::toT(e.getError()).c_str());
		}
	}
	return 0;
}

LRESULT DirectoryListingFrame::onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
	if(kd->wVKey == VK_BACK) {
		up();
	} else if(kd->wVKey == VK_TAB) {
		onTab();
	} else if(kd->wVKey == VK_LEFT && WinUtil::isAlt()) {
		back();
	} else if(kd->wVKey == VK_RIGHT && WinUtil::isAlt()) {
		forward();
	} else if(kd->wVKey == VK_RETURN) {
		if(ctrlList.GetSelectedCount() == 1) {
			ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));
			if(ii->type == ItemInfo::DIRECTORY) {
				HTREEITEM ht = ctrlTree.GetChildItem(ctrlTree.GetSelectedItem());
				while(ht != NULL) {
					if((DirectoryListing::Directory*)ctrlTree.GetItemData(ht) == ii->dir) {
						ctrlTree.SelectItem(ht);
						break;
					}
					ht = ctrlTree.GetNextSiblingItem(ht);
				}
			} else {
				downloadList(Text::toT(SETTING(DOWNLOAD_DIRECTORY)));
			}
		} else {
			downloadList(Text::toT(SETTING(DOWNLOAD_DIRECTORY)));
		}
	}
	return 0;
}

void DirectoryListingFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */) {
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);

	if(ctrlStatus.IsWindow()) {
		CRect sr;
		int w[9];
		ctrlStatus.GetClientRect(sr);
		w[8] = sr.right - 16;
#define setw(x) w[x] = max(w[x+1] - statusSizes[x], 0)
		setw(7); setw(6); setw(5); setw(4); setw(3); setw(2); setw(1); setw(0);

		ctrlStatus.SetParts(9, w);

		ctrlStatus.GetRect(6, sr);

		sr.left = w[4];
		sr.right = w[5];
		ctrlListDiff.MoveWindow(sr);

		sr.left = w[5];
		sr.right = w[6];
		ctrlMatchQueue.MoveWindow(sr);

		sr.left = w[6];
		sr.right = w[7];
		ctrlFind.MoveWindow(sr);

		sr.left = w[7];
		sr.right = w[8];
		ctrlFindNext.MoveWindow(sr);
	}

	SetSplitterRect(&rect);
}

HTREEITEM DirectoryListingFrame::findFile(const StringSearch& str, HTREEITEM root,
										  int &foundFile, int &skipHits)
{
	// Check dir name for match
	DirectoryListing::Directory* dir = (DirectoryListing::Directory*)ctrlTree.GetItemData(root);
	if(str.match(dir->getName()))
	{
		if(skipHits == 0)
		{
			foundFile = -1;
			return root;
		}
		else
			skipHits--;
	}

	// Force list pane to contain files of current dir
	changeDir(dir, FALSE);

	// Check file names in list pane
	for(int i=0; i<ctrlList.GetItemCount(); i++)
	{
		ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(i);
		if(ii->type == ItemInfo::FILE)
		{
			if(str.match(ii->file->getName()))
			{
				if(skipHits == 0)
				{
					foundFile = i;
					return root;
				}
				else
					skipHits--;
			}
		}
	}

	dcdebug("looking for directories...\n");
	// Check subdirs recursively
	HTREEITEM item = ctrlTree.GetChildItem(root);
	while(item != NULL)
	{
		HTREEITEM srch = findFile(str, item, foundFile, skipHits);
		if(srch)
			return srch;
		else
			item = ctrlTree.GetNextSiblingItem(item);
	}

	return 0;
}

void DirectoryListingFrame::findFile(bool findNext)
{
	if(!findNext) {
		// Prompt for substring to find
		LineDlg dlg;
		dlg.title = TSTRING(SEARCH_FOR_FILE);
		dlg.description = TSTRING(ENTER_SEARCH_STRING);
		dlg.line = Util::emptyStringT;

		if(dlg.DoModal() != IDOK)
			return;

		findStr = Text::fromT(dlg.line);
		skipHits = 0;
	} else {
		skipHits++;
	}

	if(findStr.empty())
		return;

	// Do a search
	int foundFile = -1, skipHitsTmp = skipHits;
	HTREEITEM const oldDir = ctrlTree.GetSelectedItem();
	HTREEITEM const foundDir = findFile(StringSearch(findStr), ctrlTree.GetRootItem(), foundFile, skipHitsTmp);
	ctrlTree.SetRedraw(TRUE);

	if(foundDir) {
		// Highlight the directory tree and list if the parent dir/a matched dir was found
		if(foundFile >= 0) {
			// SelectItem won't update the list if SetRedraw was set to FALSE and then
			// to TRUE and the item selected is the same as the last one... workaround:
			if(oldDir == foundDir)
				ctrlTree.SelectItem(NULL);

			ctrlTree.SelectItem(foundDir);
		} else {
			// Got a dir; select its parent directory in the tree if there is one
			HTREEITEM parentItem = ctrlTree.GetParentItem(foundDir);
			if(parentItem) {
				// Go to parent file list
				ctrlTree.SelectItem(parentItem);

				// Locate the dir in the file list
				DirectoryListing::Directory* dir = (DirectoryListing::Directory*)ctrlTree.GetItemData(foundDir);
				
				foundFile = ctrlList.findItem(Text::toT(dir->getName()), -1, false);
			} else {
				// If no parent exists, just the dir tree item and skip the list highlighting
				ctrlTree.SelectItem(foundDir);
			}
		}

		// Remove prev. selection from file list
		if(ctrlList.GetSelectedCount() > 0) {
			for(int i=0; i<ctrlList.GetItemCount(); i++)
				ctrlList.SetItemState(i, 0, LVIS_SELECTED);
		}

		// Highlight and focus the dir/file if possible
		if(foundFile >= 0) {
			ctrlList.SetFocus();
			ctrlList.EnsureVisible(foundFile, FALSE);
			ctrlList.SetItemState(foundFile, LVIS_SELECTED | LVIS_FOCUSED, (UINT)-1);
		} else {
			ctrlTree.SetFocus();
		}
	} else {
		ctrlTree.SelectItem(oldDir);
		MessageBox(CTSTRING(NO_MATCHES), CTSTRING(SEARCH_FOR_FILE));
	}
}

void DirectoryListingFrame::runUserCommand(UserCommand& uc) {
	StringMap ucParams;
	if(!WinUtil::getUCParams(m_hWnd, uc, ucParams))
		return;
	set<User::Ptr> nicks;

	int sel = -1;
	while((sel = ctrlList.GetNextItem(sel, LVNI_SELECTED)) != -1) {
		ItemInfo* ii = (ItemInfo*)ctrlList.getItemData(sel);
		if(uc.getType() == UserCommand::TYPE_RAW_ONCE) {
			if(nicks.find(dl->getUser()) != nicks.end())
				continue;
			nicks.insert(dl->getUser());
		}
		if(!dl->getUser()->isOnline())
			return;
		ucParams["mynick"] = dl->getUser()->getClientNick();
		ucParams["mycid"] = dl->getUser()->getClientCID().toBase32();
		ucParams["tth"] = "NONE";
		if(ii->type == ItemInfo::FILE) {
			ucParams["type"] = "File";
			ucParams["file"] = dl->getPath(ii->file) + ii->file->getName();
			ucParams["filesize"] = Util::toString(ii->file->getSize());
			ucParams["filesizeshort"] = Util::formatBytes(ii->file->getSize());
			TTHValue *hash = ii->file->getTTH();
			if(hash != NULL) {
				ucParams["tth"] = hash->toBase32();
			}
		}
		else
		{
			ucParams["type"] = "Directory";
			ucParams["file"] = dl->getPath(ii->dir) + ii->dir->getName();
			ucParams["filesize"] = Util::toString(ii->dir->getTotalSize());
			ucParams["filesizeshort"] = Util::formatBytes(ii->dir->getTotalSize());
		}

		StringMap tmp = ucParams;
		User::Ptr tmpPtr = dl->getUser();
		tmpPtr->getParams(tmp);
		tmpPtr->clientEscapeParams(tmp);
		tmpPtr->sendUserCmd(Util::formatParams(uc.getCommand(), tmp, false));
	}
}

LRESULT DirectoryListingFrame::onSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	tstring searchTerm;
	ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));
	searchTerm = ii->getText(COLUMN_FILENAME);
	
	WinUtil::search(searchTerm, 0);
	return 0;
}

void DirectoryListingFrame::closeAll(){
	for(FrameIter i = frames.begin(); i != frames.end(); ++i)
		i->second->PostMessage(WM_CLOSE, 0, 0);
}

LRESULT DirectoryListingFrame::onMenuCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	HMENU menu = reinterpret_cast<HMENU>(lParam);
	UINT pos = static_cast<UINT>(wParam);

	if(searchMenu.m_hMenu == menu) {
		tstring searchTerm;
		ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));
		searchTerm = ii->getText(COLUMN_FILENAME);
		
		WinUtil::search(searchTerm, pos + 1);
	} else {
		MENUITEMINFO inf;
		inf.cbSize = sizeof(MENUITEMINFO);
		inf.fMask = MIIM_ID;
		inf.wID = 0;

		if(fileMenu.m_hMenu == menu) {
			fileMenu.GetMenuItemInfo(pos, TRUE, &inf);
		} else if(copyMenu.m_hMenu == menu) {
			ctrlList.copy(copyMenu, pos);
			return 0;
		} else if(targetMenu.m_hMenu == menu) {
			targetMenu.GetMenuItemInfo(pos, TRUE, &inf);
		} else if(targetDirMenu.m_hMenu == menu) {
			targetDirMenu.GetMenuItemInfo(pos, TRUE, &inf);
		}
		PostMessage(WM_COMMAND, inf.wID, 0);
	}
	return 0;
}

LRESULT DirectoryListingFrame::onCustomDrawList(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMLVCUSTOMDRAW* plvcd = reinterpret_cast<NMLVCUSTOMDRAW*>( pnmh );

	if( CDDS_PREPAINT == plvcd->nmcd.dwDrawStage )
		return CDRF_NOTIFYITEMDRAW;

	if( CDDS_ITEMPREPAINT == plvcd->nmcd.dwDrawStage ) {
		ItemInfo *ii = reinterpret_cast<ItemInfo*>(plvcd->nmcd.lItemlParam);

		if(!mylist) {
			//check if the file or dir is a dupe, then use the dupesetting color
			if( ( ii->type == ItemInfo::FILE && ii->file->getDupe() ) || 
				( ii->type == ItemInfo::DIRECTORY && ii->dir->getDupe() == DirectoryListing::Directory::DUPE )
				) {
				plvcd->clrTextBk = SETTING(DUPE_COLOR);

			//if it's a partial dupe, try to use some simple blending to indicate that
			//a dupe exists somewhere down the directory tree.
			} else if(ii->type == ItemInfo::DIRECTORY && ii->dir->getDupe() == DirectoryListing::Directory::PARTIAL_DUPE) {
				BYTE r, b, g;
				//cache these to avoid unnecessary calls.
				DWORD dupe = SETTING(DUPE_COLOR);
				DWORD bg = SETTING(BACKGROUND_COLOR);

				r = static_cast<BYTE>(( static_cast<DWORD>(GetRValue(dupe)) + static_cast<DWORD>(GetRValue(bg)) ) / 2);
				g = static_cast<BYTE>(( static_cast<DWORD>(GetGValue(dupe)) + static_cast<DWORD>(GetGValue(bg)) ) / 2);
				b = static_cast<BYTE>(( static_cast<DWORD>(GetBValue(dupe)) + static_cast<DWORD>(GetBValue(bg)) ) / 2);
				plvcd->clrTextBk = RGB(r, g, b);
			}
		}

	}

	return CDRF_DODEFAULT;
}
LRESULT DirectoryListingFrame::onCustomDrawTree(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMLVCUSTOMDRAW* plvcd = reinterpret_cast<NMLVCUSTOMDRAW*>( pnmh );

	if( CDDS_PREPAINT == plvcd->nmcd.dwDrawStage )
		return CDRF_NOTIFYITEMDRAW;

	if( (CDDS_ITEMPREPAINT == plvcd->nmcd.dwDrawStage) && ((plvcd->nmcd.uItemState & CDIS_SELECTED) == 0)) {
		DirectoryListing::Directory* dir = reinterpret_cast<DirectoryListing::Directory*>(plvcd->nmcd.lItemlParam);

		if(!mylist) {
			//check if the dir is a dupe, then use the dupesetting color
			if( dir->getDupe() == DirectoryListing::Directory::DUPE ) {
				plvcd->clrTextBk = SETTING(DUPE_COLOR);

			//if it's a partial dupe, try to use some simple blending to indicate that
			//a dupe exists somewhere down the directory tree.
			} else if(dir->getDupe() == DirectoryListing::Directory::PARTIAL_DUPE) {
				BYTE r, b, g;
				//cache these to avoid unnecessary calls.
				DWORD dupe = SETTING(DUPE_COLOR);
				DWORD bg = SETTING(BACKGROUND_COLOR);

				r = static_cast<BYTE>(( static_cast<DWORD>(GetRValue(dupe)) + static_cast<DWORD>(GetRValue(bg)) ) / 2);
				g = static_cast<BYTE>(( static_cast<DWORD>(GetGValue(dupe)) + static_cast<DWORD>(GetGValue(bg)) ) / 2);
				b = static_cast<BYTE>(( static_cast<DWORD>(GetBValue(dupe)) + static_cast<DWORD>(GetBValue(bg)) ) / 2);
				plvcd->clrTextBk = RGB(r, g, b);
			}
		}

	}

	return CDRF_DODEFAULT;
}

LRESULT DirectoryListingFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	switch(wParam) {
		case FINISHED:
			loading = false;
			initStatus();
			ctrlStatus.SetText(0, CTSTRING(LOADED_FILE_LIST));
			//notify the user that we've loaded the list
			setDirty();
			break;
		case ABORTED:
			loading = false;
			PostMessage(WM_CLOSE, 0, 0);
			break;
		default: dcassert(0); break;
	}
	return 0;
}

LRESULT DirectoryListingFrame::onOpenDupe(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ItemInfo* ii = ctrlList.getSelectedItem();

	try {
		tstring path;
		
		if(ii->type == ItemInfo::FILE) {
			if(ii->file->getTTH()) {
				path = Text::toT(ShareManager::getInstance()->getPhysicalPath(*ii->file->getTTH()));
			}
		} else {
			if(ii->dir->getFileCount() > 0) {
				for(DirectoryListing::File::Iter i = ii->dir->files.begin(); i != ii->dir->files.end(); ++i) {
					if((*i)->getDupe() && (*i)->getTTH())
						break;
				}
				if(i != ii->dir->files.end()) {
					path = Text::toT(ShareManager::getInstance()->getPhysicalPath(*((*i)->getTTH())));
					wstring::size_type end = path.find_last_of(_T("\\"));
					if(end != wstring::npos) {
						path = path.substr(0, end);
					}
				}
			}
		}

		if(wID == IDC_OPEN) {
			WinUtil::openFile(path);
		} else {
			WinUtil::openFolder(path);
		}
	} catch(const ShareException& se) {
		error = Text::toT(se.getError());
	}

	return 0;
}
/**
 * @file
 * $Id: DirectoryListingFrm.cpp,v 1.11 2004/02/21 15:14:43 trem Exp $
 */
