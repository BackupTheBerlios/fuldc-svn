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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"

#include "../client/File.h"
#include "../client/QueueManager.h"
#include "../client/StringTokenizer.h"

#include "Resource.h"

#include "DirectoryListingFrm.h"
#include "WinUtil.h"
#include "LineDlg.h"
#include "stack"
#include "../client/MerkleTree.h"

DirectoryListingFrame::FrameMap DirectoryListingFrame::frames;
void DirectoryListingFrame::openWindow(const tstring& aFile, const User::Ptr& aUser, const tstring& start) {
	DirectoryListingFrame* frame = new DirectoryListingFrame(aFile, aUser, start);

	frames.insert( FramePair(aUser->getNick(), frame) );
	
	if(BOOLSETTING(POPUNDER_FILELIST))
		WinUtil::hiddenCreateEx(frame);
	else
		frame->CreateEx(WinUtil::mdiClient);
		
}

DirectoryListingFrame::DirectoryListingFrame(const tstring& aFile, const User::Ptr& aUser, const tstring& s) :
	statusContainer(STATUSCLASSNAME, this, STATUS_MESSAGE_MAP),
	treeRoot(NULL), skipHits(0), updating(false), dl(NULL), searching(false), start(s)
{
	tstring tmp;
	if(aFile.size() < 4) {
		error = WinUtil::toT(aUser->getFullNick() + ": " + STRING(UNSUPPORTED_FILELIST_FORMAT));
		return;
	}

	dl = new DirectoryListing(aUser);
	try {
		dl->loadFile(WinUtil::fromT(aFile), true);
	} catch(const Exception& e) {
		error = WinUtil::toT(aUser->getFullNick() + ": " + e.getError());
	}

	StringTokenizer<tstring> token(SETTING(DOWNLOAD_TO_PATHS), _T("|"));
	downloadPaths = token.getTokens();
}

