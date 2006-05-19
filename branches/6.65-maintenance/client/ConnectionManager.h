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

#if !defined(CONNECTION_MANAGER_H)
#define CONNECTION_MANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TimerManager.h"

#include "UserConnection.h"
#include "User.h"
#include "CriticalSection.h"
#include "Singleton.h"
#include "Util.h"

#include "ConnectionManagerListener.h"

class ConnectionQueueItem {
public:
	typedef ConnectionQueueItem* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	
	enum State {
		CONNECTING,					// Recently sent request to connect
		WAITING,					// Waiting to send request to connect
		NO_DOWNLOAD_SLOTS,			// Not needed right now
		ACTIVE						// In one up/downmanager
	};

	ConnectionQueueItem(const User::Ptr& aUser, bool aDownload) : state(WAITING), lastAttempt(0), download(aDownload), user(aUser) { }
	
	User::Ptr& getUser() { return user; }
	const User::Ptr& getUser() const { return user; }
	
	GETSET(State, state, State);
	GETSET(u_int32_t, lastAttempt, LastAttempt);
	GETSET(bool, download, Download);
private:
	ConnectionQueueItem(const ConnectionQueueItem&);
	ConnectionQueueItem& operator=(const ConnectionQueueItem&);
	
	User::Ptr user;
};
// Comparing with a user...
inline bool operator==(ConnectionQueueItem::Ptr ptr, const User::Ptr& aUser) { return ptr->getUser() == aUser; }

class ConnectionManager : public Speaker<ConnectionManagerListener>, 
	public UserConnectionListener, TimerManagerListener, 
	public Singleton<ConnectionManager>
{
public:
	void nmdcConnect(const string& aServer, short aPort, const string& aMyNick, const string& hubUrl);
	
	void getDownloadConnection(const User::Ptr& aUser);

	void disconnect(const User::Ptr& aUser, int isDownload);

	void shutdown();

	/** Find a suitable port to listen on, and start doing it */
	void listen() throw(Exception);
	void disconnect() throw() {
		delete server;

		server = 0;
		port = 0;
	}

	unsigned short getPort() { return port; }
private:

	class Server : public Thread {
	public:
		Server(short port, const string& ip = "0.0.0.0");
		virtual ~Server() { die = true; join(); }
	private:
		virtual int run() throw();

		Socket sock;
		bool die;
	};

	friend class Server;

	CriticalSection cs;
	short port;

	/** All ConnectionQueueItems */
	ConnectionQueueItem::List downloads;
	ConnectionQueueItem::List uploads;

	/** All active connections */
	UserConnection::List userConnections;

	User::List checkIdle;

	StringList features;

	u_int32_t floodCounter;

	Server* server;

	bool shuttingDown;

	friend class Singleton<ConnectionManager>;
	ConnectionManager();

	virtual ~ConnectionManager() throw() { shutdown(); }
	
	UserConnection* getConnection(bool aNmdc) throw();
	void putConnection(UserConnection* aConn);

	void addUploadConnection(UserConnection* uc);
	void addDownloadConnection(UserConnection* uc);

	ConnectionQueueItem* getCQI(const User::Ptr& aUser, bool download);
	void putCQI(ConnectionQueueItem* cqi);

	void accept(const Socket& sock) throw();

	// UserConnectionListener
	virtual void on(Connected, UserConnection*) throw();
	virtual void on(Failed, UserConnection*, const string&) throw();
	virtual void on(CLock, UserConnection*, const string&, const string&) throw();
	virtual void on(Key, UserConnection*, const string&) throw();
	virtual void on(Direction, UserConnection*, const string&, const string&) throw();
	virtual void on(MyNick, UserConnection*, const string&) throw();
	virtual void on(Supports, UserConnection*, const StringList&) throw();

	// TimerManagerListener
	virtual void on(TimerManagerListener::Second, u_int32_t aTick) throw();	
	virtual void on(TimerManagerListener::Minute, u_int32_t aTick) throw();	

};

#endif // !defined(CONNECTION_MANAGER_H)

/**
 * @file
 * $Id: ConnectionManager.h,v 1.1 2003/12/15 16:51:38 trem Exp $
 */
