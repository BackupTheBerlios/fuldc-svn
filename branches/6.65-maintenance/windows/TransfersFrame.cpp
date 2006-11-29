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

#include "../client/ResourceManager.h"
#include "../client/QueueManager.h"
#include "../client/ConnectionManager.h"
#include "../client/Socket.h"
#include "../client/IgnoreManager.h"

#include "WinUtil.h"
#include "TransfersFrame.h"
#include "LineDlg.h"
#include "memdc.h"
#include "SearchFrm.h"
#include "MainFrm.h"

int TransfersFrame::columnIndexes[] = { COLUMN_USER, COLUMN_HUB, COLUMN_STATUS, COLUMN_TIMELEFT, COLUMN_TOTALTIMELEFT, COLUMN_SPEED, COLUMN_FILE, COLUMN_SIZE, COLUMN_PATH, COLUMN_IP, COLUMN_RATIO };
int TransfersFrame::columnSizes[] = { 150, 100, 250, 75, 75, 75, 175, 100, 200, 50, 75 };

static ResourceManager::Strings columnNames[] = { ResourceManager::USER, ResourceManager::HUB, ResourceManager::STATUS,
ResourceManager::TIME_LEFT, ResourceManager::TOTAL_TIME_LEFT, ResourceManager::SPEED, ResourceManager::FILENAME, ResourceManager::SIZE, ResourceManager::PATH,
ResourceManager::IP_BARE, ResourceManager::RATIO};