LRESULT DirectoryListingFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	statusContainer.SubclassWindow(ctrlStatus.m_hWnd);

	ctrlTree.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP, WS_EX_CLIENTEDGE, IDC_DIRECTORIES);
	ctrlList.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_FILES);
	ctrlList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
	
	ctrlList.SetBkColor(WinUtil::bgColor);
	ctrlList.SetTextBkColor(WinUtil::bgColor);
	ctrlList.SetTextColor(WinUtil::textColor);
	
	ctrlTree.SetBkColor(WinUtil::bgColor);
	ctrlTree.SetTextColor(WinUtil::textColor);
	
	ctrlList.insertColumn(COLUMN_FILENAME, CTSTRING(FILENAME), LVCFMT_LEFT, 300, COLUMN_FILENAME);
	ctrlList.insertColumn(COLUMN_TYPE, CTSTRING(FILE_TYPE), LVCFMT_LEFT, 60, COLUMN_TYPE);
	ctrlList.insertColumn(COLUMN_SIZE, CTSTRING(SIZE), LVCFMT_RIGHT, 100, COLUMN_SIZE);
	ctrlList.insertColumn(COLUMN_TTH, CTSTRING(TTH_ROOT), LVCFMT_LEFT, 200, COLUMN_TTH);

	ctrlList.setSortColumn(COLUMN_FILENAME);
	
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

	SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	SetSplitterPanes(ctrlTree.m_hWnd, ctrlList.m_hWnd);
	m_nProportionalPos = 2500;
	
	if(dl != NULL)
		treeRoot = ctrlTree.InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, WinUtil::toT(dl->getUser()->getNick()).c_str(), WinUtil::getDirIconIndex(), WinUtil::getDirIconIndex(), 0, 0, (LPARAM)dl->getRoot(), NULL, TVI_SORT);;

	updateTree(dl->getRoot(), treeRoot);
	files = dl->getTotalFileCount();
	size = Util::formatBytes(dl->getTotalSize());

	memset(statusSizes, 0, sizeof(statusSizes));
	tstring tmp1 = WinUtil::toT(STRING(FILES) + ": " + Util::toString(dl->getTotalFileCount(true)));
	tstring tmp2 = WinUtil::toT(STRING(SIZE) + ": " + Util::formatBytes(dl->getTotalSize(true)));
	statusSizes[2] = WinUtil::getTextWidth(tmp1, m_hWnd);
	statusSizes[3] = WinUtil::getTextWidth(tmp2, m_hWnd);
	statusSizes[4] = WinUtil::getTextWidth(TSTRING(MATCH_QUEUE), m_hWnd) + 8;
	statusSizes[5] = WinUtil::getTextWidth(TSTRING(FIND), m_hWnd) + 8;
	statusSizes[6] = WinUtil::getTextWidth(TSTRING(NEXT), m_hWnd) + 8;

	ctrlStatus.SetParts(8, statusSizes);
	ctrlStatus.SetText(3, tmp1.c_str());
	ctrlStatus.SetText(4, tmp2.c_str());

	if(!start.empty()) {
		StringTokenizer<tstring> tok(start, _T('\\'));
		TStringIter i = tok.getTokens().begin();
		GoToDirectory(treeRoot, i, tok.getTokens().end());
	} else {
		ctrlTree.SelectItem(treeRoot);
	}
	
	fileMenu.CreatePopupMenu();
	targetMenu.CreatePopupMenu();
	directoryMenu.CreatePopupMenu();
	targetDirMenu.CreatePopupMenu();
	copyMenu.CreatePopupMenu();
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_FILENAME, CTSTRING(FILENAME));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_PATH, CTSTRING(PATH));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_SIZE, CTSTRING(SIZE));
	copyMenu.AppendMenu(MF_STRING, IDC_COPY_TTH, CTSTRING(TTH_ROOT));

	searchMenu.CreatePopupMenu();
		
	fileMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD, CTSTRING(DOWNLOAD));
	fileMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)targetMenu, CTSTRING(DOWNLOAD_TO));
	fileMenu.AppendMenu(MF_STRING, IDC_VIEW_AS_TEXT, CTSTRING(VIEW_AS_TEXT));
	fileMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)copyMenu, CTSTRING(COPY_TO_CLIPBOARD));
	fileMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
	fileMenu.AppendMenu(MF_STRING, IDC_SEARCH, CTSTRING(SEARCH));
	fileMenu.AppendMenu(MF_STRING, IDC_SEARCH_BY_TTH, CTSTRING(SEARCH_BY_TTH));
	fileMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)searchMenu, CTSTRING(SEARCH_SITES));
	fileMenu.SetMenuDefaultItem(IDC_DOWNLOAD);
	
	//Set the MNS_NOTIFYBYPOS flag to receive WM_MENUCOMMAND
	MENUINFO inf;
	inf.cbSize = sizeof(MENUINFO);
	inf.fMask = MIM_STYLE | MIM_APPLYTOSUBMENUS;
	inf.dwStyle = MNS_NOTIFYBYPOS;
	fileMenu.SetMenuInfo(&inf);

	directoryMenu.AppendMenu(MF_STRING, IDC_DOWNLOADDIR, CTSTRING(DOWNLOAD));
	directoryMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)targetDirMenu, CTSTRING(DOWNLOAD_TO));
	
	setWindowTitle();

	WinUtil::SetIcon(m_hWnd, _T("Directory.ico"));
	
	bHandled = FALSE;
	return 1;
}

void DirectoryListingFrame::updateTree(DirectoryListing::Directory* aTree, HTREEITEM aParent) {
	for(DirectoryListing::Directory::Iter i = aTree->directories.begin(); i != aTree->directories.end(); ++i) {
		tstring name;
		if(dl->getUtf8()) {
			name = WinUtil::toT((*i)->getName());
		} else {
			string tmp = (*i)->getName();
			Util::toUtf8(tmp);
			name = WinUtil::toT(tmp);
		}
		HTREEITEM ht = ctrlTree.InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, name.c_str(), WinUtil::getDirIconIndex(), WinUtil::getDirIconIndex(), 0, 0, (LPARAM)*i, aParent, TVI_SORT);;
		if((*i)->getAdls())
			ctrlTree.SetItemState(ht, TVIS_BOLD, TVIS_BOLD);
		updateTree(*i, ht);
	}
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

		tstring tmp1 = WinUtil::toT(STRING(ITEMS) + ": " + Util::toString(cnt));
		tstring tmp2 = WinUtil::toT(STRING(SIZE) + ": " + Util::formatBytes(total));
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

