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
#include "Resource.h"

#include "../client/ResourceManager.h"
#include "../client/QueueManager.h"
#include "../client/ConnectionManager.h"
#include "../client/Socket.h"

#include "WinUtil.h"
#include "TransferView.h"
#include "LineDlg.h"
#include "memdc.h"

int TransferView::columnIndexes[] = { COLUMN_USER, COLUMN_HUB, COLUMN_STATUS, COLUMN_TIMELEFT, COLUMN_TOTALTIMELEFT, COLUMN_SPEED, COLUMN_FILE, COLUMN_SIZE, COLUMN_PATH, COLUMN_IP, COLUMN_RATIO };
int TransferView::columnSizes[] = { 150, 100, 250, 75, 75, 75, 175, 100, 200, 50, 75 };

static ResourceManager::Strings columnNames[] = { ResourceManager::USER, ResourceManager::HUB, ResourceManager::STATUS,
ResourceManager::TIME_LEFT, ResourceManager::TOTAL_TIME_LEFT, ResourceManager::SPEED, ResourceManager::FILENAME, ResourceManager::SIZE, ResourceManager::PATH,
ResourceManager::IP_BARE, ResourceManager::RATIO};

TransferView::~TransferView() {
	delete[] headerBuf;
	arrows.Destroy();
}

LRESULT TransferView::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	arrows.CreateFromImage("icons\\arrows.bmp", 16, 2, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED | LR_LOADFROMFILE);
	ctrlTransfers.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_TRANSFERS);
	ctrlTransfers.SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);

	WinUtil::splitTokens(columnIndexes, SETTING(MAINFRAME_ORDER), COLUMN_LAST);
	WinUtil::splitTokens(columnSizes, SETTING(MAINFRAME_WIDTHS), COLUMN_LAST);

	for(int j=0; j<COLUMN_LAST; j++) {
		int fmt = (j == COLUMN_SIZE || j == COLUMN_TIMELEFT || j == COLUMN_TOTALTIMELEFT || j == COLUMN_SPEED) ? LVCFMT_RIGHT : LVCFMT_LEFT;
		ctrlTransfers.insertColumn(j, CTSTRING_I(columnNames[j]), fmt, columnSizes[j], j);
	}

	ctrlTransfers.setColumnOrderArray(COLUMN_LAST, columnIndexes);
	ctrlTransfers.setVisible(SETTING(MAINFRAME_VISIBLE));

	ctrlTransfers.SetBkColor(WinUtil::bgColor);
	ctrlTransfers.SetTextBkColor(WinUtil::bgColor);
	ctrlTransfers.SetTextColor(WinUtil::textColor);

	ctrlTransfers.SetImageList(arrows, LVSIL_SMALL);
	ctrlTransfers.setSortColumn(COLUMN_USER);

	openMenu.CreatePopupMenu();
	openMenu.AppendMenu(MF_STRING, IDC_OPEN, CSTRING(OPEN));
	openMenu.AppendMenu(MF_STRING, IDC_OPEN_FOLDER, CSTRING(OPEN_FOLDER));

	pmMenu.CreatePopupMenu();
	pmMenu.AppendMenu(MF_STRING, IDC_PRIVATEMESSAGE, CSTRING(USER));
	pmMenu.AppendMenu(MF_STRING, IDC_PM_UP, CSTRING(ALL_UPLOADS));
	pmMenu.AppendMenu(MF_STRING, IDC_PM_DOWN, CSTRING(ALL_DOWNLOADS));

	copyMenu.CreatePopupMenu();
	for(int i = 0; i < COLUMN_LAST; ++i)
		copyMenu.AppendMenu(MF_STRING, IDC_COPY+i, CSTRING_I(columnNames[i]));

	transferMenu.CreatePopupMenu();
	transferMenu.AppendMenu(MF_STRING, IDC_GETLIST, CTSTRING(GET_FILE_LIST));
	transferMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)pmMenu, CTSTRING(SEND_PRIVATE_MESSAGE));
	transferMenu.AppendMenu(MF_STRING, IDC_MATCH_QUEUE, CTSTRING(MATCH_QUEUE));
	transferMenu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CTSTRING(ADD_TO_FAVORITES));
	transferMenu.AppendMenu(MF_STRING, IDC_GRANTSLOT, CTSTRING(GRANT_EXTRA_SLOT));
	transferMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)openMenu, CTSTRING(OPEN));
	transferMenu.AppendMenu(MF_POPUP, (UINT)(HMENU)copyMenu, CTSTRING(COPY));
	transferMenu.AppendMenu(MF_STRING, IDC_RESOLVE_IP, CTSTRING(RESOLVE_IP));
	transferMenu.AppendMenu(MF_STRING, IDC_FORCE, CTSTRING(FORCE_ATTEMPT));
	transferMenu.AppendMenu(MF_SEPARATOR);
	transferMenu.AppendMenu(MF_STRING, IDC_REMOVE, CTSTRING(CLOSE_CONNECTION));
	transferMenu.AppendMenu(MF_STRING, IDC_REMOVEALL, CTSTRING(REMOVE_FROM_ALL));
	transferMenu.SetMenuDefaultItem(IDC_PRIVATEMESSAGE);

	ConnectionManager::getInstance()->addListener(this);
	DownloadManager::getInstance()->addListener(this);
	UploadManager::getInstance()->addListener(this);