LRESULT TransfersFrame::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {

	arrows.CreateFromImage(WinUtil::getIconPath(_T("arrows.bmp")).c_str(), 16, 2, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED | LR_LOADFROMFILE);
	categories.CreateFromImage(WinUtil::getIconPath(_T("transfers.bmp")).c_str(), 16, 3, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED | LR_LOADFROMFILE);

	ctrlTransfers.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_TRANSFERS);
	ctrlTransfers.SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);

	ctrlCategories.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | LVS_SINGLESEL | 
		LVS_NOCOLUMNHEADER, WS_EX_CLIENTEDGE, IDC_CATEGORIES);
	ctrlCategories.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	SetSplitterPanes(ctrlCategories.m_hWnd, ctrlTransfers.m_hWnd, true);
	m_nProportionalPos = 850;
	SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	
	ctrlCategories.SetImageList(categories, LVSIL_SMALL);
	ctrlCategories.SetBkColor(WinUtil::bgColor);
	ctrlCategories.SetTextBkColor(WinUtil::bgColor);
	ctrlCategories.SetTextColor(WinUtil::textColor);

	ctrlCategories.InsertColumn(0, _T("dummy"), LVCFMT_LEFT, 100);
	ctrlCategories.insert(0, TSTRING(ALL), 0);
	ctrlCategories.insert(1, TSTRING(DOWNLOADS), 1);
	ctrlCategories.insert(2, TSTRING(UPLOADS), 2);
	ctrlCategories.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
	

	WinUtil::splitTokens(columnIndexes, SETTING(MAINFRAME_ORDER), COLUMN_LAST);
	WinUtil::splitTokens(columnSizes, SETTING(MAINFRAME_WIDTHS), COLUMN_LAST);

	for(int j=0; j<COLUMN_LAST; j++) {
		int fmt = (j == COLUMN_SIZE || j == COLUMN_TIMELEFT || j == COLUMN_TOTALTIMELEFT || j == COLUMN_SPEED) ? LVCFMT_RIGHT : LVCFMT_LEFT;
		ctrlTransfers.InsertColumn(j, CTSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
	}

	ctrlTransfers.setColumnOrderArray(COLUMN_LAST, columnIndexes);
	ctrlTransfers.setVisible(SETTING(MAINFRAME_VISIBLE));

	ctrlTransfers.SetBkColor(WinUtil::bgColor);
	ctrlTransfers.SetTextBkColor(WinUtil::bgColor);
	ctrlTransfers.SetTextColor(WinUtil::textColor);
	ctrlTransfers.setOwnerDraw(true);

	ctrlTransfers.SetImageList(arrows, LVSIL_SMALL);
	ctrlTransfers.setSortColumn(COLUMN_STATUS);

	openMenu.CreatePopupMenu();
	openMenu.AppendMenu(MF_STRING, IDC_OPEN, CTSTRING(OPEN));
	openMenu.AppendMenu(MF_STRING, IDC_OPEN_FOLDER, CTSTRING(OPEN_FOLDER));

	pmMenu.CreatePopupMenu();
	pmMenu.AppendMenu(MF_STRING, IDC_PRIVATEMESSAGE, CTSTRING(USER));
	pmMenu.AppendMenu(MF_STRING, IDC_PM_UP, CTSTRING(ALL_UPLOADS));
	pmMenu.AppendMenu(MF_STRING, IDC_PM_DOWN, CTSTRING(ALL_DOWNLOADS));
	pmMenu.SetMenuDefaultItem(IDC_PRIVATEMESSAGE);

	ucMenu.CreatePopupMenu(); // this will be populated in onContextMenu

	copyMenu.CreatePopupMenu();
	ctrlTransfers.buildCopyMenu(copyMenu);

	transferMenu.CreatePopupMenu();
	transferMenu.AppendMenu(MF_STRING, IDC_GETLIST, CTSTRING(GET_FILE_LIST));
	transferMenu.AppendMenu(MF_STRING, IDC_BROWSELIST, CTSTRING(BROWSE_FILE_LIST));
	transferMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)pmMenu, CTSTRING(SEND_PRIVATE_MESSAGE));
	transferMenu.AppendMenu(MF_STRING, IDC_MATCH_QUEUE, CTSTRING(MATCH_QUEUE));
	transferMenu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CTSTRING(ADD_TO_FAVORITES));
	transferMenu.AppendMenu(MF_STRING, IDC_GRANTSLOT, CTSTRING(GRANT_EXTRA_SLOT));
	transferMenu.AppendMenu(MF_STRING, IDC_REMOVEALL, CTSTRING(REMOVE_FROM_ALL));
	transferMenu.AppendMenu(MF_STRING, IDC_SHOWLOG, CTSTRING(SHOW_LOG));
	transferMenu.AppendMenu(MF_SEPARATOR);
	transferMenu.AppendMenu(MF_STRING, IDC_IGNORE, CTSTRING(IGNOREA));
	transferMenu.AppendMenu(MF_STRING, IDC_UNIGNORE, CTSTRING(UNIGNORE));
	transferMenu.AppendMenu(MF_SEPARATOR);
	transferMenu.AppendMenu(MF_POPUP, (HMENU)ucMenu, CTSTRING(USERCOMMANDS));
	transferMenu.AppendMenu(MF_SEPARATOR);
	transferMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)copyMenu, CTSTRING(COPY));
	transferMenu.AppendMenu(MF_SEPARATOR);
	transferMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)openMenu, CTSTRING(OPEN));
	transferMenu.AppendMenu(MF_SEPARATOR);
	transferMenu.AppendMenu(MF_STRING, IDC_SEARCH_ALTERNATES, CTSTRING(SEARCH_FOR_ALTERNATES));
	transferMenu.AppendMenu(MF_STRING, IDC_FORCE, CTSTRING(FORCE_ATTEMPT));
	transferMenu.AppendMenu(MF_SEPARATOR);
	transferMenu.AppendMenu(MF_STRING, IDC_REMOVE, CTSTRING(CLOSE_CONNECTION));
	transferMenu.AppendMenu(MF_STRING, IDC_REMOVE_FILE, CTSTRING(REMOVE_FILE));
	transferMenu.SetMenuDefaultItem(IDC_PRIVATEMESSAGE);

	WinUtil::SetIcon(m_hWnd, _T("transfers.ico"));

	TransfersManager::getInstance()->addListener(this);
	TransfersManager::getInstance()->fireInitialList();

	bHandled = FALSE;
	return 1;
}

LRESULT TransfersFrame::onClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	TransfersManager::getInstance()->removeListener(this);

	ctrlTransfers.saveHeaderOrder(SettingsManager::MAINFRAME_ORDER, SettingsManager::MAINFRAME_WIDTHS,
		SettingsManager::MAINFRAME_VISIBLE);
	
	ctrlTransfers.forEach(&TransferInfo::deleteSelf);

	checkButton(false);

	bHandled = FALSE;
	return 0;
}

