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

#if !defined(HASH_MANAGER_H)
#define HASH_MANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Singleton.h"
#include "MerkleTree.h"
#include "Thread.h"
#include "CriticalSection.h"
#include "Semaphore.h"
#include "TimerManager.h"
#include "Util.h"
#include "FastAlloc.h"
#include "Text.h"
#include "Streams.h"

STANDARD_EXCEPTION(HashException);
class File;
class CRC32Filter;

class HashManagerListener {
public:
	virtual ~HashManagerListener() { }
	template<int I>	struct X { enum { TYPE = I }; };

	typedef X<0> TTHDone;
	typedef X<1> Paused;
	typedef X<2> Resumed;

	virtual void on(TTHDone, const string& /* fileName */, const TTHValue& /* root */) throw() = 0;
	virtual void on(Paused) { }
	virtual void on(Resumed) { }
};

class HashLoader;
class FileException;

class HashManager : public Singleton<HashManager>, public Speaker<HashManagerListener>,
	private TimerManagerListener
{
public:

	/** We don't keep leaves for blocks smaller than this... */
	static const int64_t MIN_BLOCK_SIZE;

	HashManager(): lastSave(0) {
		TimerManager::getInstance()->addListener(this);
	}
	virtual ~HashManager() throw() {
		TimerManager::getInstance()->removeListener(this);
		hasher.join();
	}

	/**
	 * Check if the TTH tree associated with the filename is current.
	 */
	bool checkTTH(const string& aFileName, int64_t aSize, uint32_t aTimeStamp);

	void stopHashing(const string& baseDir) { hasher.stopHashing(baseDir); }
	void setPriority(Thread::Priority p) { hasher.setThreadPriority(p); }

	/** @return TTH root */
	TTHValue getTTH(const string& aFileName, int64_t aSize) throw(HashException);

	bool getTree(const TTHValue& root, TigerTree& tt);

	void addTree(const string& aFileName, uint32_t aTimeStamp, const TigerTree& tt) {
		hashDone(aFileName, aTimeStamp, tt, -1);
	}
	void addTree(const TigerTree& tree) { Lock l(cs); store.addTree(tree); }

	void getStats(string& curFile, int64_t& bytesLeft, size_t& filesLeft) {
		hasher.getStats(curFile, bytesLeft, filesLeft);
	}

	/**
	 * Rebuild hash data file
	 */
	void rebuild() { hasher.scheduleRebuild(); }

	void startup() { hasher.start(); store.load(); }

	void shutdown() { 
		hasher.shutdown();
		hasher.join();
		Lock l(cs);
		store.save();
	}

	void pause() {
		fire(HashManagerListener::Paused());
		hasher.pause();
	}

	void resume() {
		hasher.resume();
		fire(HashManagerListener::Resumed());
	}

	bool isPaused() const { return hasher.isPaused(); }

	void clear() {
		hasher.clear();
	}

private:

	class Hasher : public Thread {
	public:
		Hasher() : stop(false), running(false), paused(false), rebuild(false), currentSize(0) { }

		void hashFile(const string& fileName, int64_t size);

		void stopHashing(const string& baseDir);
		virtual int run();
		bool fastHash(const string& fname, uint8_t* buf, TigerTree& tth, int64_t size, CRC32Filter* xcrc32);
		void getStats(string& curFile, int64_t& bytesLeft, size_t& filesLeft);
		void shutdown() { stop = true; s.signal(); }
		void scheduleRebuild() { rebuild = true; s.signal(); }

		void pause() {
			paused = true;
		}

		void resume() {
			paused = false;
			p.signal();
		}

		bool isPaused() const { return paused; }

		void clear() {
			Lock l(cs);
			w.clear();
		}

	private:
		// Case-sensitive (faster), it is rather unlikely that case changes, and if it does it's harmless.
		// map because it's sorted (to avoid random hash order that would create quite strange shares while hashing)
		typedef map<string, int64_t> WorkMap;
		typedef WorkMap::iterator WorkIter;

		WorkMap w;
		CriticalSection cs;
		Semaphore s;
		Semaphore p;

		bool paused;
		bool stop;
		bool running;
		bool rebuild;
		string currentFile;
		int64_t currentSize;
	};

	friend class Hasher;

	class HashStore {
	public:
		HashStore();
		void addFile(const string& aFileName, time_t aTimeStamp, const TigerTree& tth, bool aUsed);

		void load();
		void save();

		void rebuild();

		bool checkTTH(const string& aFileName, int64_t aSize, uint32_t aTimeStamp);

		void addTree(const TigerTree& tt) throw();
		const TTHValue* getTTH(const string& aFileName);
		bool getTree(const TTHValue& root, TigerTree& tth);
		bool isDirty() { return dirty; }
	private:
		/** Root -> tree mapping info, we assume there's only one tree for each root (a collision would mean we've broken tiger...) */
		struct TreeInfo {
			TreeInfo() : size(0), index(0), blockSize(0) { }
			TreeInfo(int64_t aSize, int64_t aIndex, int64_t aBlockSize) : size(aSize), index(aIndex), blockSize(aBlockSize) { }
			TreeInfo(const TreeInfo& rhs) : size(rhs.size), index(rhs.index), blockSize(rhs.blockSize) { }
			TreeInfo& operator=(const TreeInfo& rhs) { size = rhs.size; index = rhs.index; blockSize = rhs.blockSize; return *this; }

			GETSET(int64_t, size, Size);
			GETSET(int64_t, index, Index);
			GETSET(int64_t, blockSize, BlockSize);
		};

		/** File -> root mapping info */
		struct FileInfo {
		public:
			FileInfo(const string& aFileName, const TTHValue& aRoot, time_t aTimeStamp, bool aUsed) :
				fileName(aFileName), root(aRoot), timeStamp(aTimeStamp), used(aUsed) { }

			bool operator==(const string& name) { return name == fileName; }

			GETSET(string, fileName, FileName);
			GETSET(TTHValue, root, Root);
			GETSET(time_t, timeStamp, TimeStamp);
			GETSET(bool, used, Used);
		};

		typedef vector<FileInfo> FileInfoList;
		typedef FileInfoList::iterator FileInfoIter;

		typedef HASH_MAP<string, FileInfoList> DirMap;
		typedef DirMap::iterator DirIter;

		typedef HASH_MAP_X(TTHValue, TreeInfo, TTHValue::Hash, equal_to<TTHValue>, less<TTHValue>) TreeMap;
		typedef TreeMap::iterator TreeIter;

		friend class HashLoader;

		DirMap fileIndex;
		TreeMap treeIndex;

		bool dirty;

		void createDataFile(const string& name);

		bool loadTree(File& dataFile, const TreeInfo& ti, const TTHValue& root, TigerTree& tt);
		int64_t saveTree(File& dataFile, const TigerTree& tt) throw(FileException);

		string getIndexFile() { return Util::getConfigPath() + "HashIndex.xml"; }
		string getDataFile() { return Util::getConfigPath() + "HashData.dat"; }
	};

	friend class HashLoader;

	Hasher hasher;
	HashStore store;

	CriticalSection cs;

	/** Single node tree where node = root, no storage in HashData.dat */
	static const int64_t SMALL_TREE = -1;

	time_t lastSave;

	void hashDone(const string& aFileName, time_t aTimeStamp, const TigerTree& tth, int64_t speed);
	void doRebuild() {
		Lock l(cs);
		store.rebuild();
	}
	virtual void on(TimerManagerListener::Minute, time_t) throw() {
		if(GET_TICK() - lastSave > 15*60*1000) {
			Lock l(cs);
			store.save();
			lastSave = GET_TICK();
		}
	}
};

#endif // !defined(HASH_MANAGER_H)