#if 0
	ItemInfo* ii = new ItemInfo(ClientManager::getInstance()->getUser("test"), 
		ItemInfo::TYPE_DOWNLOAD, ItemInfo::STATUS_RUNNING, 75, 100, 25, 50);
	ctrlTransfers.insert(0, tstring("Test"), 0, (LPARAM)ii);
#endif
	return 0;
}

void TransferView::prepareClose() {
	ctrlTransfers.saveHeaderOrder(SettingsManager::MAINFRAME_ORDER, SettingsManager::MAINFRAME_WIDTHS,
		SettingsManager::MAINFRAME_VISIBLE);
	
	ConnectionManager::getInstance()->removeListener(this);
	DownloadManager::getInstance()->removeListener(this);
	UploadManager::getInstance()->removeListener(this);

}

LRESULT TransferView::onSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	RECT rc;
	GetClientRect(&rc);
	ctrlTransfers.MoveWindow(&rc);

	return 0;
}

LRESULT TransferView::onContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	RECT rc, rc2;                    // client area of window 
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };        // location of mouse click 

	// Get the bounding rectangle of the client area. 
	ctrlTransfers.GetWindowRect(&rc);
	ctrlTransfers.GetHeader().GetWindowRect(&rc2);
	if(PtInRect(&rc2, pt)){
		ctrlTransfers.showMenu(pt);
		return TRUE;
	}

	if (PtInRect(&rc, pt) && ctrlTransfers.GetSelectedCount() > 0) 
	{ 
		int i = -1;
		ItemInfo* itemI;
		bool bCustomMenu = false;
		if( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
			itemI = ctrlTransfers.getItemData(i);
			bCustomMenu = true;

			prepareMenu(transferMenu, UserCommand::CONTEXT_CHAT, WinUtil::toT(itemI->user->getClientAddressPort()), itemI->user->isClientOp());
			transferMenu.AppendMenu(MF_SEPARATOR);
		}

		transferMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);

		if ( bCustomMenu ) {
			transferMenu.DeleteMenu(transferMenu.GetMenuItemCount()-1, MF_BYPOSITION);
			cleanMenu(transferMenu);
		}
		return TRUE; 
	}
	return FALSE; 
}

void TransferView::runUserCommand(UserCommand& uc) {
	if(!WinUtil::getUCParams(m_hWnd, uc, ucParams))
		return;

	int i = -1;
	while((i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		ItemInfo* itemI = ctrlTransfers.getItemData(i);

		ucParams["mynick"] = itemI->user->getClientNick();

		itemI->user->getParams(ucParams);

		itemI->user->send(Util::formatParams(uc.getCommand(), ucParams));
		}
	return;
};


LRESULT TransferView::onForce(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		ctrlTransfers.SetItemText(i, COLUMN_STATUS, CTSTRING(CONNECTING_FORCED));
		ctrlTransfers.getItemData(i)->user->connect();
	}
	return 0;
}

void TransferView::ItemInfo::removeAll() {
	QueueManager::getInstance()->removeSources(user, QueueItem::Source::FLAG_REMOVED);
}