void TransfersFrame::UpdateLayout(BOOL bResizeBars /* = TRUE */) {
	RECT rect;
	GetClientRect(&rect);
	// position bars and offset their dimensions
	UpdateBarsPosition(rect, bResizeBars);

	/*if(ctrlStatus.IsWindow()) {
		CRect sr;
		int w[3];
		ctrlStatus.GetClientRect(sr);
		int tmp = (sr.Width()) > 316 ? 216 : ((sr.Width() > 116) ? sr.Width()-100 : 16);

		w[0] = sr.right - tmp;
		w[1] = w[0] + (tmp-16)/2;
		w[2] = w[0] + (tmp-16);

		ctrlStatus.SetParts(3, w);
	}*/

	CRect rc = rect;
	//ctrlTransfers.MoveWindow(rc);
	SetSplitterRect(rc);
}

LRESULT TransfersFrame::onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMLISTVIEW* nm = reinterpret_cast<NMLISTVIEW*>(pnmh);

	if(nm->uNewState & LVIS_SELECTED) {
		category = nm->iItem;

		ctrlTransfers.SetRedraw(FALSE);
		ctrlTransfers.DeleteAllItems();

		for(TransferIter i = transfers.begin(); i != transfers.end(); ++i) {
			TransferInfo* ii = *i;
			if((ii->download && category != UPLOADS) || (!ii->download && category != DOWNLOADS)) {
				ctrlTransfers.insertItem(ii, ii->download ? IMAGE_DOWNLOAD : IMAGE_UPLOAD);
			}
		}

		ctrlTransfers.SetRedraw(TRUE);
	}
	return 0;
}

LRESULT TransfersFrame::onContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (reinterpret_cast<HWND>(wParam) == ctrlTransfers && ctrlTransfers.GetSelectedCount() > 0) { 
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		
		if(pt.x == -1 && pt.y == -1) {
			WinUtil::getContextMenuPos(ctrlTransfers, pt);
		}

		int i = -1;
		TransferInfo* itemI;
		bool bCustomMenu = false;
		if( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
			itemI = ctrlTransfers.getItemData(i);
			bCustomMenu = true;

			prepareMenu(ucMenu, UserCommand::CONTEXT_CHAT, itemI->user->getClientUrl());
			//remove the first separator appended by prepareMenu
			ucMenu.RemoveMenu(0, MF_BYPOSITION); 
		}

		pmMenu.RemoveMenu(2, MF_BYPOSITION);
		pmMenu.RemoveMenu(1, MF_BYPOSITION);

		if(category != UPLOADS)
			pmMenu.AppendMenu(MF_STRING, IDC_PM_DOWN, CTSTRING(ALL_DOWNLOADS));
		if(category != DOWNLOADS)
			pmMenu.AppendMenu(MF_STRING, IDC_PM_UP, CTSTRING(ALL_UPLOADS));
		
		if(ctrlTransfers.GetSelectedCount() == 1) {
			int pos = ctrlTransfers.GetNextItem(-1, LVNI_SELECTED);
			if(pos != -1) {
				TransferInfo* ii = (TransferInfo*)ctrlTransfers.getItemData(pos);
				if(IgnoreManager::getInstance()->isUserIgnored(ii->user->getNick())) {
					transferMenu.EnableMenuItem(IDC_IGNORE, MF_GRAYED);
					transferMenu.EnableMenuItem(IDC_UNIGNORE, MF_ENABLED);
				} else {
					transferMenu.EnableMenuItem(IDC_IGNORE, MF_ENABLED);
					transferMenu.EnableMenuItem(IDC_UNIGNORE, MF_GRAYED);
				}
				
				
				//check that we have a filename and that it's not a file list
				if(!ii->getText(COLUMN_FILE).empty() && !ii->filelist) {
					transferMenu.EnableMenuItem(IDC_SEARCH_ALTERNATES, MF_ENABLED);
				} else {
					transferMenu.EnableMenuItem(IDC_SEARCH_ALTERNATES, MF_GRAYED);
				}
			}
		} else {
			transferMenu.EnableMenuItem(IDC_SEARCH_ALTERNATES, MF_GRAYED);
			transferMenu.EnableMenuItem(IDC_IGNORE, MF_ENABLED);
			transferMenu.EnableMenuItem(IDC_UNIGNORE, MF_ENABLED);
		}

		transferMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);

		if ( bCustomMenu ) {
			//transferMenu.DeleteMenu(transferMenu.GetMenuItemCount()-1, MF_BYPOSITION);
			cleanMenu(ucMenu);
		}
		return TRUE; 
	}
	bHandled = FALSE;
	return FALSE; 
}

