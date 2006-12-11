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

#if !defined(FINISHED_MANAGER_H)
#define FINISHED_MANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/DownloadManager.h"
#include "../client/UploadManager.h"
#include "../client/ResourceManager.h"

#include "../client/CriticalSection.h"
#include "../client/Singleton.h"

#include "WinUtil.h"

//this is a bit ugly having the finisheditem use it's own set of columns definitions
//but it'll avoid lots of string conversion/copies when viewing the list, only a pointer
//to the list of items will be used.
class FinishedItem
{
public:
	typedef FinishedItem* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	enum {
		COLUMN_FIRST,
		COLUMN_FILE = COLUMN_FIRST,
		COLUMN_DONE,
		COLUMN_PATH,
		COLUMN_NICK,
		COLUMN_HUB,
		COLUMN_SIZE,
		COLUMN_SPEED,
		COLUMN_CRC32,
		COLUMN_TTH,
		COLUMN_LAST
	};

	FinishedItem(string const& aTarget, string const& aUser, string const& aHub, string const& aTTH,
		int64_t aSize, int64_t aChunkSize, int64_t aMSeconds, time_t aTime, 
		bool aCrc32 = false) : 
		size(aSize), chunkSize(aChunkSize),
		milliSeconds(aMSeconds), time(aTime), crc32Checked(aCrc32) 
	{ 
		columns[COLUMN_FILE]  = Text::toT(Util::getFileName(aTarget));
		columns[COLUMN_DONE]  = Text::toT(Util::formatTime("%Y-%m-%d %H:%M:%S", aTime));
		columns[COLUMN_PATH]  = Text::toT(Util::getFilePath(aTarget));
		columns[COLUMN_NICK]  = Text::toT(aUser);
		columns[COLUMN_HUB]   = Text::toT(aHub);
		columns[COLUMN_SIZE]  = Text::toT(Util::formatBytes(aSize));
		columns[COLUMN_SPEED] = Text::toT(Util::formatBytes(getAvgSpeed()) + "/s");
		columns[COLUMN_CRC32] = getCrc32Checked() ? TSTRING(YES_STR) : TSTRING(NO_STR);
		columns[COLUMN_TTH]	  = Text::toT(aTTH);
		
		//cache this, should be cheaper to do it once a file is added instead of
		//a few thousand calls once the list is created
		iconIndex = WinUtil::getIconIndex(columns[COLUMN_FILE]);
	}
	tstring columns[COLUMN_LAST];

	const tstring& getText(int col) const {
		dcassert(col >= 0 && col < COLUMN_LAST);
		return columns[col];
	}

	const tstring& copy(int col) {
		if(col >= 0 && col < COLUMN_LAST)
			return getText(col);

		return Util::emptyStringT;
	}

	static int compareItems(FinishedItem* a, FinishedItem* b, int col) {
		switch(col) {
			case COLUMN_SPEED:	return compare(a->getAvgSpeed(), b->getAvgSpeed());
			case COLUMN_SIZE:	return compare(a->getSize(), b->getSize());
			default:			return lstrcmpi(a->columns[col].c_str(), b->columns[col].c_str());
		}
	}

	int64_t getAvgSpeed() { return milliSeconds > 0 ? (chunkSize * ((int64_t)1000) / milliSeconds) : 0; };

	GETSET(int64_t, size, Size);
	GETSET(int64_t, chunkSize, ChunkSize);
	GETSET(int64_t, milliSeconds, MilliSeconds);
	GETSET(time_t, time, Time);
	GETSET(bool, crc32Checked, Crc32Checked);
	GETSET(int, iconIndex, IconIndex);

private:
	friend class FinishedManager;

};

class FinishedManagerListener {
public:
	template<int I>	struct X { enum { TYPE = I };  };

	typedef X<0> AddedUl;
	typedef X<1> AddedDl;
	typedef X<2> RemovedUl;
	typedef X<3> RemovedDl;
	typedef X<4> RemovedAllUl;
	typedef X<5> RemovedAllDl;

	virtual void on(AddedDl, FinishedItem*) throw() { }
	virtual void on(RemovedDl, FinishedItem*) throw() { }
	virtual void on(RemovedAllDl) throw() { }
	virtual void on(AddedUl, FinishedItem*) throw() { }
	virtual void on(RemovedUl, FinishedItem*) throw() { }
	virtual void on(RemovedAllUl) throw() { }

}; 

class FinishedManager : public Singleton<FinishedManager>,
	public Speaker<FinishedManagerListener>, private DownloadManagerListener, private UploadManagerListener
{
public:
	FinishedItem::List& lockList(bool upload = false) { cs.enter(); return upload ? uploads : downloads; };
	void unlockList() { cs.leave(); };

	void remove(FinishedItem *item, bool upload = false);
	void removeAll(bool upload = false);
private:
	friend class Singleton<FinishedManager>;
	
	FinishedManager() { 
		DownloadManager::getInstance()->addListener(this);
		UploadManager::getInstance()->addListener(this);
	}
	virtual ~FinishedManager() throw();

	virtual void on(DownloadManagerListener::Complete, Download* d) throw();
	virtual void on(UploadManagerListener::Complete, Upload*) throw();

	CriticalSection cs;
	FinishedItem::List downloads, uploads;
};

#endif // !defined(FINISHED_MANAGER_H)

/**
 * @file
 * $Id: FinishedManager.h,v 1.1 2003/12/15 16:51:41 trem Exp $
 */