LRESULT TransferView::onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled) {
	if(!BOOLSETTING(SHOW_PROGRESS_BARS)) {
		bHandled = FALSE;
		return 0;
	}

	CRect rc;
	LPNMLVCUSTOMDRAW cd = (LPNMLVCUSTOMDRAW)pnmh;

	switch(cd->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;

	case CDDS_ITEMPREPAINT:
		return CDRF_NOTIFYSUBITEMDRAW;

	case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
		// Let's draw a box if needed...
		LVCOLUMN lvc;
		lvc.mask = LVCF_TEXT;
		lvc.pszText = headerBuf;
		lvc.cchTextMax = 128;
		ctrlTransfers.GetColumn(cd->iSubItem, &lvc);
		if(Util::stricmp(headerBuf, CSTRING_I(columnNames[COLUMN_STATUS])) == 0) {
			ItemInfo* ii = (ItemInfo*)cd->nmcd.lItemlParam;
			if(ii->status == ItemInfo::STATUS_RUNNING) {
				// draw something nice...	
				TCHAR buf[256];
				COLORREF barBase = ii->type == ItemInfo::TYPE_DOWNLOAD ? SETTING(DOWNLOAD_BAR_COLOR) : SETTING(UPLOAD_BAR_COLOR);
				COLORREF bgBase = WinUtil::bgColor;
				int mod = (HLS_L(RGB2HLS(bgBase)) >= 128) ? -30 : 30;
				COLORREF barPal[3] = { HLS_TRANSFORM(barBase, -40, 50), barBase, HLS_TRANSFORM(barBase, 40, -30) };
				COLORREF bgPal[2] = { HLS_TRANSFORM(bgBase, mod, 0), HLS_TRANSFORM(bgBase, mod/2, 0) };

				ctrlTransfers.GetItemText((int)cd->nmcd.dwItemSpec, cd->iSubItem, buf, 255);
				buf[255] = 0;

				ctrlTransfers.GetSubItemRect((int)cd->nmcd.dwItemSpec, cd->iSubItem, LVIR_BOUNDS, rc);
				CRect rc2 = rc;
				rc2.left += 6;
				CMemDC memDC(cd->nmcd.hdc, &rc);
				
				// draw background
				HPEN oldpen = memDC.SelectPen(CreatePen(PS_SOLID,0,bgPal[0]));
				HBRUSH oldbr = memDC.SelectBrush(CreateSolidBrush(bgPal[1]));
				memDC.Rectangle(rc.left, rc.top - 1, rc.right, rc.bottom);			
				rc.DeflateRect(1, 0, 1, 1);

				int left = rc.left;
				int w = rc.Width();
				// draw start part
				if(ii->size == 0)
					ii->size = 1;
				rc.right = left + (int) (w * ii->start / ii->size);
				DeleteObject(memDC.SelectBrush(CreateSolidBrush(barPal[0])));
				DeleteObject(memDC.SelectPen(CreatePen(PS_SOLID,0,barPal[0])));
				
				memDC.Rectangle(rc.left, rc.top, rc.right, rc.bottom);
				
				// Draw actual part
				rc.left = rc.right;
				rc.right = left + (int) (w * ii->actual / ii->size);
				DeleteObject(memDC.SelectBrush(CreateSolidBrush(barPal[1])));

				memDC.Rectangle(rc.left, rc.top, rc.right, rc.bottom);

				// And the effective part...
				if(ii->pos > ii->actual) {
					rc.left = rc.right - 1;
					rc.right = left + (int) (w * ii->pos / ii->size);
					DeleteObject(memDC.SelectBrush(CreateSolidBrush(barPal[2])));

					memDC.Rectangle(rc.left, rc.top, rc.right, rc.bottom);

				}
				rc.left = left;
				// draw progressbar highlight
				if(rc.Width()>2) {
					DeleteObject(memDC.SelectPen(CreatePen(PS_SOLID,1,barPal[2])));

					rc.top += 2;
					memDC.MoveTo(rc.left+1,rc.top,(LPPOINT)NULL);
					memDC.LineTo(rc.right-2,rc.top);
				};
				
				// draw status text
				DeleteObject(memDC.SelectPen(oldpen));
				DeleteObject(memDC.SelectBrush(oldbr));

				int right = rc2.right;
				left = rc2.left;
				rc2.right = rc.right;
				int top = rc2.top + (rc2.Height() - WinUtil::getTextHeight(cd->nmcd.hdc) - 1)/2;
				memDC.SetBkMode(TRANSPARENT);
				memDC.SetTextColor(RGB(255, 255, 255));
				memDC.ExtTextOut(left, top, ETO_CLIPPED, rc2, buf, _tcslen(buf), NULL);
				
				rc2.left = rc2.right;
				rc2.right = right;

				memDC.SetTextColor(WinUtil::textColor);
				memDC.ExtTextOut(left, top, ETO_CLIPPED, rc2, buf, _tcslen(buf), NULL);
				
				return CDRF_SKIPDEFAULT;
			}
		}
		// Fall through
	default:
		return CDRF_DODEFAULT;
	}
}