LRESULT DirectoryListingFrame::onSelChangedDirectories(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMTREEVIEW* p = (NMTREEVIEW*) pnmh;

	if(p->itemNew.state & TVIS_SELECTED) {
		DirectoryListing::Directory* d = (DirectoryListing::Directory*)p->itemNew.lParam;
		changeDir(d, TRUE);
	}
	return 0;
}


void DirectoryListingFrame::changeDir(DirectoryListing::Directory* d, BOOL enableRedraw)
{
	ctrlList.SetRedraw(FALSE);
	updating = true;
	clearList();

	for(DirectoryListing::Directory::Iter i = d->directories.begin(); i != d->directories.end(); ++i) {
		ctrlList.insertItem(ctrlList.GetItemCount(), new ItemInfo(*i, dl->getUtf8()), WinUtil::getDirIconIndex());
	}
	for(DirectoryListing::File::Iter j = d->files.begin(); j != d->files.end(); ++j) {
		ItemInfo* ii = new ItemInfo(*j, dl->getUtf8());
		ctrlList.insertItem(ctrlList.GetItemCount(), ii, WinUtil::getIconIndex(ii->getText(COLUMN_FILENAME)));
	}
	ctrlList.resort();
	ctrlList.SetRedraw(enableRedraw);
	updating = false;
	updateStatus();
}

LRESULT DirectoryListingFrame::onDoubleClickFiles(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*) pnmh;

	HTREEITEM t = ctrlTree.GetSelectedItem();
	if(t != NULL && item->iItem != -1) {
		ItemInfo* ii = (ItemInfo*) ctrlList.GetItemData(item->iItem);

		if(ii->type == ItemInfo::FILE) {
			try {
				dl->download(ii->file, SETTING(DOWNLOAD_DIRECTORY) + WinUtil::fromT(ii->getText(COLUMN_FILENAME)));
			} catch(const Exception& e) {
				ctrlStatus.SetText(0, WinUtil::toT(e.getError()).c_str());
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
			dl->download(dir, SETTING(DOWNLOAD_DIRECTORY));
		} catch(const Exception& e) {
			ctrlStatus.SetText(0, WinUtil::toT(e.getError()).c_str());
		}
	}
	return 0;
}

LRESULT DirectoryListingFrame::onDownloadDirTo(WORD , WORD , HWND , BOOL& ) {
	HTREEITEM t = ctrlTree.GetSelectedItem();
	if(t != NULL) {
		DirectoryListing::Directory* dir = (DirectoryListing::Directory*)ctrlTree.GetItemData(t);
		tstring target = WinUtil::toT(SETTING(DOWNLOAD_DIRECTORY));
		if(WinUtil::browseDirectory(target, m_hWnd)) {
			WinUtil::addLastDir(target);
			
			try {
				dl->download(dir, WinUtil::fromT(target));
			} catch(const Exception& e) {
				ctrlStatus.SetText(0, WinUtil::toT(e.getError()).c_str());
			}
		}
	}
	return 0;
}

void DirectoryListingFrame::downloadList(const tstring& aTarget, bool view /* = false */) {
	int i=-1;
	while( (i = ctrlList.GetNextItem(i, LVNI_SELECTED)) != -1) {
		ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(i);

		tstring target = aTarget.empty() ? WinUtil::toT(SETTING(DOWNLOAD_DIRECTORY)) : aTarget;

		try {
			if(ii->type == ItemInfo::FILE) {
				if(view) {
					File::deleteFile(WinUtil::fromT(target) + Util::validateFileName(ii->file->getName()));
				}
				dl->download(ii->file, WinUtil::fromT(target + ii->getText(COLUMN_FILENAME)), view);
			} else if(!view) {
				dl->download(ii->dir, WinUtil::fromT(target));
			} 
		} catch(const Exception& e) {
			ctrlStatus.SetText(0, WinUtil::toT(e.getError()).c_str());
		}
	}
}

