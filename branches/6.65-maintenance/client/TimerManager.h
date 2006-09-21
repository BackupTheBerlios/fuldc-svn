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

#if !defined(TIMER_MANAGER_H)
#define TIMER_MANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Thread.h"
#include "Semaphore.h"
#include "Speaker.h"
#include "Singleton.h"

#if !(defined(_WIN32) || defined(_WIN64))
#include <sys/time.h>
#endif

class TimerManagerListener {
public:
	virtual ~TimerManagerListener() { }
	template<int I>	struct X { enum { TYPE = I };  };

	typedef X<0> Second;
	typedef X<1> Minute;

	// We expect everyone to implement this...
	virtual void on(Second, time_t) throw() { }
	virtual void on(Minute, time_t) throw() { }
};

class TimerManager : public Speaker<TimerManagerListener>, public Singleton<TimerManager>, public Thread
{
public:
	void shutdown() {
		s.signal();
		join();
	}

	static time_t getTime() {
		return (time_t)time(NULL);
	}
	static time_t getTick() { 
#if defined(_WIN32) || defined(_WIN64)
		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);
		//convert to millisecond resolution, don't care about nanoseconds but don't want
		//the overflow with GetTickCount
		return ( ((time_t)ft.dwHighDateTime) << 32 | (time_t)ft.dwLowDateTime ) / 10000; 
#else
		timeval tv2;
		gettimeofday(&tv2, NULL);
		return (u_int32_t)((tv2.tv_sec - tv.tv_sec) * 1000 ) + ( (tv2.tv_usec - tv.tv_usec) / 1000);
#endif
	}
private:

	Semaphore s;

	friend class Singleton<TimerManager>;
	TimerManager() { 
#if !(defined(_WIN32) || defined(_WIN64))
		gettimeofday(&tv, NULL);
#endif
	}

	virtual ~TimerManager() throw() {
		shutdown();
	}

	virtual int run();
	
#if !(defined(_WIN32) || defined(_WIN64))
	static timeval tv;
#endif
};

#define GET_TICK() TimerManager::getTick()
#define GET_TIME() TimerManager::getTime()

#endif // !defined(TIMER_MANAGER_H)