LRESULT TransferView::onEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return TRUE;
}

LRESULT TransferView::onDoubleClickTransfers(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;
	if (item->iItem != -1 ) {
		ctrlTransfers.getItemData(item->iItem)->pm();
	}
	return 0;
}

LRESULT TransferView::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if(wParam == ADD_ITEM) {
		ItemInfo* i = (ItemInfo*)lParam;
		ctrlTransfers.insertItem(i, (i->type == ItemInfo::TYPE_DOWNLOAD) ? IMAGE_DOWNLOAD : IMAGE_UPLOAD);
	} else if(wParam == REMOVE_ITEM) {
		ItemInfo* i = (ItemInfo*)lParam;
		dcassert(ctrlTransfers.findItem(i) != -1);
		ctrlTransfers.deleteItem(i);
		delete i;
	} else if(wParam == UPDATE_ITEM) {
		ItemInfo* i = (ItemInfo*)lParam;
		i->update();
		ctrlTransfers.updateItem(i);
		if(ctrlTransfers.getSortColumn() != COLUMN_USER)
			ctrlTransfers.resort();
	} else if(wParam == UPDATE_ITEMS) {
		vector<ItemInfo*>* v = (vector<ItemInfo*>*)lParam;
		ctrlTransfers.SetRedraw(FALSE);
		for(vector<ItemInfo*>::iterator j = v->begin(); j != v->end(); ++j) {
			ItemInfo* i = *j;
			i->update();
			ctrlTransfers.updateItem(i);
		}

		if(ctrlTransfers.getSortColumn() != COLUMN_STATUS)
			ctrlTransfers.resort();
		ctrlTransfers.SetRedraw(TRUE);
		
		delete v;
	}

	return 0;
}

void TransferView::ItemInfo::update() {
	u_int32_t colMask = updateMask;
	updateMask = 0;

	if(colMask & MASK_USER) {
		columns[COLUMN_USER] = WinUtil::toT(user->getNick());
	}
	if(colMask & MASK_HUB) {
		columns[COLUMN_HUB] = WinUtil::toT(user->getClientName());
	}
	if(colMask & MASK_STATUS) {
		columns[COLUMN_STATUS] = statusString;
	}
	if(colMask & MASK_TIMELEFT) {
		if (status == STATUS_RUNNING) {
			columns[COLUMN_TIMELEFT] = WinUtil::toT(Util::formatSeconds(timeLeft));
		} else {
			columns[COLUMN_TIMELEFT] = Util::emptyStringT;
		}
	}
	if(colMask & MASK_TOTALTIMELEFT) {
		if(status == STATUS_RUNNING)
			columns[COLUMN_TOTALTIMELEFT] = Util::formatSeconds(totalTimeLeft);
		else
			columns[COLUMN_TOTALTIMELEFT] = Util::emptyString;
	}
	if(colMask & MASK_SPEED) {
		if (status == STATUS_RUNNING) {
			columns[COLUMN_SPEED] = WinUtil::toT(Util::formatBytes(speed) + "/s");
		} else {
			columns[COLUMN_SPEED] = Util::emptyStringT;
		}
	}
	if(colMask & MASK_FILE) {
		columns[COLUMN_FILE] = file;
	}
	if(colMask & MASK_SIZE) {
		columns[COLUMN_SIZE] = WinUtil::toT(Util::formatBytes(size));
	}
	if(colMask & MASK_PATH) {
		columns[COLUMN_PATH] = path;
	}
	if(colMask & MASK_IP) {
		if (country == _T("")) 
			columns[COLUMN_IP] = IP;
		else 
			columns[COLUMN_IP] = country + _T(" (") + IP + _T(")");
	}
	if(colMask & MASK_RATIO) {
		columns[COLUMN_RATIO] = WinUtil::toT(Util::toString(getRatio()));
	}
}