LRESULT DirectoryListingFrame::onDownload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	downloadList(WinUtil::toT(SETTING(DOWNLOAD_DIRECTORY)));
	return 0;
}

LRESULT DirectoryListingFrame::onDownloadTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlList.GetSelectedCount() == 1) {
		ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));

		try {
			if(ii->type == ItemInfo::FILE) {
				tstring target = WinUtil::toT(SETTING(DOWNLOAD_DIRECTORY)) + ii->getText(COLUMN_FILENAME);
				if(WinUtil::browseFile(target, m_hWnd)) {
					WinUtil::addLastDir(Util::getFilePath(target));
					dl->download(ii->file, WinUtil::fromT(target));
				}
			} else {
				tstring target = WinUtil::toT(SETTING(DOWNLOAD_DIRECTORY));
				if(WinUtil::browseDirectory(target, m_hWnd)) {
					WinUtil::addLastDir(target);
					dl->download(ii->dir, WinUtil::fromT(target));
				}
			} 
		} catch(const Exception& e) {
			ctrlStatus.SetText(0, WinUtil::toT(e.getError()).c_str());
		}
	} else {
		tstring target = WinUtil::toT(SETTING(DOWNLOAD_DIRECTORY));
		if(WinUtil::browseDirectory(target, m_hWnd)) {
			WinUtil::addLastDir(target);			
			downloadList(target);
		}
	}
	return 0;
}

LRESULT DirectoryListingFrame::onViewAsText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	downloadList(WinUtil::toT(Util::getTempPath()), true);
	return 0;
}

LRESULT DirectoryListingFrame::onCopyTTH(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ItemInfo* ii = ctrlList.getSelectedItem();
	if(ii != NULL)
		WinUtil::copyToClipboard(ii->getText(COLUMN_TTH));

	return 0;
}

LRESULT DirectoryListingFrame::onSearchByTTH(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ItemInfo* ii = ctrlList.getSelectedItem();
	if(ii != NULL) {
		TTHValue tmp(WinUtil::fromT(ii->getText(COLUMN_TTH)));
		WinUtil::searchHash(&tmp);
	}
	return 0;
}

LRESULT DirectoryListingFrame::onMatchQueue(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int x = QueueManager::getInstance()->matchListing(dl);
	TCHAR* buf = new TCHAR[STRING(MATCHED_FILES).length() + 32];
	_stprintf(buf, CTSTRING(MATCHED_FILES), x);
	ctrlStatus.SetText(0, buf);
	delete[] buf;
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
		DirectoryListing::Directory* pd = ii->file->getParent();
		while(pd != NULL && pd != dl->getRoot()) {
			fullPath = _T("\\") + WinUtil::toT(pd->getName()) + fullPath;
			pd = pd->getParent();
		}
	} else if(ii->type == ItemInfo::DIRECTORY) {
		if(!(ii->dir->getAdls() && ii->dir->getParent() != dl->getRoot()))
			return 0;
		fullPath = WinUtil::toT(((DirectoryListing::AdlDirectory*)ii->dir)->getFullPath());
	}

	// Break full path
	TStringList brokenPath;
	while(1) {
		if(fullPath.size() == 0 || fullPath[0] != '\\') 
			break;
		fullPath.erase(0, 1);
		tstring subPath = fullPath.substr(0, fullPath.find_first_of('\\'));
		fullPath.erase(0, subPath.size());
		brokenPath.push_back(subPath);
	}
	
	// Go to directory (recursive)
	TStringList::iterator iPath = brokenPath.begin();
	GoToDirectory(ctrlTree.GetRootItem(), iPath, brokenPath.end());
	
	return 0;
}