void TransfersFrame::runUserCommand(UserCommand& uc) {
	StringMap ucParams;
	if(!WinUtil::getUCParams(m_hWnd, uc, ucParams))
		return;

	int i = -1;
	while((i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		TransferInfo* itemI = ctrlTransfers.getItemData(i);
		if(!itemI->user->isOnline())
			continue;

		ucParams["mynick"] = itemI->user->getClientNick();
		ucParams["file"] = Text::fromT(itemI->getText(COLUMN_PATH)) + Text::fromT(itemI->getText(COLUMN_FILE));

		StringMap tmp = ucParams;
		itemI->user->getParams(tmp);
		itemI->user->clientEscapeParams(tmp);
		itemI->user->sendUserCmd(Util::formatParams(uc.getCommand(), tmp, false));
	}
	return;
};


LRESULT TransfersFrame::onForce(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		ctrlTransfers.SetItemText(i, COLUMN_STATUS, CTSTRING(CONNECTING_FORCED));
		((TransferInfo*)ctrlTransfers.getItemData(i))->user->connect();
	}
	return 0;
}

LRESULT TransfersFrame::onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled) {
	if(!BOOLSETTING(SHOW_PROGRESS_BARS)) {
		bHandled = FALSE;
		return 0;
	}

	LPNMLVCUSTOMDRAW cd = (LPNMLVCUSTOMDRAW)pnmh;

	switch(cd->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;

	case CDDS_ITEMPREPAINT:
		return CDRF_NOTIFYSUBITEMDRAW;

	case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
		// Let's draw a box if needed...
		if(ctrlTransfers.findColumn(cd->iSubItem) == COLUMN_STATUS) {
			TransferInfo* ii = reinterpret_cast<TransferInfo*>(cd->nmcd.lItemlParam);
			if(ii->status == TransferInfo::STATUS_RUNNING) {
				// draw something nice...	
				COLORREF barBase = ii->download ? SETTING(DOWNLOAD_BAR_COLOR) : SETTING(UPLOAD_BAR_COLOR);
				COLORREF bgBase = WinUtil::bgColor;
				int mod = (HLS_L(RGB2HLS(bgBase)) >= 128) ? -30 : 30;
				COLORREF barPal[3] = { HLS_TRANSFORM(barBase, -40, 50), barBase, HLS_TRANSFORM(barBase, 40, -30) };
				COLORREF bgPal[2] = { HLS_TRANSFORM(bgBase, mod, 0), HLS_TRANSFORM(bgBase, mod/2, 0) };

				CRect rc, rc2;
				
				//this is just severely broken, msdn says GetSubItemRect requires a one based index
				//but it wont work and index 0 gives the rect of the whole item
				if(cd->iSubItem == 0) {
					//use LVIR_LABEL to exclude the icon area since we will be painting over that
					//later
					ctrlTransfers.GetItemRect((int)cd->nmcd.dwItemSpec, &rc, LVIR_LABEL);
				} else {
					ctrlTransfers.GetSubItemRect((int)cd->nmcd.dwItemSpec, cd->iSubItem, LVIR_BOUNDS, &rc);
				}
				
				rc2 = rc;
				rc2.left += 6;
				
				// draw background
				HGDIOBJ oldpen = ::SelectObject(cd->nmcd.hdc, CreatePen(PS_SOLID,0,bgPal[0]));
				HGDIOBJ oldbr = ::SelectObject(cd->nmcd.hdc, CreateSolidBrush(bgPal[1]));
				::Rectangle(cd->nmcd.hdc, rc.left, rc.top - 1, rc.right, rc.bottom);			
				rc.DeflateRect(1, 0, 1, 1);

				LONG left = rc.left;
				int64_t w = rc.Width();
				// draw start part
				if(ii->size == 0)
					ii->size = 1;
				rc.right = left + (int) (w * ii->start / ii->size);
				DeleteObject(SelectObject(cd->nmcd.hdc, CreateSolidBrush(barPal[0])));
				DeleteObject(SelectObject(cd->nmcd.hdc, CreatePen(PS_SOLID,0,barPal[0])));
				
				::Rectangle(cd->nmcd.hdc, rc.left, rc.top, rc.right, rc.bottom);
				
				// Draw actual part
				rc.left = rc.right;
				rc.right = left + (int) (w * ii->actual / ii->size);
				DeleteObject(SelectObject(cd->nmcd.hdc, CreateSolidBrush(barPal[1])));

				::Rectangle(cd->nmcd.hdc, rc.left, rc.top, rc.right, rc.bottom);

				// And the effective part...
				if(ii->pos > ii->actual) {
					rc.left = rc.right - 1;
					rc.right = left + (int) (w * ii->pos / ii->size);
					DeleteObject(SelectObject(cd->nmcd.hdc, CreateSolidBrush(barPal[2])));

					::Rectangle(cd->nmcd.hdc, rc.left, rc.top, rc.right, rc.bottom);

				}
				rc.left = left;
				// draw progressbar highlight
				if(rc.Width()>2) {
					DeleteObject(SelectObject(cd->nmcd.hdc, CreatePen(PS_SOLID,1,barPal[2])));

					rc.top += 2;
					::MoveToEx(cd->nmcd.hdc,rc.left+1,rc.top,(LPPOINT)NULL);
					::LineTo(cd->nmcd.hdc,rc.right-2,rc.top);
				}
				
				// draw status text
				DeleteObject(::SelectObject(cd->nmcd.hdc, oldpen));
				DeleteObject(::SelectObject(cd->nmcd.hdc, oldbr));

				LONG right = rc2.right;
				left = rc2.left;
				rc2.right = rc.right;
				LONG top = rc2.top + (rc2.Height() - WinUtil::getTextHeight(cd->nmcd.hdc) - 1)/2;
				SetTextColor(cd->nmcd.hdc, RGB(255, 255, 255));
				::ExtTextOut(cd->nmcd.hdc, left, top, ETO_CLIPPED, rc2, ii->getText(COLUMN_STATUS).c_str(),
					ii->getText(COLUMN_STATUS).length(), NULL);

				rc2.left = rc2.right;
				rc2.right = right;

				SetTextColor(cd->nmcd.hdc, WinUtil::textColor);
				::ExtTextOut(cd->nmcd.hdc, left, top, ETO_CLIPPED, rc2, ii->getText(COLUMN_STATUS).c_str(),
					ii->getText(COLUMN_STATUS).length(), NULL);

				//bah crap, if we return CDRF_SKIPDEFAULT windows won't paint the icons
				//so we have to do it
				if(cd->iSubItem == 0){
					LVITEM lvItem;
					lvItem.iItem = cd->nmcd.dwItemSpec;
					lvItem.iSubItem = 0;
					lvItem.mask = LVIF_IMAGE | LVIF_STATE;
					lvItem.stateMask = LVIS_SELECTED;
					ctrlTransfers.GetItem(&lvItem);

					HIMAGELIST imageList = (HIMAGELIST)::SendMessage(ctrlTransfers.m_hWnd, LVM_GETIMAGELIST, LVSIL_SMALL, 0);
					if(imageList) {
						//let's find out where to paint it
						//and draw the background to avoid having 
						//the selection color as background
						CRect iconRect;
						ctrlTransfers.GetSubItemRect((int)cd->nmcd.dwItemSpec, 0, LVIR_ICON, iconRect);

						//we don't need to paint the background if the item's not selected
						//since it will already have the right color
						if(lvItem.state & LVIS_SELECTED) {
							HBRUSH brush = (HBRUSH)::SendMessage(::GetParent(ctrlTransfers.m_hWnd), WM_CTLCOLORLISTBOX, (WPARAM)cd->nmcd.hdc, (LPARAM)ctrlTransfers.m_hWnd);
							if(brush) {
								//remove 4 pixels to repaint the offset between the
								//column border and the icon.
								iconRect.left -= 4;
								::FillRect(cd->nmcd.hdc, &iconRect, brush);

								//have to add them back otherwise the icon will be painted
								//in the wrong place
								iconRect.left += 4;
							}
						}
						ImageList_Draw(imageList, lvItem.iImage, cd->nmcd.hdc, iconRect.left, iconRect.top, ILD_TRANSPARENT);
					}
				}

				return CDRF_SKIPDEFAULT;
			}
		}
		// Fall through
	default:
		return CDRF_DODEFAULT;
	}
}