void TransferView::on(ConnectionManagerListener::Added, ConnectionQueueItem* aCqi) {
	ItemInfo::Types t = aCqi->getConnection() && aCqi->getConnection()->isSet(UserConnection::FLAG_UPLOAD) ? ItemInfo::TYPE_UPLOAD : ItemInfo::TYPE_DOWNLOAD;
	ItemInfo* i = new ItemInfo(aCqi->getUser(), t, ItemInfo::STATUS_WAITING);

	{
		Lock l(cs);
		dcassert(transferItems.find(aCqi) == transferItems.end());
		transferItems.insert(make_pair(aCqi, i));
		i->columns[COLUMN_STATUS] = i->statusString = TSTRING(CONNECTING);
	}

	PostMessage(WM_SPEAKER, ADD_ITEM, (LPARAM)i);
}

void TransferView::on(ConnectionManagerListener::StatusChanged, ConnectionQueueItem* aCqi) {
	ItemInfo* i;
	{
		Lock l(cs);
		dcassert(transferItems.find(aCqi) != transferItems.end());
		i = transferItems[aCqi];		
		i->statusString = aCqi->getState() == ConnectionQueueItem::CONNECTING ? TSTRING(CONNECTING) : TSTRING(WAITING_TO_RETRY);
		i->updateMask |= ItemInfo::MASK_STATUS;
	}

	PostMessage(WM_SPEAKER, UPDATE_ITEM, (LPARAM)i);
}

void TransferView::on(ConnectionManagerListener::Removed, ConnectionQueueItem* aCqi) {
	ItemInfo* i;
	{
		Lock l(cs);
		ItemInfo::MapIter ii = transferItems.find(aCqi);
		dcassert(ii != transferItems.end());
		i = ii->second;
		transferItems.erase(ii);
	}
	PostMessage(WM_SPEAKER, REMOVE_ITEM, (LPARAM)i);
}

void TransferView::on(ConnectionManagerListener::Failed, ConnectionQueueItem* aCqi, const string& aReason) {
	ItemInfo* i;
	{
		Lock l(cs);
		dcassert(transferItems.find(aCqi) != transferItems.end());
		i = transferItems[aCqi];		
		i->statusString = WinUtil::toT(aReason);
		i->updateMask |= ItemInfo::MASK_STATUS;
	}
	PostMessage(WM_SPEAKER, UPDATE_ITEM, (LPARAM)i);
}

void TransferView::on(DownloadManagerListener::Starting, Download* aDownload) {
	ConnectionQueueItem* aCqi = aDownload->getUserConnection()->getCQI();
	ItemInfo* i;
	{
		Lock l(cs);
		dcassert(transferItems.find(aCqi) != transferItems.end());
		i = transferItems[aCqi];		
		i->status = ItemInfo::STATUS_RUNNING;
		i->pos = 0;
		i->start = aDownload->getPos();
		i->actual = i->start;
		i->size = aDownload->getSize();
		i->file = WinUtil::toT(Util::getFileName(aDownload->getTarget()));
		i->path = WinUtil::toT(Util::getFilePath(aDownload->getTarget()));
		i->statusString = TSTRING(DOWNLOAD_STARTING);
		i->IP = WinUtil::toT(aDownload->getUserConnection()->getRemoteIp());
		i->country = WinUtil::toT(Util::getIpCountry(aDownload->getUserConnection()->getRemoteIp()));
		i->updateMask |= ItemInfo::MASK_STATUS | ItemInfo::MASK_FILE | ItemInfo::MASK_PATH |
			ItemInfo::MASK_SIZE | ItemInfo::MASK_IP;
	}

	PostMessage(WM_SPEAKER, UPDATE_ITEM, (LPARAM)i);
}

