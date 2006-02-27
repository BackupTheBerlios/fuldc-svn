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

#include "../client/ResourceManager.h"
#include "../client/QueueManager.h"
#include "../client/ConnectionManager.h"
#include "../client/Socket.h"
#include "../client/IgnoreManager.h"

#include "WinUtil.h"
#include "TransferView.h"
#include "LineDlg.h"
#include "memdc.h"
#include "SearchFrm.h"
#include "MainFrm.h"

int TransferView::columnIndexes[] = { COLUMN_USER, COLUMN_HUB, COLUMN_STATUS, COLUMN_TIMELEFT, COLUMN_TOTALTIMELEFT, COLUMN_SPEED, COLUMN_FILE, COLUMN_SIZE, COLUMN_PATH, COLUMN_IP, COLUMN_RATIO };
int TransferView::columnSizes[] = { 150, 100, 250, 75, 75, 75, 175, 100, 200, 50, 75 };

static ResourceManager::Strings columnNames[] = { ResourceManager::USER, ResourceManager::HUB, ResourceManager::STATUS,
ResourceManager::TIME_LEFT, ResourceManager::TOTAL_TIME_LEFT, ResourceManager::SPEED, ResourceManager::FILENAME, ResourceManager::SIZE, ResourceManager::PATH,
ResourceManager::IP_BARE, ResourceManager::RATIO};

TransferView::~TransferView() {
	arrows.Destroy();
}

LRESULT TransferView::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	arrows.CreateFromImage(WinUtil::getIconPath(_T("arrows.bmp")).c_str(), 16, 2, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_SHARED | LR_LOADFROMFILE);
	ctrlTransfers.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_HSCROLL | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, WS_EX_CLIENTEDGE, IDC_TRANSFERS);
	ctrlTransfers.SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);

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
	ctrlTransfers.setSortColumn(COLUMN_USER);

	openMenu.CreatePopupMenu();
	openMenu.AppendMenu(MF_STRING, IDC_OPEN, CTSTRING(OPEN));
	openMenu.AppendMenu(MF_STRING, IDC_OPEN_FOLDER, CTSTRING(OPEN_FOLDER));

	pmMenu.CreatePopupMenu();
	pmMenu.AppendMenu(MF_STRING, IDC_PRIVATEMESSAGE, CTSTRING(USER));
	pmMenu.AppendMenu(MF_STRING, IDC_PM_UP, CTSTRING(ALL_UPLOADS));
	pmMenu.AppendMenu(MF_STRING, IDC_PM_DOWN, CTSTRING(ALL_DOWNLOADS));
	pmMenu.SetMenuDefaultItem(IDC_PRIVATEMESSAGE);

	copyMenu.CreatePopupMenu();
	ctrlTransfers.buildCopyMenu(copyMenu);

	userMenu.CreatePopupMenu();
	userMenu.AppendMenu(MF_STRING, IDC_GETLIST, CTSTRING(GET_FILE_LIST));
	userMenu.AppendMenu(MF_STRING, IDC_BROWSELIST, CTSTRING(BROWSE_FILE_LIST));
	userMenu.AppendMenu(MF_POPUP, (UINT_PTR)(HMENU)pmMenu, CTSTRING(SEND_PRIVATE_MESSAGE));
	userMenu.AppendMenu(MF_STRING, IDC_MATCH_QUEUE, CTSTRING(MATCH_QUEUE));
	userMenu.AppendMenu(MF_STRING, IDC_ADD_TO_FAVORITES, CTSTRING(ADD_TO_FAVORITES));
	userMenu.AppendMenu(MF_STRING, IDC_GRANTSLOT, CTSTRING(GRANT_EXTRA_SLOT));
	userMenu.AppendMenu(MF_STRING, IDC_REMOVEALL, CTSTRING(REMOVE_FROM_ALL));
	userMenu.AppendMenu(MF_STRING, IDC_SHOWLOG, CTSTRING(SHOW_LOG));
	userMenu.AppendMenu(MF_SEPARATOR);
	userMenu.AppendMenu(MF_STRING, IDC_IGNORE, CTSTRING(IGNOREA));
	userMenu.AppendMenu(MF_STRING, IDC_UNIGNORE, CTSTRING(UNIGNORE));

	transferMenu.CreatePopupMenu();
	transferMenu.AppendMenu(MF_POPUP, (HMENU)userMenu, CTSTRING(USER));
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

	ConnectionManager::getInstance()->addListener(this);
	DownloadManager::getInstance()->addListener(this);
	UploadManager::getInstance()->addListener(this);
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