void DirectoryListingFrame::GoToDirectory(
	HTREEITEM hItem, 
	TStringList::iterator& iPath, 
	const TStringList::iterator& iPathEnd)
{
	if(iPath == iPathEnd)
		return;	// unexpected
	if(!ctrlTree.ItemHasChildren(hItem))
		return; // unexpected

	// Check on tree children
	HTREEITEM hChild = ctrlTree.GetChildItem(hItem);
	TCHAR itemText[256];
	while(hChild != NULL) {
		if(!ctrlTree.GetItemText(hChild, itemText, 255))
			return; // unexpected
		if(Util::stricmp(*iPath, itemText) == 0) {
			++iPath;
			if(iPath == iPathEnd) {
				ctrlTree.SelectItem(hChild);
				ctrlTree.EnsureVisible(hChild);
				return;
			}
			GoToDirectory(hChild, iPath, iPathEnd);
			return;
		}
		hChild = ctrlTree.GetNextItem(hChild, TVGN_NEXT);
	}
}

LRESULT DirectoryListingFrame::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	RECT rc;                    // client area of window 
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 
	
	fileMenu.RemoveMenu(IDC_GO_TO_DIRECTORY, MF_BYCOMMAND);

	// Get the bounding rectangle of the client area. 
	ctrlList.GetClientRect(&rc);
	ctrlList.ScreenToClient(&pt); 

	WinUtil::AppendSearchMenu(searchMenu);

	if (PtInRect(&rc, pt) && ctrlList.GetSelectedCount() > 0) {
		int n = 0;
		ctrlList.ClientToScreen(&pt);

		ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));

		while(targetMenu.GetMenuItemCount() > 0) {
			targetMenu.DeleteMenu(0, MF_BYPOSITION);
		}

		
		if(downloadPaths.size() > 0) {
			for(StringIter i = downloadPaths.begin(); i != downloadPaths.end(); ++i) {
				targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET + (n++), i->c_str());
			}
			targetMenu.AppendMenu(MF_SEPARATOR);
		}

		targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOADTO, CSTRING(BROWSE));

		if(WinUtil::lastDirs.size() > 0) {
			targetMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
			for(StringIter i = WinUtil::lastDirs.begin(); i != WinUtil::lastDirs.end(); ++i) {
				targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET + (n++), i->c_str());
			}
		}
		
		tstring hash = ii->getText(COLUMN_TTH);
		if (ctrlList.GetSelectedCount() == 1 && hash.length() == 39) {
			fileMenu.EnableMenuItem(IDC_SEARCH_BY_TTH, MF_ENABLED);
		} else {
			fileMenu.EnableMenuItem(IDC_SEARCH_BY_TTH, MF_GRAYED);
		}

		if(ctrlList.GetSelectedCount() == 1 && ii->type == ItemInfo::FILE) {
			targets.clear();
			QueueManager::getInstance()->getTargetsBySize(targets, ii->file->getSize(), Util::getFileExt(ii->file->getName()));
			if(targets.size() > 0) {
				targetMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
				for(StringIter i = targets.begin(); i != targets.end(); ++i) {
					targetMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET + (n++), i->c_str());
				}
			}

			if(ii->file->getAdls())	{
				fileMenu.AppendMenu(MF_STRING, IDC_GO_TO_DIRECTORY, CTSTRING(GO_TO_DIRECTORY));
			}
			fileMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		} else {
			if(ii->dir->getAdls() && ii->dir->getParent() != dl->getRoot()) {
				fileMenu.AppendMenu(MF_STRING, IDC_GO_TO_DIRECTORY, CTSTRING(GO_TO_DIRECTORY));
			}
			fileMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		}
		
		return TRUE; 
	} else { 
		
		ctrlList.ClientToScreen(&pt);
		
		ctrlTree.GetClientRect(&rc);
		ctrlTree.ScreenToClient(&pt); 
		
		if (PtInRect(&rc, pt) && ctrlTree.GetSelectedItem() != NULL) { 
			// Strange, windows doesn't change the selection on right-click... (!)
			UINT a = 0;
			HTREEITEM ht = ctrlTree.HitTest(pt, &a);
			if(ht != NULL && ht != ctrlTree.GetSelectedItem())
				ctrlTree.SelectItem(ht);
			
			while(targetDirMenu.GetMenuItemCount() > 0) {
				targetDirMenu.DeleteMenu(0, MF_BYPOSITION);
			}

			int n = 0;
			if(downloadPaths.size() > 0) {
				for(StringIter i = downloadPaths.begin(); i != downloadPaths.end(); ++i) {
					targetDirMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET_DIR + n, i->c_str());
					++n;
				}
				targetDirMenu.AppendMenu(MF_SEPARATOR);
			}

			targetDirMenu.AppendMenu(MF_STRING, IDC_DOWNLOADDIRTO, CSTRING(BROWSE));

			if(WinUtil::lastDirs.size() > 0) {
				targetDirMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
				for(TStringIter i = WinUtil::lastDirs.begin(); i != WinUtil::lastDirs.end(); ++i) {
					targetDirMenu.AppendMenu(MF_STRING, IDC_DOWNLOAD_TARGET_DIR + (n++), i->c_str());
				}
			}
			
			ctrlTree.ClientToScreen(&pt);
			directoryMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
			
			return TRUE; 
		} 
	}
	
	return FALSE; 
}