void TransferView::on(DownloadManagerListener::Tick, const Download::List& dl) {
	vector<ItemInfo*>* v = new vector<ItemInfo*>();
	v->reserve(dl.size());

	TCHAR* buf = new TCHAR[TSTRING(DOWNLOADED_BYTES).size() + 64];

	{
		Lock l(cs);
		for(Download::List::const_iterator j = dl.begin(); j != dl.end(); ++j) {
			Download* d = *j;

			_stprintf(buf, CTSTRING(DOWNLOADED_BYTES), WinUtil::toT(Util::formatBytes(d->getPos())).c_str(), 
				(double)d->getPos()*100.0/(double)d->getSize(), WinUtil::toT(Util::formatSeconds((GET_TICK() - d->getStart())/1000)).c_str());

			ConnectionQueueItem* aCqi = d->getUserConnection()->getCQI();
			ItemInfo* i = transferItems[aCqi];
			i->actual = i->start + d->getActual();
			i->pos = i->start + d->getTotal();
			i->timeLeft = d->getSecondsLeft();
			i->totalTimeLeft = d->getTotalSecondsLeft();
			i->speed = d->getRunningAverage();

			if(d->isSet(Download::FLAG_ZDOWNLOAD)) {
				i->statusString = _T("* ") + tstring(buf);
			} else {
				i->statusString = buf;
			}
			i->updateMask |= ItemInfo::MASK_STATUS | ItemInfo::MASK_TIMELEFT | ItemInfo::MASK_TOTALTIMELEFT | ItemInfo::MASK_SPEED | ItemInfo::MASK_RATIO;

			v->push_back(i);
		}
	}
	delete[] buf;

	PostMessage(WM_SPEAKER, UPDATE_ITEMS, (LPARAM)v);
}

void TransferView::on(DownloadManagerListener::Failed, Download* aDownload, const string& aReason) {
	ConnectionQueueItem* aCqi = aDownload->getUserConnection()->getCQI();
	ItemInfo* i;
	{
		Lock l(cs);
		dcassert(transferItems.find(aCqi) != transferItems.end());
		i = transferItems[aCqi];		
		i->status = ItemInfo::STATUS_WAITING;
		i->pos = 0;

		i->statusString = WinUtil::toT(aReason);
		i->size = aDownload->getSize();
		i->file = WinUtil::toT(Util::getFileName(aDownload->getTarget()));
		i->path = WinUtil::toT(Util::getFilePath(aDownload->getTarget()));
		i->updateMask |= ItemInfo::MASK_STATUS | ItemInfo::MASK_SIZE | ItemInfo::MASK_FILE |
		ItemInfo::MASK_PATH;
	}
	PostMessage(WM_SPEAKER, UPDATE_ITEM, (LPARAM)i);
}

void TransferView::on(UploadManagerListener::Starting, Upload* aUpload) {
	ConnectionQueueItem* aCqi = aUpload->getUserConnection()->getCQI();
	ItemInfo* i;
	{
		Lock l(cs);
		dcassert(transferItems.find(aCqi) != transferItems.end());
		i = transferItems[aCqi];		
		i->pos = 0;
		i->start = aUpload->getPos();
		i->actual = i->start;
		i->size = aUpload->getSize();
		i->status = ItemInfo::STATUS_RUNNING;
		i->speed = 0;
		i->timeLeft = 0;

		i->file = WinUtil::toT(Util::getFileName(aUpload->getFileName()));
		i->path = WinUtil::toT(Util::getFilePath(aUpload->getFileName()));
		i->statusString = TSTRING(UPLOAD_STARTING);
		i->IP = WinUtil::toT(aUpload->getUserConnection()->getRemoteIp());
		i->country = WinUtil::toT(Util::getIpCountry(aUpload->getUserConnection()->getRemoteIp()));
		i->updateMask |= ItemInfo::MASK_STATUS | ItemInfo::MASK_FILE | ItemInfo::MASK_PATH |
			ItemInfo::MASK_SIZE | ItemInfo::MASK_IP;
	}

	PostMessage(WM_SPEAKER, UPDATE_ITEM, (LPARAM)i);
}

void TransferView::on(UploadManagerListener::Tick, const Upload::List& ul) {
	vector<ItemInfo*>* v = new vector<ItemInfo*>();
	v->reserve(ul.size());

	TCHAR* buf = new TCHAR[STRING(UPLOADED_BYTES).size() + 64];

	{
		Lock l(cs);
		for(Upload::List::const_iterator j = ul.begin(); j != ul.end(); ++j) {
			Upload* u = *j;

			ConnectionQueueItem* aCqi = u->getUserConnection()->getCQI();
			ItemInfo* i = transferItems[aCqi];	
			i->actual = i->start + u->getActual();
			i->pos = i->start + u->getTotal();
			i->timeLeft = u->getSecondsLeft();
			i->speed = u->getRunningAverage();

			_stprintf(buf, CTSTRING(UPLOADED_BYTES), WinUtil::toT(Util::formatBytes(u->getPos())).c_str(), 
				(double)u->getPos()*100.0/(double)u->getSize(), WinUtil::toT(Util::formatSeconds((GET_TICK() - u->getStart())/1000)).c_str());

			if(u->isSet(Upload::FLAG_ZUPLOAD)) {
				i->statusString = _T("* ") + tstring(buf);
			} else {
				i->statusString = buf;
			}

			i->updateMask |= ItemInfo::MASK_STATUS | ItemInfo::MASK_TIMELEFT | ItemInfo::MASK_SPEED | ItemInfo::MASK_RATIO;
			v->push_back(i);
		}
	}

	delete[] buf;

	PostMessage(WM_SPEAKER, UPDATE_ITEMS, (LPARAM)v);
}