LRESULT TransferView::onContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (reinterpret_cast<HWND>(wParam) == ctrlTransfers && ctrlTransfers.GetSelectedCount() > 0) { 
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		
		if(pt.x == -1 && pt.y == -1) {
			WinUtil::getContextMenuPos(ctrlTransfers, pt);
		}

		int i = -1;
		ItemInfo* itemI;
		bool bCustomMenu = false;
		if( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
			itemI = ctrlTransfers.getItemData(i);
			bCustomMenu = true;

			prepareMenu(userMenu, UserCommand::CONTEXT_CHAT, Text::toT(itemI->user->getClientAddressPort()), itemI->user->isClientOp());
			//userMenu.AppendMenu(MF_SEPARATOR);
		}

		if(ctrlTransfers.GetSelectedCount() == 1) {
			int pos = ctrlTransfers.GetNextItem(-1, LVNI_SELECTED);
			if(pos != -1) {
				ItemInfo* ii = (ItemInfo*)ctrlTransfers.getItemData(pos);
				if(IgnoreManager::getInstance()->isUserIgnored(ii->user->getNick())) {
					userMenu.EnableMenuItem(IDC_IGNORE, MF_GRAYED);
					userMenu.EnableMenuItem(IDC_UNIGNORE, MF_ENABLED);
				} else {
					userMenu.EnableMenuItem(IDC_IGNORE, MF_ENABLED);
					userMenu.EnableMenuItem(IDC_UNIGNORE, MF_GRAYED);
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
			userMenu.EnableMenuItem(IDC_IGNORE, MF_ENABLED);
			userMenu.EnableMenuItem(IDC_UNIGNORE, MF_ENABLED);
		}

		checkAdcItems(transferMenu);
		transferMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);

		if ( bCustomMenu ) {
			//userMenu.DeleteMenu(userMenu.GetMenuItemCount()-1, MF_BYPOSITION);
			cleanMenu(userMenu);
		}
		return TRUE; 
	}
	bHandled = FALSE;
	return FALSE; 
}

void TransferView::runUserCommand(UserCommand& uc) {
	StringMap ucParams;
	if(!WinUtil::getUCParams(m_hWnd, uc, ucParams))
		return;

	int i = -1;
	while((i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		ItemInfo* itemI = ctrlTransfers.getItemData(i);
		if(!itemI->user->isOnline())
			continue;

		ucParams["mynick"] = itemI->user->getClientNick();
		ucParams["mycid"] = itemI->user->getClientCID().toBase32();
		ucParams["file"] = Text::fromT(itemI->getText(COLUMN_PATH)) + Text::fromT(itemI->getText(COLUMN_FILE));

		StringMap tmp = ucParams;
		itemI->user->getParams(tmp);
		itemI->user->clientEscapeParams(tmp);
		itemI->user->sendUserCmd(Util::formatParams(uc.getCommand(), tmp));
	}
	return;
};


LRESULT TransferView::onForce(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		ctrlTransfers.SetItemText(i, COLUMN_STATUS, CTSTRING(CONNECTING_FORCED));
		((ItemInfo*)ctrlTransfers.getItemData(i))->user->connect();
	}
	return 0;
}

