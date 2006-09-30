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

#if !defined(TRANSFERS_MANAGER_H)
#define TRANSFERS_MANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/DownloadManager.h"
#include "../client/UploadManager.h"
#include "../client/ResourceManager.h"
#include "../client/ConnectionManager.h"
#include "../client/ConnectionManagerListener.h"

#include "../client/CriticalSection.h"
#include "../client/Singleton.h"
#include "../client/User.h"
#include "../client/Speaker.h"

#include "WinUtil.h"
#include "UserInfoBase.h"
#include "TransferInfo.h"

class TransfersManagerListener {
public:
	virtual ~TransfersManagerListener() { }
	template<int I>	struct X { enum { TYPE = I }; };

	typedef X<0> Added;
	typedef X<1> Updated;
	typedef X<2> Removed;

	virtual void on(Added, TransferInfo*) throw() { };
	virtual void on(Updated, TransferInfo*) throw() { };
	virtual void on(Removed, TransferInfo*) throw() { };
};

class TransfersManager : 
	public Singleton<TransfersManager>,
	public Speaker<TransfersManagerListener>,
	private ConnectionManagerListener,
	private UploadManagerListener,
	private DownloadManagerListener
{
public:
	TransfersManager();
	virtual ~TransfersManager();

	class CompareTransferInfo {
	public:
		CompareTransferInfo(const User::Ptr& compareTo1, bool compareTo2) : a1(compareTo1), a2(compareTo2) { }
		bool operator()(const TransferInfo& b) { return a1 == b.user && a2 == b.download; }
	private:
		CompareTransferInfo& operator=(const CompareTransferInfo&);
		const User::Ptr& a1;
		const bool a2;
	};

	void fireInitialList();

private:
	typedef vector<TransferInfo> Transfers;
	typedef Transfers::iterator TransferIter;

	Transfers transfers;

	CriticalSection cs;

	virtual void on(ConnectionManagerListener::Added, ConnectionQueueItem* aCqi) throw();
	virtual void on(ConnectionManagerListener::Failed, ConnectionQueueItem* aCqi, const string& aReason) throw();
	virtual void on(ConnectionManagerListener::Removed, ConnectionQueueItem* aCqi) throw();
	virtual void on(ConnectionManagerListener::StatusChanged, ConnectionQueueItem* aCqi) throw();

	virtual void on(DownloadManagerListener::Complete, Download* aDownload) throw() { onTransferComplete(aDownload, true);}
	virtual void on(DownloadManagerListener::Failed, Download* aDownload, const string& aReason) throw();
	virtual void on(DownloadManagerListener::Starting, Download* aDownload) throw();
	virtual void on(DownloadManagerListener::Tick, const Download::List& aDownload) throw();

	virtual void on(UploadManagerListener::Starting, Upload* aUpload) throw();
	virtual void on(UploadManagerListener::Tick, const Upload::List& aUpload) throw();
	virtual void on(UploadManagerListener::Complete, Upload* aUpload) throw() { onTransferComplete(aUpload, false); }

	void onTransferComplete(Transfer* aTransfer, bool isDownload);
};

#endif