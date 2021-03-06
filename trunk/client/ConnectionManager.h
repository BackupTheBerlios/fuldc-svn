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

class SocketException;

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

class ExpectedMap {
public:
	void add(const string& aNick, const string& aMyNick, const string& aHubUrl) {
		Lock l(cs);
		expectedConnections.insert(make_pair(aNick, make_pair(aMyNick, aHubUrl)));
	}

	pair<string, string> remove(const string& aNick) {
		Lock l(cs);
		ExpectMap::iterator i = expectedConnections.find(aNick);

		if(i == expectedConnections.end()) 
			return make_pair(Util::emptyString, Util::emptyString);

		StringPair tmp = i->second;
		expectedConnections.erase(i);

		return tmp;
	}

private:
	/** Nick -> myNick, hubUrl for expected NMDC incoming connections */
	typedef map<string, StringPair> ExpectMap;
	ExpectMap expectedConnections;

	CriticalSection cs;
};

// Comparing with a user...
inline bool operator==(ConnectionQueueItem::Ptr ptr, const User::Ptr& aUser) { return ptr->getUser() == aUser; }

class ConnectionManager : public Speaker<ConnectionManagerListener>,
	public UserConnectionListener, TimerManagerListener,
	public Singleton<ConnectionManager>
{
public:
	void adcConnect(const OnlineUser& aUser, short aPort, const string& aToken, bool secure);

	void getDownloadConnection(const User::Ptr& aUser);

	void disconnect(const User::Ptr& aUser, int isDownload);

	void shutdown();

	/** Find a suitable port to listen on, and start doing it */
	void listen() throw(SocketException);
	void disconnect() throw();

	unsigned short getPort() { return server ? static_cast<unsigned short>(server->getPort()) : 0; }
	unsigned short getSecurePort() { return secureServer ? static_cast<unsigned short>(secureServer->getPort()) : 0; }
private:

	class Server : public Thread {
	public:
		Server(bool secure_, short port, const string& ip = "0.0.0.0");
		short getPort() { return port; }
		virtual ~Server() { die = true; join(); }
	private:
		virtual int run() throw();

		Socket sock;
		short port;
		bool secure;
		bool die;
	};

	friend class Server;

	CriticalSection cs;

	/** All ConnectionQueueItems */
	ConnectionQueueItem::List downloads;
	ConnectionQueueItem::List uploads;

	/** All active connections */
	UserConnection::List userConnections;

	User::List checkIdle;

	StringList features;
	StringList adcFeatures;

	ExpectedMap expectedConnections;

	u_int32_t floodCounter;

	Server* server;
	Server* secureServer;

	bool shuttingDown;

	friend class Singleton<ConnectionManager>;
	ConnectionManager();

	virtual ~ConnectionManager() throw() { shutdown(); }

	UserConnection* getConnection(bool aNmdc, bool secure) throw();
	void putConnection(UserConnection* aConn);

	void addUploadConnection(UserConnection* uc);
	void addDownloadConnection(UserConnection* uc);

	ConnectionQueueItem* getCQI(const User::Ptr& aUser, bool download);
	void putCQI(ConnectionQueueItem* cqi);

	void accept(const Socket& sock, bool secure) throw();

	// UserConnectionListener
	virtual void on(Connected, UserConnection*) throw();
	virtual void on(Failed, UserConnection*, const string&) throw();

	virtual void on(AdcCommand::SUP, UserConnection*, const AdcCommand&) throw();
	virtual void on(AdcCommand::INF, UserConnection*, const AdcCommand&) throw();
	virtual void on(AdcCommand::STA, UserConnection*, const AdcCommand&) throw();

	// TimerManagerListener
	virtual void on(TimerManagerListener::Second, u_int32_t aTick) throw();
	virtual void on(TimerManagerListener::Minute, u_int32_t aTick) throw();

};

#endif // !defined(CONNECTION_MANAGER_H)