LRESULT TransferView::onCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled) {
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
			ItemInfo* ii = reinterpret_cast<ItemInfo*>(cd->nmcd.lItemlParam);
			if(ii->status == ItemInfo::STATUS_RUNNING) {
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

LRESULT TransferView::onDoubleClickTransfers(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* item = (NMITEMACTIVATE*)pnmh;
	if (item->iItem != -1 ) {
		ctrlTransfers.getItemData(item->iItem)->pm();
	}
	return 0;
}

LRESULT TransferView::onSpeaker(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if(wParam == ADD_ITEM) {
		auto_ptr<UpdateInfo> ui(reinterpret_cast<UpdateInfo*>(lParam));
		ItemInfo* ii = new ItemInfo(ui->user, ui->download);
		ii->update(*ui);
		ctrlTransfers.insertItem(ii, ii->download ? IMAGE_DOWNLOAD : IMAGE_UPLOAD);
	} else if(wParam == REMOVE_ITEM) {
		auto_ptr<UpdateInfo> ui(reinterpret_cast<UpdateInfo*>(lParam));
		int ic = ctrlTransfers.GetItemCount(); 
		for(int i = 0; i < ic; ++i) {
			ItemInfo* ii = ctrlTransfers.getItemData(i);
			if(*ui == *ii) {
				ctrlTransfers.DeleteItem(i);
				delete ii;
				break;
			}
		}
	} else if(wParam == UPDATE_ITEM) {
		auto_ptr<UpdateInfo> ui(reinterpret_cast<UpdateInfo*>(lParam));
		int ic = ctrlTransfers.GetItemCount(); 
		for(int i = 0; i < ic; ++i) {
			ItemInfo* ii = ctrlTransfers.getItemData(i);
			if(ii->download == ui->download && ii->user == ui->user) {
				ii->update(*ui);
				ctrlTransfers.updateItem(i);
				break;
			}
		}
	} else if(wParam == UPDATE_ITEMS) {
		auto_ptr<vector<UpdateInfo*> > v(reinterpret_cast<vector<UpdateInfo*>* >(lParam));
		ctrlTransfers.SetRedraw(FALSE);
		int ic = ctrlTransfers.GetItemCount(); 
		for(int i = 0; i < ic; ++i) {
			ItemInfo* ii = ctrlTransfers.getItemData(i);
			for(vector<UpdateInfo*>::iterator j = v->begin(); j != v->end(); ++j) {
				UpdateInfo* ui = *j;
				if(*ui == *ii) {
					ii->update(*ui);
					ctrlTransfers.updateItem(i);
				}
			}
		}

		if(ctrlTransfers.getSortColumn() != COLUMN_STATUS)
			ctrlTransfers.resort();
		ctrlTransfers.SetRedraw(TRUE);
		
		for_each(v->begin(), v->end(), DeleteFunction());
	}

	return 0;
}

TransferView::ItemInfo::ItemInfo(const User::Ptr& u, bool aDownload) : UserInfoBase(u), download(aDownload), transferFailed(false),
	status(STATUS_WAITING), pos(0), size(0), start(0), actual(0), speed(0), timeLeft(0) 
{ 
	columns[COLUMN_USER] = Text::toT(u->getNick());
	columns[COLUMN_HUB] = Text::toT(u->getClientName());
};

void TransferView::ItemInfo::update(const UpdateInfo& ui) {
	
	if(ui.updateMask & UpdateInfo::MASK_FILE_LIST) {
		filelist = ui.filelist;
	}
	if(ui.updateMask & UpdateInfo::MASK_STATUS) {
		status = ui.status;
	}
	if(ui.updateMask & UpdateInfo::MASK_STATUS_STRING) {
		// No slots etc from transfermanager better than disconnected from connectionmanager
		if(!transferFailed)
			columns[COLUMN_STATUS] = ui.statusString;
		transferFailed = ui.transferFailed;
	}
	if(ui.updateMask & UpdateInfo::MASK_SIZE) {
		size = ui.size;
		columns[COLUMN_SIZE] = Text::toT(Util::formatBytes(size));
	}
	if(ui.updateMask & UpdateInfo::MASK_START) {
		start = ui.start;
	}
	if(ui.updateMask & UpdateInfo::MASK_POS) {
		pos = start + ui.pos;
	}
	if(ui.updateMask & UpdateInfo::MASK_ACTUAL) {
		actual = start + ui.actual;
		columns[COLUMN_RATIO] = Text::toT(Util::toString(getRatio()));
	}
	if(ui.updateMask & UpdateInfo::MASK_TOTALTIMELEFT) {
		if(status == STATUS_RUNNING)
			columns[COLUMN_TOTALTIMELEFT] = Util::formatSecondsW(ui.totalTimeLeft);
		else
			columns[COLUMN_TOTALTIMELEFT] = Util::emptyStringT;
	}

	if(ui.updateMask & UpdateInfo::MASK_SPEED) {
		speed = ui.speed;
		if (status == STATUS_RUNNING) {
			columns[COLUMN_SPEED] = Text::toT(Util::formatBytes(speed) + "/s");
		} else {
			columns[COLUMN_SPEED] = Util::emptyStringT;
		}
	}
	if(ui.updateMask & UpdateInfo::MASK_FILE) {
		columns[COLUMN_FILE] = ui.file;
		columns[COLUMN_PATH] = ui.path;
	}
	if(ui.updateMask & UpdateInfo::MASK_TIMELEFT) {
		timeLeft = ui.timeLeft;
		if (status == STATUS_RUNNING) {
			columns[COLUMN_TIMELEFT] = Text::toT(Util::formatSeconds(timeLeft));
		} else {
			columns[COLUMN_TIMELEFT] = Util::emptyStringT;
		}
	}
	if(ui.updateMask & UpdateInfo::MASK_IP) {
		columns[COLUMN_IP] = ui.IP;
	}
}

void TransferView::on(ConnectionManagerListener::Added, ConnectionQueueItem* aCqi) {
	UpdateInfo* ui = new UpdateInfo(aCqi->getUser(), aCqi->getDownload());

	ui->setStatus(ItemInfo::STATUS_WAITING);
	ui->setStatusString(TSTRING(CONNECTING));

	speak(ADD_ITEM, ui);
}

void TransferView::on(ConnectionManagerListener::StatusChanged, ConnectionQueueItem* aCqi) {
	UpdateInfo* ui = new UpdateInfo(aCqi->getUser(), aCqi->getDownload());

	ui->setStatusString((aCqi->getState() == ConnectionQueueItem::CONNECTING) ? TSTRING(CONNECTING) : TSTRING(WAITING_TO_RETRY));

	speak(UPDATE_ITEM, ui);
}

void TransferView::on(ConnectionManagerListener::Removed, ConnectionQueueItem* aCqi) {
	speak(REMOVE_ITEM, new UpdateInfo(aCqi->getUser(), aCqi->getDownload()));
}

void TransferView::on(ConnectionManagerListener::Failed, ConnectionQueueItem* aCqi, const string& aReason) {
	UpdateInfo* ui = new UpdateInfo(aCqi->getUser(), aCqi->getDownload());
	ui->setStatusString(Text::toT(aReason));
	speak(UPDATE_ITEM, ui);
}

void TransferView::on(DownloadManagerListener::Starting, Download* aDownload) {
	UpdateInfo* ui = new UpdateInfo(aDownload->getUserConnection()->getUser(), true);
	ui->setFileList(aDownload->isSet(Download::FLAG_USER_LIST));
	ui->setStatus(ItemInfo::STATUS_RUNNING);
	ui->setPos(aDownload->getTotal());
	ui->setActual(aDownload->getActual());
	ui->setStart(aDownload->getPos());
	ui->setSize(aDownload->getSize());
	ui->setFile(Text::toT(aDownload->getTarget()));
	ui->setStatusString(TSTRING(DOWNLOAD_STARTING));
	tstring country = Text::toT(Util::getIpCountry(aDownload->getUserConnection()->getRemoteIp()));
	tstring ip = Text::toT(aDownload->getUserConnection()->getRemoteIp());
	if(country.empty()) {
		ui->setIP(ip);
	} else {
		ui->setIP(country + _T(" (") + ip + _T(")"));
	}
	if(aDownload->isSet(Download::FLAG_TREE_DOWNLOAD)) {
		ui->file = _T("TTH: ") + ui->file;
	}

	speak(UPDATE_ITEM, ui);
}

void TransferView::on(DownloadManagerListener::Tick, const Download::List& dl) {
	vector<UpdateInfo*>* v = new vector<UpdateInfo*>();
	v->reserve(dl.size());

	AutoArray<TCHAR> buf(TSTRING(DOWNLOADED_BYTES).size() + 64);

	for(Download::List::const_iterator j = dl.begin(); j != dl.end(); ++j) {
		Download* d = *j;

		UpdateInfo* ui = new UpdateInfo(d->getUserConnection()->getUser(), true);
		ui->setActual(d->getActual());
		ui->setPos(d->getTotal());
		ui->setTimeLeft(d->getSecondsLeft());
		ui->setTotalTimeLeft(d->getTotalSecondsLeft());
		ui->setSpeed(d->getRunningAverage());

		_stprintf(buf, CTSTRING(DOWNLOADED_BYTES), Text::toT(Util::formatBytes(d->getPos())).c_str(), 
			(double)d->getPos()*100.0/(double)d->getSize(), Text::toT(Util::formatSeconds((GET_TICK() - d->getStart())/1000)).c_str());

		tstring statusString;

		if(d->getUserConnection()->isSecure()) {
			statusString += _T("[S]");
		}
		if(d->isSet(Download::FLAG_TTH_CHECK)) {
			statusString += _T("[T]");
		}
		if(d->isSet(Download::FLAG_ZDOWNLOAD)) {
			statusString += _T("[Z]");
		} 
		if(d->isSet(Download::FLAG_ROLLBACK)) {
			statusString += _T("[R]");
		}
		if(!statusString.empty()) {
			statusString += _T(" ");
		}
		statusString += buf;
		ui->setStatusString(statusString);

		v->push_back(ui);
	}

	speak(UPDATE_ITEMS, v);
}

void TransferView::on(DownloadManagerListener::Failed, Download* aDownload, const string& aReason) {
	UpdateInfo* ui = new UpdateInfo(aDownload->getUserConnection()->getUser(), true, true);
	ui->setStatus(ItemInfo::STATUS_WAITING);
	ui->setPos(0);
	ui->setStatusString(Text::toT(aReason));
	ui->setSize(aDownload->getSize());
	ui->setFile(Text::toT(aDownload->getTarget()));
	if(aDownload->isSet(Download::FLAG_TREE_DOWNLOAD)) {
		ui->file = _T("TTH: ") + ui->file;
	}

	speak(UPDATE_ITEM, ui);
}

void TransferView::on(UploadManagerListener::Starting, Upload* aUpload) {
	UpdateInfo* ui = new UpdateInfo(aUpload->getUserConnection()->getUser(), false);

	ui->setStatus(ItemInfo::STATUS_RUNNING);
	ui->setPos(aUpload->getTotal());
	ui->setActual(aUpload->getActual());
	ui->setStart(aUpload->getPos());
	ui->setSize(aUpload->getSize());
	ui->setFile(Text::toT(aUpload->getFileName()));
	ui->setStatusString(TSTRING(UPLOAD_STARTING));
	tstring country = Text::toT(Util::getIpCountry(aUpload->getUserConnection()->getRemoteIp()));
	tstring ip = Text::toT(aUpload->getUserConnection()->getRemoteIp());
	if(country.empty()) {
		ui->setIP(ip);
	} else {
		ui->setIP(country + _T(" (") + ip + _T(")"));
	}
	if(aUpload->isSet(Download::FLAG_TREE_DOWNLOAD)) {
		ui->file = _T("TTH: ") + ui->file;
	}

	speak(UPDATE_ITEM, ui);
}

void TransferView::on(UploadManagerListener::Tick, const Upload::List& ul) {
	vector<UpdateInfo*>* v = new vector<UpdateInfo*>();
	v->reserve(ul.size());

	AutoArray<TCHAR> buf(TSTRING(UPLOADED_BYTES).size() + 64);

	for(Upload::List::const_iterator j = ul.begin(); j != ul.end(); ++j) {
		Upload* u = *j;

		UpdateInfo* ui = new UpdateInfo(u->getUserConnection()->getUser(), false);
		ui->setActual(u->getActual());
		ui->setPos(u->getTotal());
		ui->setTimeLeft(u->getSecondsLeft());
		ui->setSpeed(u->getRunningAverage());

		_stprintf(buf, CTSTRING(UPLOADED_BYTES), Text::toT(Util::formatBytes(u->getPos())).c_str(), 
			(double)u->getPos()*100.0/(double)u->getSize(), Text::toT(Util::formatSeconds((GET_TICK() - u->getStart())/1000)).c_str());

		tstring statusString;
		
		if(u->getUserConnection()->isSecure()) {
			statusString += _T("[S]");
		}
		if(u->isSet(Upload::FLAG_ZUPLOAD)) {
			statusString += _T("[Z]");
		}
		if(!statusString.empty()) {
			statusString += _T(" ");
		}
		statusString += buf;

		ui->setStatusString(statusString);

		v->push_back(ui);
	}

	speak(UPDATE_ITEMS, v);
}

void TransferView::onTransferComplete(Transfer* aTransfer, bool isUpload) {
	UpdateInfo* ui = new UpdateInfo(aTransfer->getUserConnection()->getUser(), !isUpload);

	ui->setStatus(ItemInfo::STATUS_WAITING);
	ui->setPos(0);
	ui->setStatusString(isUpload ? TSTRING(UPLOAD_FINISHED_IDLE) : TSTRING(DOWNLOAD_FINISHED_IDLE));
	
	speak(UPDATE_ITEM, ui);
}

void TransferView::ItemInfo::disconnect() {
	ConnectionManager::getInstance()->disconnect(user, download);
}

LRESULT TransferView::onOpen(WORD , WORD wID, HWND , BOOL& ) {
	ItemInfo *ii = ctrlTransfers.getItemData(ctrlTransfers.GetNextItem(-1, LVNI_SELECTED));

	tstring path;

	if(wID == IDC_OPEN) {
		path = ii->getText(COLUMN_PATH) + ii->getText(COLUMN_FILE);
	} else {
		path = ii->getText(COLUMN_PATH);
	}

	ShellExecute(NULL, _T("open"), path.c_str(), NULL, NULL, SW_SHOW);

	return 0;
}

LRESULT TransferView::onCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	ctrlTransfers.copy(copyMenu, wID - IDC_COPY);

	return 0;
}

LRESULT TransferView::onRemoveFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_SELECTED)) != -1) {
		ItemInfo *ii = ctrlTransfers.getItemData(i);
		QueueManager::getInstance()->remove(Text::fromT(ii->getText(COLUMN_PATH) + ii->getText(COLUMN_FILE)));
	}

	return 0;
}