LRESULT TransfersFrame::onDoubleClickTransfers(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;
	if (item->iItem != -1 ) {
		ctrlTransfers.getItemData(item->iItem)->pm();
	}
	return 0;
}

LRESULT TransfersFrame::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if(wParam == ADD_ITEM) {
		TransferInfo* ti = reinterpret_cast<TransferInfo*>(lParam);
		transfers.push_back(ti);
		if((ti->download && category != UPLOADS) || (!ti->download && category != DOWNLOADS))
			ctrlTransfers.insertItem(ti, ti->download ? IMAGE_DOWNLOAD : IMAGE_UPLOAD);
	} else if(wParam == REMOVE_ITEM) {
		auto_ptr<TransferInfo> ui(reinterpret_cast<TransferInfo*>(lParam));
		int ic = ctrlTransfers.GetItemCount(); 
		for(int i = 0; i < ic; ++i) {
			TransferInfo* ti = ctrlTransfers.getItemData(i);
			if(*ui == *ti) {
				ctrlTransfers.DeleteItem(i);
				break;
			}
		}
		for(TransferIter i = transfers.begin(); i != transfers.end(); ++i) {
			if((*ui) == (*(*i))) {
				delete *i;
				transfers.erase(i);
				break;
			}
		}
	} else if(wParam == UPDATE_ITEM) {
		auto_ptr<TransferInfo> ui(reinterpret_cast<TransferInfo*>(lParam));
		for(TransferIter i = transfers.begin(); i != transfers.end(); ++i) {
			TransferInfo* ti = *i;
			if(*ui == *ti) {
				*ti = *ui;
			}
		}
		int ic = ctrlTransfers.GetItemCount(); 
		for(int i = 0; i < ic; ++i) {
			TransferInfo* ti = ctrlTransfers.getItemData(i);
			if(ti->download == ui->download && ti->user == ui->user) {
				ctrlTransfers.updateItem(i);
				break;
			}
		}
	}

	ctrlTransfers.resort();

	if(BOOLSETTING(BOLD_TRANSFERS))
		setDirty();

	return 0;
}