LRESULT DirectoryListingFrame::onDownloadTarget(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	u_int newId = wID - IDC_DOWNLOAD_TARGET;
	dcassert(newId >= 0);
	
	if(ctrlList.GetSelectedCount() == 1) {
		ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));

		if(ii->type == ItemInfo::FILE) {
			
			try {
				if(newId < downloadPaths.size())
					dl->download(ii->file, downloadPaths[newId] + ii->file->getName());
				else if( (newId - downloadPaths.size()) < WinUtil::lastDirs.size() )
					dl->download(ii->file, WinUtil::lastDirs[newId - downloadPaths.size()] + ii->file->getName());
				else
					dl->download(ii->file, targets[newId - downloadPaths.size() - WinUtil::lastDirs.size()]);

			} catch(const Exception& e) {
				ctrlStatus.SetText(0, WinUtil::toT(e.getError()).c_str());
			} 
		} else {
			if(newId < downloadPaths.size())
				downloadList(downloadPaths[newId]);
			else
				downloadList(WinUtil::lastDirs[newId - downloadPaths.size()]);
		}
	} else if(ctrlList.GetSelectedCount() > 1) {
		if(newId < downloadPaths.size())
			downloadList(downloadPaths[newId]);
		else
			downloadList(WinUtil::lastDirs[newId - downloadPaths.size()]);
	}
	return 0;
}

LRESULT DirectoryListingFrame::onDownloadTargetDir(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	u_int newId = wID - IDC_DOWNLOAD_TARGET_DIR;
	dcassert(newId >= 0);
	
	HTREEITEM t = ctrlTree.GetSelectedItem();
	if(t != NULL) {
		DirectoryListing::Directory* dir = (DirectoryListing::Directory*)ctrlTree.GetItemData(t);
		string target = SETTING(DOWNLOAD_DIRECTORY);
		try {
			if(newId < downloadPaths.size())
				dl->download(dir, downloadPaths[newId]);
			else
				dl->download(dir, WinUtil::lastDirs[newId - downloadPaths.size()]);

		} catch(const Exception& e) {
			ctrlStatus.SetText(0, WinUtil::toT(e.getError()).c_str());
		}
	}
	return 0;
}

LRESULT DirectoryListingFrame::onKeyDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMLVKEYDOWN* kd = (NMLVKEYDOWN*) pnmh;
	if(kd->wVKey == VK_BACK) {
		HTREEITEM cur = ctrlTree.GetSelectedItem();
		if(cur != NULL)
		{
			HTREEITEM parent = ctrlTree.GetParentItem(cur);
			if(parent != NULL)
				ctrlTree.SelectItem(parent);
		}
	} else if(kd->wVKey == VK_TAB) {
		onTab();
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
				downloadList(WinUtil::toT(SETTING(DOWNLOAD_DIRECTORY)));
			}
		} else {
			downloadList(WinUtil::toT(SETTING(DOWNLOAD_DIRECTORY)));
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
		int w[8];
		ctrlStatus.GetClientRect(sr);
		w[7] = sr.right - 16;
#define setw(x) w[x] = max(w[x+1] - statusSizes[x], 0)
		setw(6); setw(5); setw(4); setw(3); setw(2); setw(1); setw(0);

		ctrlStatus.SetParts(8, w);

		ctrlStatus.GetRect(6, sr);

		sr.left = w[4];
		sr.right = w[5];
		ctrlMatchQueue.MoveWindow(sr);

		sr.left = w[5];
		sr.right = w[6];
		ctrlFind.MoveWindow(sr);

		sr.left = w[6];
		sr.right = w[7];
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

		findStr = WinUtil::fromT(dlg.line);
		if(!dl->getUtf8())
			findStr = Util::toAcp(findStr);
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
				
				foundFile = ctrlList.findItem(WinUtil::toT(dir->getName()), -1, false);
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



LRESULT DirectoryListingFrame::onCopyFilename(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));
	if( ii->type == ItemInfo::FILE ){
		WinUtil::copyToClipboard(ii->file->getName());
	}else {
		WinUtil::copyToClipboard(ii->dir->getName());
	}
	return 0;
}