LRESULT TransferView::onSearchAlternates(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = ctrlTransfers.GetNextItem(-1, LVNI_SELECTED);

	if(i != -1) {
		ItemInfo *ii = ctrlTransfers.getItemData(i);

		QueueItem::StringMap queue = QueueManager::getInstance()->lockQueue();

		string tmp = Text::fromT(ii->getText(COLUMN_PATH) + ii->getText(COLUMN_FILE));
		QueueItem::StringIter qi = queue.find(&tmp);

		//create a copy of the tth to avoid holding the filequeue lock while calling
		//into searchframe, searchmanager and all of that
		TTHValue *val = NULL;
		if(qi != queue.end() && qi->second->getTTH()) {
			val = new TTHValue(*qi->second->getTTH());
		}

		QueueManager::getInstance()->unlockQueue();

		if(val) {
			WinUtil::searchHash(val);
			delete val;
		} 
	}

	return 0;
}

LRESULT TransferView::onPmAll(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	bool download = wID == IDC_PM_DOWN;

	LineDlg dlg;
	if(dlg.DoModal() == IDCANCEL)
		return 0;

	tstring msg = dlg.line;

	int i = -1;
	while( (i = ctrlTransfers.GetNextItem(i, LVNI_ALL)) != -1) {
		ItemInfo *ii = ctrlTransfers.getItemData(i);
		if(ii->download == download && ii->status == ItemInfo::STATUS_RUNNING)
			ii->pm(msg);
	}

	return 0;	
}
/**
 * @file
 * $Id: TransferView.cpp,v 1.8 2004/02/21 10:46:10 trem Exp $
 */