LRESULT TransfersFrame::onSearchAlternates(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = ctrlTransfers.GetNextItem(-1, LVNI_SELECTED);

	if(i != -1) {
		TransferInfo *ti = ctrlTransfers.getItemData(i);

		string target = Text::fromT(ti->getText(COLUMN_PATH) + ti->getText(COLUMN_FILE));

		TTHValue tth;
		if(QueueManager::getInstance()->getTTH(target, tth)) {
			WinUtil::searchHash(tth);
		}
	}

	return 0;
}

LRESULT TransfersFrame::onOpen(WORD , WORD wID, HWND , BOOL& ) {
	TransferInfo *ii = ctrlTransfers.getItemData(ctrlTransfers.GetNextItem(-1, LVNI_SELECTED));

	tstring path;

	if(wID == IDC_OPEN) {
		path = ii->getText(COLUMN_PATH) + ii->getText(COLUMN_FILE);
	} else {
		path = ii->getText(COLUMN_PATH);
	}

	ShellExecute(NULL, _T("open"), path.c_str(), NULL, NULL, SW_SHOW);

	return 0;
}

LRESULT TransfersFrame::onCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	ctrlTransfers.copy(copyMenu, wID - IDC_COPY);

	return 0;
}

LRESULT TransfersFrame::onRemoveFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		TransferInfo *ii = ctrlTransfers.getItemData(i);
		QueueManager::getInstance()->remove(Text::fromT(ii->getText(COLUMN_PATH) + ii->getText(COLUMN_FILE)));
	}

	return 0;
}

LRESULT TransfersFrame::onPmAll(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	bool download = wID == IDC_PM_DOWN;

	LineDlg dlg;
	if(dlg.DoModal() == IDCANCEL)
		return 0;

	tstring msg = dlg.line;

	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_ALL)) != -1) {
		TransferInfo *ii = ctrlTransfers.getItemData(i);
		if(ii->download == download && ii->status == TransferInfo::STATUS_RUNNING)
			ii->pm(msg);
	}

	return 0;	
}
