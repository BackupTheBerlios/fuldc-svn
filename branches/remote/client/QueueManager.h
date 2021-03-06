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

#if !defined(QUEUE_MANAGER_H)
#define QUEUE_MANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TimerManager.h"

#include "CriticalSection.h"
#include "Exception.h"
#include "User.h"
#include "File.h"
#include "QueueItem.h"
#include "Singleton.h"
#include "DirectoryListing.h"
#include "MerkleTree.h"

#include "QueueManagerListener.h"
#include "SearchManagerListener.h"
#include "ClientManagerListener.h"

#include "../client/pme.h"

STANDARD_EXCEPTION(QueueException);

class UserConnection;

class DirectoryItem {
public:
	typedef DirectoryItem* Ptr;
	typedef HASH_MULTIMAP<User::Ptr, Ptr, User::HashFunction> DirectoryMap;
	typedef DirectoryMap::iterator DirectoryIter;
	typedef pair<DirectoryIter, DirectoryIter> DirectoryPair;

	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	DirectoryItem() : priority(QueueItem::DEFAULT) { }
	DirectoryItem(const User::Ptr& aUser, const string& aName, const string& aTarget,
		QueueItem::Priority p) : name(aName), target(aTarget), priority(p), user(aUser) { }
	~DirectoryItem() { }

	User::Ptr& getUser() { return user; }
	void setUser(const User::Ptr& aUser) { user = aUser; }

	GETSET(string, name, Name);
	GETSET(string, target, Target);
	GETSET(QueueItem::Priority, priority, Priority);
private:
	User::Ptr user;
};

class ConnectionQueueItem;
class QueueLoader;