LRESULT DirectoryListingFrame::onCopyPath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));
	DirectoryListing::Directory* dir;

	if(ii->type == ItemInfo::FILE)
		dir = ii->file->getParent();
	else
		dir = ii->dir;

	DirectoryListing::Directory* tempDir = dir;
	stack< string > tempPath;
	string path;
	while(!tempDir->getName().empty()) {
		tempPath.push(tempDir->getName());
		tempDir = tempDir->getParent();
	}
	while(!tempPath.empty()){
		if(!path.empty()) {
			path += "\\";
		}
		path += tempPath.top();
		tempPath.pop();
	}
	
	if(!path.empty()) {
		path += "\\";
	}

	if(ii->type == ItemInfo::FILE) {
		WinUtil::copyToClipboard(path + ii->file->getName());
	} else {
		WinUtil::copyToClipboard(path); // + dir->getName());
	}

	return 0;
}

LRESULT DirectoryListingFrame::onCopySize(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));
	if(ii->type == ItemInfo::FILE) {
		WinUtil::copyToClipboard(Util::formatBytes(ii->file->getSize()));
	} else{
		WinUtil::copyToClipboard(Util::formatBytes(ii->dir->getTotalSize()));
	}
	return 0;
}

LRESULT DirectoryListingFrame::onSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	string searchTerm;
	ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));
	if( ii->type == ItemInfo::FILE ){
		searchTerm = ii->file->getName();
	}else {
		searchTerm = ii->dir->getName();
	}

	WinUtil::search(searchTerm, 0);
	return 0;
}

void DirectoryListingFrame::closeAll(){
	for(FrameIter i = frames.begin(); i != frames.end(); ++i)
		i->second->PostMessage(WM_CLOSE, 0, 0);

	frames.clear();
}

LRESULT DirectoryListingFrame::onMenuCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if(searchMenu.m_hMenu == (HMENU)lParam) {
		string searchTerm;
		ItemInfo* ii = (ItemInfo*)ctrlList.GetItemData(ctrlList.GetNextItem(-1, LVNI_SELECTED));
		if( ii->type == ItemInfo::FILE ){
			searchTerm = ii->file->getName();
		}else {
			searchTerm = ii->dir->getName();
		}

		WinUtil::search(searchTerm, wParam+1);
	} else {
		MENUITEMINFO inf;
		inf.cbSize = sizeof(MENUITEMINFO);
		inf.fMask = MIIM_ID;
		inf.wID = 0;

		if(fileMenu.m_hMenu == (HMENU)lParam) {
			fileMenu.GetMenuItemInfo(wParam, TRUE, &inf);
		} else if(copyMenu.m_hMenu == (HMENU)lParam) {
			copyMenu.GetMenuItemInfo(wParam, TRUE, &inf);
		} else if(targetMenu.m_hMenu == (HMENU)lParam) {
			targetMenu.GetMenuItemInfo(wParam, TRUE, &inf);
		} else if(targetDirMenu.m_hMenu == (HMENU)lParam) {
			targetDirMenu.GetMenuItemInfo(wParam, TRUE, &inf);
		}
		PostMessage(WM_COMMAND, inf.wID, 0);
	}
	return 0;
}
/**
 * @file
 * $Id: DirectoryListingFrm.cpp,v 1.11 2004/02/21 15:14:43 trem Exp $
 */
