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

#if !defined(DOWNLOAD_MANAGER_H)
#define DOWNLOAD_MANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TimerManager.h"

#include "UserConnection.h"
#include "Singleton.h"
#include "FilteredFile.h"
#include "ZUtils.h"
#include "MerkleTree.h"
#include "QueueItem.h"

class ConnectionQueueItem;

/**
 * Comes as an argument in the DownloadManagerListener functions.
 * Use it to retrieve information about the ongoing transfer.
 */
class Download : public Transfer, public Flags {
public:

	typedef Download* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	enum {
		FLAG_USER_LIST = 0x01,
		FLAG_RESUME = 0x02,
		FLAG_ZDOWNLOAD = 0x04,
		FLAG_CALC_CRC32 = 0x08,
		FLAG_CRC32_OK = 0x10,
		FLAG_ANTI_FRAG = 0x20,
		FLAG_TREE_DOWNLOAD = 0x40,
		FLAG_TREE_TRIED = 0x80,
		FLAG_TTH_CHECK = 0x100
	};

	Download(UserConnection& conn) throw();
	Download(UserConnection& conn, QueueItem& qi) throw();

	virtual void getParams(const UserConnection& aSource, StringMap& params);

	virtual ~Download();

	/** @return Target filename without path. */
	string getTargetFileName() {
		return Util::getFileName(getTarget());
	}

	/** @internal */
	string getDownloadTarget() {
		const string& tgt = (getTempTarget().empty() ? getTarget() : getTempTarget());
		return isSet(FLAG_ANTI_FRAG) ? tgt + Util::ANTI_FRAG_EXT : tgt;			
	}

	/** @internal */
	TigerTree& getTigerTree() { return tt; }

	int64_t getTotalSecondsLeft();

	/** @internal */
	AdcCommand getCommand(bool zlib);

	typedef CalcOutputStream<CRC32Filter, true> CrcOS;
	GETSET(string, source, Source);
	GETSET(string, target, Target);
	GETSET(string, tempTarget, TempTarget);
	GETSET(OutputStream*, file, File);
	GETSET(CrcOS*, crcCalc, CrcCalc);
	GETSET(bool, treeValid, TreeValid);

private:
	Download(const Download&);
	Download& operator=(const Download&);

	TigerTree tt;
};

/**
 * Use this listener interface to get progress information for downloads.
 *
 * @remarks All methods are sending a pointer to a Download but the receiver
 * (TransfersFrame) is not using any of the methods in Download, only methods
 * from its super class, Transfer. The listener functions should send Transfer
 * objects instead.
 *
 * Changing this will will cause a problem with Download::List which is used
 * in the on Tick function. One solution is reimplement on Tick to call once
 * for every Downloads, sending one Download at a time. But maybe updating the
 * GUI is not DownloadManagers problem at all???
 */
class DownloadManagerListener {
public:
	virtual ~DownloadManagerListener() { }
	template<int I>	struct X { enum { TYPE = I }; };

	typedef X<0> Complete;
	typedef X<1> Failed;
	typedef X<2> Starting;
	typedef X<3> Tick;

	/**
	 * This is the first message sent before a download starts.
	 * No other messages will be sent before.
	 */
	virtual void on(Starting, Download*) throw() { }

	/**
	 * Sent once a second if something has actually been downloaded.
	 */
	virtual void on(Tick, const Download::List&) throw() { }

	/**
	 * This is the last message sent before a download is deleted.
	 * No more messages will be sent after it.
	 */
	virtual void on(Complete, Download*) throw() { }

	/**
	 * This indicates some sort of failure with a particular download.
	 * No more messages will be sent after it.
	 *
	 * @remarks Should send an error code instead of a string and let the GUI
	 * display an error string.
	 */
	virtual void on(Failed, Download*, const string&) throw() { }
};


/**
 * Singleton. Use its listener interface to update the download list
 * in the user interface.
 */
class DownloadManager : public Speaker<DownloadManagerListener>,
	private UserConnectionListener, private TimerManagerListener,
	public Singleton<DownloadManager>
{
public:

	/** @internal */
	void addConnection(UserConnection::Ptr conn);
	void checkIdle(const User::Ptr& user);

	/** @return Running average download speed in Bytes/s */
	int64_t getRunningAverage();

	/** @return Number of downloads. */
	size_t getDownloadCount() {
		Lock l(cs);
		return downloads.size();
	}

	bool startDownload(QueueItem::Priority prio);

	int64_t getAverageSpeed(const string & path){
		size_t pos = path.rfind("\\");
		string tmp = path.substr(0, pos);
		
		return averageSpeedMap.find(tmp)->second;
	}

	uint64_t getAveragePos(const string & path) {
		size_t pos = path.rfind("\\");
		string tmp = path.substr(0, pos);

		return averagePosMap.find(tmp)->second;
	}

	/**
	 * Get the count of active downloads.
	 * @remarks Function should be renamed because it sounds like it returns a
	 * list with downloads.
	 *
	 * @return Number of active downloads.
	 */ 
	size_t getDownloads() {
		Lock l(cs);
		return downloads.size();
	}

private:
	typedef HASH_MAP< string, int64_t > StringIntMap;
	typedef StringIntMap::iterator StringIntIter;
	typedef pair< string, int64_t > StringIntPair;

	StringIntMap averageSpeedMap;
	StringIntMap averagePosMap;

	enum { MOVER_LIMIT = 10*1024*1024 };
	class FileMover : public Thread {
	public:
		FileMover() : active(false) { }
		virtual ~FileMover() { join(); }

		void moveFile(const string& source, const string& target);
		virtual int run();
	private:
		typedef pair<string, string> FilePair;
		typedef vector<FilePair> FileList;
		typedef FileList::iterator FileIter;

		bool active;

		FileList files;
		CriticalSection cs;
	} mover;

	CriticalSection cs;
	Download::List downloads;
	UserConnection::List idlers;

	void removeConnection(UserConnection::Ptr aConn);
	void removeDownload(Download* aDown);
	void fileNotAvailable(UserConnection* aSource);
	void noSlots(UserConnection* aSource);

	void moveFile(const string& source, const string&target);
	void logDownload(UserConnection* aSource, Download* d);
	uint32_t calcCrc32(const string& file) throw(FileException);
	bool checkSfv(UserConnection* aSource, Download* d, uint32_t crc);
	int64_t getResumePos(const string& file, const TigerTree& tt, int64_t startPos);

	void failDownload(UserConnection* aSource, const string& reason);

	friend class Singleton<DownloadManager>;

	DownloadManager();
	virtual ~DownloadManager() throw();

	void checkDownloads(UserConnection* aConn);
	void handleEndData(UserConnection* aSource);

	// UserConnectionListener
	virtual void on(Data, UserConnection*, const uint8_t*, size_t) throw();
	virtual void on(Error, UserConnection*, const string&) throw();
	virtual void on(Failed, UserConnection*, const string&) throw();
	virtual void on(Sending, UserConnection*, int64_t) throw();
	virtual void on(FileLength, UserConnection*, int64_t) throw();
	virtual void on(MaxedOut, UserConnection*) throw();
	virtual	void on(FileNotAvailable, UserConnection*) throw();

	virtual void on(AdcCommand::SND, UserConnection*, const AdcCommand&) throw();

	bool prepareFile(UserConnection* aSource, int64_t newSize, bool z);
	// TimerManagerListener
	virtual void on(TimerManagerListener::Second, time_t aTick) throw();
};

#endif // !defined(DOWNLOAD_MANAGER_H)