class QueueManager : public Singleton<QueueManager>, public Speaker<QueueManagerListener>, private TimerManagerListener,
	private SearchManagerListener, private ClientManagerListener
{
public:
	/** Add a file to the queue. */
	void add(const string& aTarget, int64_t aSize, const TTHValue& root, User::Ptr aUser, 
		int aFlags = QueueItem::FLAG_RESUME, bool addBad = true) throw(QueueException, FileException);
	/** Add a user's filelist to the queue. */
	void addList(const User::Ptr& aUser, int aFlags) throw(QueueException, FileException);
	/** Readd a source that was removed */
	void readd(const string& target, User::Ptr& aUser) throw(QueueException);

	/** Readd a source that was removed to all queueitems it was removed from */
	int readdUser(User::Ptr& aUser) throw();

	/** Add a directory to the queue (downloads filelist and matches the directory). */
	void addDirectory(const string& aDir, const User::Ptr& aUser, const string& aTarget, QueueItem::Priority p = QueueItem::DEFAULT) throw();

	int matchListing(const DirectoryListing& dl) throw();

	bool getTTH(const string& name, TTHValue& tth) throw();

	/** Move the target location of a queued item. Running items are silently ignored */
	void move(const string& aSource, const string& aTarget) throw();

	void remove(const string& aTarget) throw();
	void removeUserFromFile(const string& aTarget, User::Ptr& aUser, int reason, bool removeConn = true) throw();
	void removeUserFromQueue(User::Ptr aUser, int reason) throw();

	void setPriority(const string& aTarget, QueueItem::Priority p) throw();

	void getTargets(const TTHValue& tth, StringList& sl);
	QueueItem::StringMap& lockQueue() throw() { cs.enter(); return fileQueue.getQueue(); } ;
	void unlockQueue() throw() { cs.leave(); }

	Download* getDownload(UserConnection& aSource, bool supportsTrees) throw();
	void putDownload(Download* aDownload, bool finished) throw();

	/** @return The highest priority download the user has, PAUSED may also mean no downloads */
	QueueItem::Priority hasDownload(const User::Ptr& aUser) throw();

	int countOnlineSources(const string& aTarget);

	void loadQueue() throw();
	void saveQueue() throw();

	GETSET(time_t, lastSave, LastSave);
	GETSET(string, queueFile, QueueFile);

	void addNotification(const string& aNotify){
		notifyList.push_back(aNotify);
	}

	bool hasNotification(const string& aNotify) {
		StringIter i = notifyList.begin();
		for(; i != notifyList.end(); ++i){
			if((*i).find(aNotify) != string::npos )
				return true;
		}
		return false;
	}

	void removeNotification(const string& aNotify) {
		StringIter i = notifyList.begin();
		for(; i != notifyList.end(); ++i){
			if((*i).find(aNotify) != string::npos){
				notifyList.erase(i);
				return;
			}
		}
		
	}

	int changePriority(const string& /*search*/, int /*priority*/);
	void SearchAlternates(const string /*path*/);
	uint64_t getTotalSize(const string & path);
	int64_t getQueueSize();

private:
	//MY STUFF =)
	PME regexp;
	deque< QueueItem::Ptr > searchQueue;
	StringList notifyList;
	bool addAlternates(string, User::Ptr);
	void onTimerSearch();
	void checkNotify();

	typedef map<string, uint64_t> StringInt64Map;
	typedef StringInt64Map::iterator StringInt64Iter;
	StringInt64Map totalSizeMap;

	void updateTotalSize(const string & path, const uint64_t& size, bool add);
	
	time_t lastSearchAlternates;
	
	/** All queue items by target */
	class FileQueue {
	public:
		FileQueue() : lastInsert(queue.end()) { }
		~FileQueue() {
			for(QueueItem::StringIter i = queue.begin(); i != queue.end(); ++i)
				delete i->second;
		}
		void add(QueueItem* qi);
		QueueItem* add(const string& aTarget, int64_t aSize,
			int aFlags, QueueItem::Priority p, const string& aTempTarget, int64_t aDownloaded,
			time_t aAdded, const TTHValue& root) throw(QueueException, FileException);

		QueueItem* find(const string& target);
		void find(QueueItem::List& ql, const TTHValue& tth);

		QueueItem* findAutoSearch(StringList& recent);
		size_t getSize() { return queue.size(); }
		QueueItem::StringMap& getQueue() { return queue; }
		void move(QueueItem* qi, const string& aTarget);
		void remove(QueueItem* qi) {
			if(lastInsert != queue.end() && Util::stricmp(*lastInsert->first, qi->getTarget()) == 0)
				lastInsert = queue.end();
			queue.erase(const_cast<string*>(&qi->getTarget()));
			delete qi;
		}

	private:
		QueueItem::StringMap queue;
		/** A hint where to insert an item... */
		QueueItem::StringIter lastInsert;
	};

	/** All queue items indexed by user (this is a cache for the FileQueue really...) */
	class UserQueue {
	public:
		void add(QueueItem* qi);
		void add(QueueItem* qi, const User::Ptr& aUser);
		QueueItem* getNext(const User::Ptr& aUser, QueueItem::Priority minPrio = QueueItem::LOWEST);
		QueueItem* getRunning(const User::Ptr& aUser);
		void setRunning(QueueItem* qi, const User::Ptr& aUser);
		void setWaiting(QueueItem* qi);
		QueueItem::UserSetMap& getList(int p) { return userQueue[p]; }
		void remove(QueueItem* qi);
		void remove(QueueItem* qi, const User::Ptr& aUser);

		QueueItem::UserMap& getRunning() { return running; }
		bool isRunning(const User::Ptr& aUser) const { 
			return (running.find(aUser) != running.end());
		}
	private:
		/** QueueItems by priority and user (this is where the download order is determined) */
		QueueItem::UserSetMap userQueue[QueueItem::LAST];
		/** Currently running downloads, a QueueItem is always either here or in the userQueue */
		QueueItem::UserMap running;
	};

	friend class QueueLoader;
	friend class Singleton<QueueManager>;

	QueueManager();
	virtual ~QueueManager() throw();

	mutable CriticalSection cs;

	/** QueueItems by target */
	FileQueue fileQueue;
	/** QueueItems by user */
	UserQueue userQueue;
	/** Directories queued for downloading */
	DirectoryItem::DirectoryMap directories;
	/** Recent searches list, to avoid searching for the same thing too often */
	StringList recent;
	/** The queue needs to be saved */
	bool dirty;
	/** Next search */
	time_t nextSearch;

	/** Sanity check for the target filename */
	static string checkTarget(const string& aTarget, int64_t aSize, int& flags) throw(QueueException, FileException);
	/** Add a source to an existing queue item */
	bool addSource(QueueItem* qi, User::Ptr aUser, Flags::MaskType addBad) throw(QueueException, FileException);

	void processList(const string& name, User::Ptr& user, int flags);

	void load(const SimpleXML& aXml);

	void setDirty() {
		if(!dirty) {
			dirty = true;
			lastSave = GET_TICK();
		}
	}

	// TimerManagerListener
	virtual void on(TimerManagerListener::Second, time_t aTick) throw();
	virtual void on(TimerManagerListener::Minute, time_t aTick) throw();

	// SearchManagerListener
	virtual void on(SearchManagerListener::SR, SearchResult*) throw();

	// ClientManagerListener
	virtual void on(ClientManagerListener::UserUpdated, const User::Ptr& aUser) throw();
};

#endif // !defined(QUEUE_MANAGER_H)