void TransferView::onTransferComplete(Transfer* aTransfer, bool isUpload) {
	ConnectionQueueItem* aCqi = aTransfer->getUserConnection()->getCQI();
	ItemInfo* i;
	{
		Lock l(cs);
		dcassert(transferItems.find(aCqi) != transferItems.end());
		i = transferItems[aCqi];		

		i->status = ItemInfo::STATUS_WAITING;
		i->pos = 0;

		i->statusString = isUpload ? TSTRING(UPLOAD_FINISHED_IDLE) : TSTRING(DOWNLOAD_FINISHED_IDLE);
		i->updateMask |= ItemInfo::MASK_STATUS;
	}
	PostMessage(WM_SPEAKER, UPDATE_ITEM, (LPARAM)i);	
}

void TransferView::ItemInfo::disconnect() {
	ConnectionManager::getInstance()->removeConnection(user, (type == TYPE_DOWNLOAD));
}

LRESULT TransferView::onPmAll(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	ItemInfo::Types t;
	if(wID == IDC_PM_UP)
		t = ItemInfo::Types::TYPE_UPLOAD;
	else
		t = ItemInfo::Types::TYPE_DOWNLOAD;

	LineDlg dlg;
	if(dlg.DoModal() == IDCANCEL)
		return 0;
	
	string msg = dlg.line;

	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_ALL)) != -1) {
		ItemInfo *ii = ctrlTransfers.getItemData(i);
		if(ii->type == t && ii->status == ItemInfo::STATUS_RUNNING)
			ii->pm(msg);
	}

	return 0;	
}

LRESULT TransferView::onOpen(WORD , WORD wID, HWND , BOOL& ) {
	ItemInfo *ii = ctrlTransfers.getItemData(ctrlTransfers.GetNextItem(-1, LVNI_SELECTED));

	string path;

	if(wID == IDC_OPEN) {
		path = ii->path + ii->file;
	} else {
		path = ii->path;
	}

	ShellExecute(NULL, "open", path.c_str(), NULL, NULL, SW_SHOW);

	return 0;
}

LRESULT TransferView::onResolveIP(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	if(resolveBuffer != NULL)
		return 0;

	int i;
	if((i = ctrlTransfers.GetNextItem(-1, LVNI_SELECTED)) == -1)
		return 0;

	resolveBuffer = new char[MAXGETHOSTSTRUCT];
	unsigned long l = inet_addr(ctrlTransfers.getItemData(i)->getText(COLUMN_IP).c_str());

	if( 0 == WSAAsyncGetHostByAddr(m_hWnd, WM_APP, (char*)&l, 4, AF_INET, resolveBuffer, MAXGETHOSTSTRUCT)) {
		delete[] resolveBuffer;
		resolveBuffer = NULL;
	}

	return 0;
}

LRESULT TransferView::onResolvedIP(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/){
	if(resolveBuffer == NULL)
		return 0;
	if(HIWORD(lParam) != 0) {
		delete[] resolveBuffer;
		resolveBuffer = NULL;
		return 0;
	}
	
	hostent *h = (hostent*)resolveBuffer;
	in_addr a;
		
	memcpy(&a.S_un.S_addr, h->h_addr_list[0], 4);
	char * c = inet_ntoa(a);
	if(c != NULL)
		::PostMessage(GetParent(), WM_SPEAKER, 5, (LPARAM)new string(string(c) + " " +  STRING(RESOLVES_TO) + " " + h->h_name));
	delete[] resolveBuffer;
	resolveBuffer = NULL;

	return 0;
}

LRESULT TransferView::onCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	ItemInfo *ii = ctrlTransfers.getSelectedItem();
	if(ii != NULL)
		WinUtil::copyToClipboard(ii->getText(wID - IDC_COPY));

	return 0;
}
/**
 * @file
 * $Id: TransferView.cpp,v 1.8 2004/02/21 10:46:10 trem Exp $
 */