/*
* Copyright (C) 2006 Pär Björklund, per.bjorklund@gmail.com
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

#include "TransfersManager.h"

TransfersManager::TransfersManager() {
	ConnectionManager::getInstance()->addListener(this);
	DownloadManager::getInstance()->addListener(this);
	UploadManager::getInstance()->addListener(this);
}

TransfersManager::~TransfersManager() {
	ConnectionManager::getInstance()->removeListener(this);
	DownloadManager::getInstance()->removeListener(this);
	UploadManager::getInstance()->removeListener(this);
}

void TransfersManager::fireInitialList() {
	Lock l(cs);
	for(TransferIter i = transfers.begin(); i != transfers.end(); ++i) {
		TransferInfo* ti = new TransferInfo(*i);
		fire(TransfersManagerListener::Added(), ti);
	}
}

void TransfersManager::on(ConnectionManagerListener::Added, ConnectionQueueItem* aCqi) {
	TransferInfo ti(aCqi->getUser(), aCqi->getDownload());

	ti.status = TransferInfo::STATUS_WAITING;
	ti.columns[TransferInfo::COLUMN_STATUS] = TSTRING(CONNECTING);

	{
		Lock l(cs);
		transfers.push_back(ti);
	}

	TransferInfo* tmp = new TransferInfo(ti);
	fire(TransfersManagerListener::Added(), tmp);
}

void TransfersManager::on(ConnectionManagerListener::StatusChanged, ConnectionQueueItem* aCqi) {
	TransferInfo* ti = NULL;
	
	{
		Lock l(cs);

		TransferIter i = find_if(transfers.begin(), transfers.end(), CompareTransferInfo(aCqi->getUser(), aCqi->getDownload()));
		if(i != transfers.end()) {
			i->columns[TransferInfo::COLUMN_STATUS] = (aCqi->getState() == ConnectionQueueItem::CONNECTING) ? TSTRING(CONNECTING) : TSTRING(WAITING_TO_RETRY);
			ti = new TransferInfo(*i);
		}
	}

	if(ti != NULL)
		fire(TransfersManagerListener::Updated(), ti);
}

void TransfersManager::on(ConnectionManagerListener::Removed, ConnectionQueueItem* aCqi) {
	TransferInfo* ti = NULL;
	{
		Lock l(cs);
		TransferIter i = find_if(transfers.begin(), transfers.end(), CompareTransferInfo(aCqi->getUser(), aCqi->getDownload()));
		if(i != transfers.end()) {
			ti = new TransferInfo(*i);
			transfers.erase(i);
		}
	}

	if(ti != NULL)
		fire(TransfersManagerListener::Removed(), ti);
}

void TransfersManager::on(ConnectionManagerListener::Failed, ConnectionQueueItem* aCqi, const string& aReason) {
	TransferInfo* ti = NULL;
	{
		Lock l(cs);
		TransferIter i = find_if(transfers.begin(), transfers.end(), CompareTransferInfo(aCqi->getUser(), aCqi->getDownload()));
		if(i != transfers.end()) {
			i->columns[TransferInfo::COLUMN_STATUS] = Text::toT(aReason);
			ti = new TransferInfo(*i);
		}
	}

	if(ti != NULL)
		fire(TransfersManagerListener::Updated(), ti);
}

void TransfersManager::on(DownloadManagerListener::Starting, Download* aDownload) {
	TransferInfo* ti = NULL;
	{
		Lock l(cs);
		TransferIter i = find_if(transfers.begin(), transfers.end(), CompareTransferInfo(aDownload->getUserConnection()->getUser(), true));
		if(i != transfers.end()) {
			i->filelist = aDownload->isSet(Download::FLAG_USER_LIST);
			i->status = TransferInfo::STATUS_RUNNING;
			i->pos = aDownload->getPos();
			i->start = aDownload->getStartPos();
			i->actual = aDownload->getActual();
			i->size = aDownload->getSize();
			i->columns[TransferInfo::COLUMN_FILE] = Text::toT(aDownload->getTarget());
			i->columns[TransferInfo::COLUMN_STATUS] = TSTRING(DOWNLOAD_STARTING);

			tstring country = Text::toT(Util::getIpCountry(aDownload->getUserConnection()->getRemoteIp()));
			tstring ip = Text::toT(aDownload->getUserConnection()->getRemoteIp());
			if(country.empty()) {
				i->columns[TransferInfo::COLUMN_IP] = ip;
			} else {
				i->columns[TransferInfo::COLUMN_IP] = country + _T(" (") + ip + _T(")");
			}
			if(aDownload->isSet(Download::FLAG_TREE_DOWNLOAD)) {
				i->columns[TransferInfo::COLUMN_FILE] = _T("TTH: ") + i->columns[TransferInfo::COLUMN_FILE];
			}

			ti = new TransferInfo(*i);
		}
	}

	if(ti != NULL)
		fire(TransfersManagerListener::Updated(), ti);
}

void TransfersManager::on(DownloadManagerListener::Tick, const Download::List& dl) {
	
	AutoArray<TCHAR> buf(TSTRING(DOWNLOADED_BYTES).size() + 64);
	Lock l(cs);

	for(Download::List::const_iterator j = dl.begin(); j != dl.end(); ++j) {
		Download* d = *j;

		TransferIter i = find_if(transfers.begin(), transfers.end(), CompareTransferInfo(d->getUserConnection()->getUser(), true));
		if(i != transfers.end()) {

			i->actual = d->getActual();
			i->pos = i->start + d->getTotal();
			i->timeLeft = d->getSecondsLeft();
			i->totalTimeLeft = d->getTotalSecondsLeft();
			i->speed = d->getRunningAverage();

			_stprintf(buf, CTSTRING(DOWNLOADED_BYTES), Text::toT(Util::formatBytes(d->getPos())).c_str(), 
				(double)d->getPos()*100.0/(double)d->getSize(), Text::toT(Util::formatSeconds((GET_TICK() - d->getStart())/1000)).c_str());

			tstring statusString;

			if(d->isSet(Download::FLAG_TTH_CHECK)) {
				statusString += _T("[T]");
			}
			if(d->isSet(Download::FLAG_ZDOWNLOAD)) {
				statusString += _T("[Z]");
			} 
			if(!statusString.empty()) {
				statusString += _T(" ");
			}
			statusString += buf;
			i->columns[TransferInfo::COLUMN_STATUS] = statusString;

			//hopefully this won't cause any problems even though it's fired inside the lock
			fire(TransfersManagerListener::Updated(), new TransferInfo(*i));
		}
	}
}

void TransfersManager::on(DownloadManagerListener::Failed, Download* aDownload, const string& aReason) {
	TransferInfo* ti = NULL;
	{
		Lock l(cs);
		TransferIter i = find_if(transfers.begin(), transfers.end(), CompareTransferInfo(aDownload->getUserConnection()->getUser(), true));
		if(i != transfers.end()) {
			i->columns[TransferInfo::COLUMN_STATUS] = TransferInfo::STATUS_WAITING;
			i->pos = i->start;
			i->columns[TransferInfo::COLUMN_STATUS] = Text::toT(aReason);
			i->size = aDownload->getSize();
			i->columns[TransferInfo::COLUMN_FILE] = Text::toT(aDownload->getTarget());
			if(aDownload->isSet(Download::FLAG_TREE_DOWNLOAD)) {
				i->columns[TransferInfo::COLUMN_FILE] = _T("TTH: ") + i->columns[TransferInfo::COLUMN_FILE];
			}
			ti = new TransferInfo(*i);
		}	
	}

	if(ti != NULL)
		fire(TransfersManagerListener::Updated(), ti);
}

void TransfersManager::on(UploadManagerListener::Starting, Upload* aUpload) {
	TransferInfo* ti = NULL;
	{
		Lock l(cs);
		TransferIter i = find_if(transfers.begin(), transfers.end(), CompareTransferInfo(aUpload->getUserConnection()->getUser(), false));
		if(i != transfers.end()) {
			i->filelist = aUpload->isSet(Download::FLAG_USER_LIST);
			i->status = TransferInfo::STATUS_RUNNING;
			i->pos = aUpload->getPos();
			i->start = aUpload->getStartPos();
			i->actual = aUpload->getActual();
			i->size = aUpload->getSize();
			i->columns[TransferInfo::COLUMN_FILE] = Text::toT(aUpload->getLocalFileName());
			i->columns[TransferInfo::COLUMN_STATUS] = TSTRING(DOWNLOAD_STARTING);

			tstring country = Text::toT(Util::getIpCountry(aUpload->getUserConnection()->getRemoteIp()));
			tstring ip = Text::toT(aUpload->getUserConnection()->getRemoteIp());
			if(country.empty()) {
				i->columns[TransferInfo::COLUMN_IP] = ip;
			} else {
				i->columns[TransferInfo::COLUMN_IP] = country + _T(" (") + ip + _T(")");
			}
			if(aUpload->isSet(Download::FLAG_TREE_DOWNLOAD)) {
				i->columns[TransferInfo::COLUMN_FILE] = _T("TTH: ") + i->columns[TransferInfo::COLUMN_FILE];
			}

			ti = new TransferInfo(*i);
		}
	}

	if(ti != NULL)
		fire(TransfersManagerListener::Updated(), ti);
}

void TransfersManager::on(UploadManagerListener::Tick, const Upload::List& ul) {
	AutoArray<TCHAR> buf(TSTRING(DOWNLOADED_BYTES).size() + 64);
	Lock l(cs);

	for(Upload::List::const_iterator j = ul.begin(); j != ul.end(); ++j) {
		Upload* u = *j;

		TransferIter i = find_if(transfers.begin(), transfers.end(), CompareTransferInfo(u->getUserConnection()->getUser(), false));
		if(i != transfers.end()) {

			i->actual = u->getActual();
			i->pos = i->start + u->getTotal();
			i->timeLeft = u->getSecondsLeft();
			i->speed = u->getRunningAverage();

			_stprintf(buf, CTSTRING(DOWNLOADED_BYTES), Text::toT(Util::formatBytes(u->getPos())).c_str(), 
				(double)u->getPos()*100.0/(double)u->getSize(), Text::toT(Util::formatSeconds((GET_TICK() - u->getStart())/1000)).c_str());

			tstring statusString;

			if(u->isSet(Upload::FLAG_ZUPLOAD)) {
				statusString += _T("[Z]");
			} 
			if(!statusString.empty()) {
				statusString += _T(" ");
			}
			statusString += buf;
			i->columns[TransferInfo::COLUMN_STATUS] = statusString;

			//hopefully this won't cause any problems even though it's fired inside the lock
			fire(TransfersManagerListener::Updated(), new TransferInfo(*i));
		}
	}
}

void TransfersManager::onTransferComplete(Transfer* aTransfer, bool isDownload) {
	TransferInfo* ti = NULL;
	{
		Lock l(cs);
		TransferIter i = find_if(transfers.begin(), transfers.end(), CompareTransferInfo(aTransfer->getUserConnection()->getUser(), isDownload));
		if(i != transfers.end()) {
			i->status = TransferInfo::STATUS_WAITING;
			i->pos = 0;
			i->columns[TransferInfo::COLUMN_STATUS] = isDownload ? TSTRING(DOWNLOAD_FINISHED_IDLE) : TSTRING(UPLOAD_FINISHED_IDLE);
			ti = new TransferInfo(*i);
		}
	}

	if(ti != NULL)
		fire(TransfersManagerListener::Updated(), ti);
}

TransferInfo::TransferInfo(const User::Ptr& u, bool aDownload) : UserInfoBase(u), download(aDownload), transferFailed(false),
status(STATUS_WAITING), pos(0), size(0), start(0), actual(0), speed(0), timeLeft(0) 
{ 
	columns[TransferInfo::COLUMN_USER] = Text::toT(u->getNick());
	columns[TransferInfo::COLUMN_HUB] = Text::toT(u->getClientName());
};

void TransferInfo::disconnect() {
	ConnectionManager::getInstance()->disconnect(user, download);
}