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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "ConnectionManager.h"

#include "ResourceManager.h"
#include "DownloadManager.h"
#include "UploadManager.h"
#include "CryptoManager.h"
#include "ClientManager.h"
#include "QueueManager.h"
#include "LogManager.h"

#include "UserConnection.h"

ConnectionManager::ConnectionManager() : floodCounter(0), server(0), shuttingDown(false) {
	TimerManager::getInstance()->addListener(this);

	features.push_back(UserConnection::FEATURE_MINISLOTS);
	features.push_back(UserConnection::FEATURE_XML_BZLIST);
	features.push_back(UserConnection::FEATURE_ADCGET);
	features.push_back(UserConnection::FEATURE_TTHL);
	features.push_back(UserConnection::FEATURE_TTHF);

}
// @todo clean this up
void ConnectionManager::listen() throw(Exception){
	disconnect();
	unsigned short port = static_cast<unsigned short>(SETTING(TCP_PORT));

	server = new Server(port, SETTING(BIND_ADDRESS));
}

/**
 * Request a connection for downloading.
 * DownloadManager::addConnection will be called as soon as the connection is ready
 * for downloading.
 * @param aUser The user to connect to.
 */
void ConnectionManager::getDownloadConnection(const User::Ptr& aUser) {
	dcassert((bool)aUser);
	{
		Lock l(cs);
		ConnectionQueueItem::Iter i = find(downloads.begin(), downloads.end(), aUser);
		if(i == downloads.end()) {
			getCQI(aUser, true);
		} else {
			if(find(checkIdle.begin(), checkIdle.end(), aUser) == checkIdle.end())
				checkIdle.push_back(aUser);
		}
	}
}

ConnectionQueueItem* ConnectionManager::getCQI(const User::Ptr& aUser, bool download) {
	ConnectionQueueItem* cqi = new ConnectionQueueItem(aUser, download);
	if(download) {
		dcassert(find(downloads.begin(), downloads.end(), aUser) == downloads.end());
		downloads.push_back(cqi);
	} else {
		dcassert(find(uploads.begin(), uploads.end(), aUser) == uploads.end());
		uploads.push_back(cqi);
	}

	fire(ConnectionManagerListener::Added(), cqi);
	return cqi;
}

void ConnectionManager::putCQI(ConnectionQueueItem* cqi) {
	fire(ConnectionManagerListener::Removed(), cqi);
	if(cqi->getDownload()) {
		dcassert(find(downloads.begin(), downloads.end(), cqi) != downloads.end());
		downloads.erase(remove(downloads.begin(), downloads.end(), cqi), downloads.end());
	} else {
		dcassert(find(uploads.begin(), uploads.end(), cqi) != uploads.end());
		uploads.erase(remove(uploads.begin(), uploads.end(), cqi), uploads.end());
	}
	delete cqi;
}

UserConnection* ConnectionManager::getConnection(bool aNmdc) throw() {
	UserConnection* uc = new UserConnection();
	uc->addListener(this);
	{
		Lock l(cs);
		userConnections.push_back(uc);
	}
	if(aNmdc)
		uc->setFlag(UserConnection::FLAG_NMDC);
	return uc;
}

void ConnectionManager::putConnection(UserConnection* aConn) {
	aConn->removeListener(this);
	aConn->disconnect();

	Lock l(cs);
	userConnections.erase(remove(userConnections.begin(), userConnections.end(), aConn), userConnections.end());
}

void ConnectionManager::on(TimerManagerListener::Second, time_t aTick) throw() {
	User::List passiveUsers;
	ConnectionQueueItem::List removed;
	User::List idlers;

	{
		Lock l(cs);

		bool attemptDone = false;

		idlers = checkIdle;
		checkIdle.clear();

		for(ConnectionQueueItem::Iter i = downloads.begin(); i != downloads.end(); ++i) {
			ConnectionQueueItem* cqi = *i;

			if(cqi->getState() != ConnectionQueueItem::ACTIVE) {
				if(!cqi->getUser()->isOnline()) {
					// Not online anymore...remove it from the pending...
					removed.push_back(cqi);
					continue;
				}

				if(cqi->getUser()->isSet(User::PASSIVE) && !ClientManager::getInstance()->isActive()) {
					passiveUsers.push_back(cqi->getUser());
					removed.push_back(cqi);
					continue;
				}

				if( ((cqi->getLastAttempt() + 60*1000) < aTick) && !attemptDone ) {
					cqi->setLastAttempt(aTick);

					QueueItem::Priority prio = QueueManager::getInstance()->hasDownload(cqi->getUser());

					if(prio == QueueItem::PAUSED) {
						removed.push_back(cqi);
						continue;
					}

					bool startDown = DownloadManager::getInstance()->startDownload(prio);
                    
					if(cqi->getState() == ConnectionQueueItem::WAITING) {
						if(startDown) {
							cqi->setState(ConnectionQueueItem::CONNECTING);
							cqi->getUser()->connect();
							fire(ConnectionManagerListener::StatusChanged(), cqi);
							attemptDone = true;
						} else {
							cqi->setState(ConnectionQueueItem::NO_DOWNLOAD_SLOTS);
							fire(ConnectionManagerListener::Failed(), cqi, STRING(ALL_DOWNLOAD_SLOTS_TAKEN));
						}
					} else if(cqi->getState() == ConnectionQueueItem::NO_DOWNLOAD_SLOTS && startDown) {
						cqi->setState(ConnectionQueueItem::WAITING);
					}
				} else if(((cqi->getLastAttempt() + 50*1000) < aTick) && (cqi->getState() == ConnectionQueueItem::CONNECTING)) {
					fire(ConnectionManagerListener::Failed(), cqi, STRING(CONNECTION_TIMEOUT));
					cqi->setState(ConnectionQueueItem::WAITING);
				}
			}
		}

		for(ConnectionQueueItem::Iter m = removed.begin(); m != removed.end(); ++m) {
			putCQI(*m);
		}

	}

	for(User::Iter i = idlers.begin(); i != idlers.end(); ++i) {
		DownloadManager::getInstance()->checkIdle(*i);
	}

	for(User::Iter ui = passiveUsers.begin(); ui != passiveUsers.end(); ++ui) {
		QueueManager::getInstance()->removeUserFromQueue(*ui, QueueItem::Source::FLAG_PASSIVE);
	}
}

void ConnectionManager::on(TimerManagerListener::Minute, time_t aTick) throw() {
	Lock l(cs);

	for(UserConnection::Iter j = userConnections.begin(); j != userConnections.end(); ++j) {
		if(((*j)->getLastActivity() + 180*1000) < aTick) {
			(*j)->disconnect(true);
		}
	}
}

static const uint32_t FLOOD_TRIGGER = 20000;
static const uint32_t FLOOD_ADD = 2000;

ConnectionManager::Server::Server(short port, const string& ip /* = "0.0.0.0" */) : die(false) {
	sock.create();
	sock.bind(port, ip);
	sock.listen();

	start();
}

static const uint32_t POLL_TIMEOUT = 250;

int ConnectionManager::Server::run() throw() {
	try {
		while(!die) {
			if(sock.wait(POLL_TIMEOUT, Socket::WAIT_READ) == Socket::WAIT_READ) {
				ConnectionManager::getInstance()->accept(sock);
			}
		}
	} catch(const Exception& e) {
		LogManager::getInstance()->message(STRING(LISTENER_FAILED) + e.getError());
	}
	return 0;
}

/**
 * Someone's connecting, accept the connection and wait for identification...
 * It's always the other fellow that starts sending if he made the connection.
 */
void ConnectionManager::accept(const Socket& sock) throw() {
	time_t now = GET_TICK();

	if(now > floodCounter) {
		floodCounter = now + FLOOD_ADD;
	} else {
		if(false && now + FLOOD_TRIGGER < floodCounter) {
			Socket s;
			try {
				s.accept(sock);
			} catch(const SocketException&) {
				// ...
			}
			dcdebug("Connection flood detected!\n");
			return;
		} else {
			floodCounter += FLOOD_ADD;
		}
	}
	UserConnection* uc = getConnection(false);
	uc->setFlag(UserConnection::FLAG_INCOMING);
	uc->setState(UserConnection::STATE_SUPNICK);
	uc->setLastActivity(GET_TICK());
	try {
		uc->accept(sock);
	} catch(const Exception&) {
		putConnection(uc);
		delete uc;
	}
}

void ConnectionManager::nmdcConnect(const string& aServer, short aPort, const string& aNick, const string& hubUrl) {
	if(shuttingDown)
		return;

	UserConnection* uc = getConnection(true);
	uc->setToken(aNick);
	uc->setHubUrl(hubUrl);
	uc->setState(UserConnection::STATE_CONNECT);
	uc->setFlag(UserConnection::FLAG_NMDC);
	try {
		uc->connect(aServer, aPort);
	} catch(const Exception&) {
		putConnection(uc);
		delete uc;
	}
}

void ConnectionManager::disconnect() throw() {
	delete server;

	server = 0;
}

void ConnectionManager::on(UserConnectionListener::Connected, UserConnection* aSource) throw() {
	dcassert(aSource->getState() == UserConnection::STATE_CONNECT);
	if(aSource->isSet(UserConnection::FLAG_NMDC)) {
		aSource->myNick(aSource->getToken());
		aSource->lock(CryptoManager::getInstance()->getLock(), CryptoManager::getInstance()->getPk());
	}
	aSource->setState(UserConnection::STATE_SUPNICK);
}

void ConnectionManager::on(UserConnectionListener::MyNick, UserConnection* aSource, const string& aNick) throw() {
	if(aSource->getState() != UserConnection::STATE_SUPNICK) {
		// Already got this once, ignore...
		dcdebug("CM::onMyNick %p sent nick twice\n", aSource);
		return;
	}

	dcassert(aNick.size() > 0);
	dcdebug("ConnectionManager::onMyNick %p, %s\n", aSource, aNick.c_str());
	dcassert(!aSource->getUser());

	// First, we try looking in the pending downloads...hopefully it's one of them...
	{
		Lock l(cs);
		for(ConnectionQueueItem::Iter i = downloads.begin(); i != downloads.end(); ++i) {
			ConnectionQueueItem* cqi = *i;
			if((cqi->getState() == ConnectionQueueItem::CONNECTING || cqi->getState() == ConnectionQueueItem::WAITING) && cqi->getUser()->getNick() == aNick) {
				aSource->setUser(cqi->getUser());
				// Indicate that we're interested in this file...
				aSource->setFlag(UserConnection::FLAG_DOWNLOAD);
				break;
			}
		}
	}

	if(!aSource->getUser()) {
		// Make sure we know who it is, i e that he/she is connected...
		if(!ClientManager::getInstance()->isOnline(aNick)) {
			dcdebug("CM::onMyNick Incoming connection from unknown user %s\n", aNick.c_str());
			putConnection(aSource);
			return;
		}

		aSource->setUser(ClientManager::getInstance()->getUser(aNick));

		if( BOOLSETTING(DROP_STUPID_CONNECTION) ) {
			if(Util::stricmp(aSource->getUser()->getNick(), aSource->getUser()->getClientNick()) == 0){
				dcdebug("CM::onMyNick Incoming connection from stupid user %s\n", aSource->getUser()->getNick());
				
				StringMap params;
				params["ip"] = aSource->getRemoteIp();
				params["hub"] = aSource->getUser()->getClientUrl();
				string tmp = Util::formatParams(STRING(DROP_STUPID_CONNECTION_LOG), params, false);

				LogManager::getInstance()->message(tmp);
				putConnection(aSource);
				return;
			}
		}

		if(aSource->getUser()->getUserIp()) {
			if(Util::stricmp(aSource->getUser()->getIp(), aSource->getRemoteIp()) != 0) {
				dcdebug("CM::onMyNick Incoming connection using fake nick(%s) %s\n", aNick, aSource->getRemoteIp());
                StringMap params;
				params["nick"] = aNick;
				params["ip"] = aSource->getRemoteIp();
				params["hub"] = aSource->getUser()->getClientUrl();
				string tmp = Util::formatParams(STRING(DROP_FAKE_NICK_CONNECTION_LOG), params, false);
								
				LogManager::getInstance()->message(tmp);

				putConnection(aSource);
				return;
			}
		}

		// We don't need this connection for downloading...make it an upload connection instead...
		aSource->setFlag(UserConnection::FLAG_UPLOAD);
	}

	if(aSource->getUser()->getIp().empty()) {
		if(Util::stricmp(aSource->getUser()->getNick(), aSource->getUser()->getClientNick()) != 0) {
			aSource->getUser()->setIp(aSource->getRemoteIp());
			aSource->getUser()->setUserIp(false);
			User::updated(aSource->getUser());
		}
	}

	if( aSource->isSet(UserConnection::FLAG_INCOMING) ) {
		aSource->myNick(aSource->getUser()->getClientNick()); 
		aSource->lock(CryptoManager::getInstance()->getLock(), CryptoManager::getInstance()->getPk());
	}

	aSource->setState(UserConnection::STATE_LOCK);
}

void ConnectionManager::on(UserConnectionListener::CLock, UserConnection* aSource, const string& aLock, const string& aPk) throw() {
	if(aSource->getState() != UserConnection::STATE_LOCK) {
		dcdebug("CM::onLock %p received lock twice, ignoring\n", aSource);
		return;
	}

	if( CryptoManager::getInstance()->isExtended(aLock) ) {
		// Alright, we have an extended protocol, set a user flag for this user and refresh his info...
		if( (aPk.find("DCPLUSPLUS") != string::npos) && aSource->getUser() && !aSource->getUser()->isSet(User::DCPLUSPLUS)) {
			aSource->getUser()->setFlag(User::DCPLUSPLUS);
			User::updated(aSource->getUser());
		}
		StringList defFeatures = features;
		if(BOOLSETTING(COMPRESS_TRANSFERS)) {
			defFeatures.push_back(UserConnection::FEATURE_ZLIB_GET);
		}

		aSource->supports(defFeatures);
	}

	aSource->setState(UserConnection::STATE_DIRECTION);
	aSource->direction(aSource->getDirectionString(), aSource->getNumber());
	aSource->key(CryptoManager::getInstance()->makeKey(aLock));
}

void ConnectionManager::on(UserConnectionListener::Direction, UserConnection* aSource, const string& dir, const string& num) throw() {
	if(aSource->getState() != UserConnection::STATE_DIRECTION) {
		dcdebug("CM::onDirection %p received direction twice, ignoring\n", aSource);
		return;
	}

	dcassert(aSource->isSet(UserConnection::FLAG_DOWNLOAD) ^ aSource->isSet(UserConnection::FLAG_UPLOAD));
	if(dir == "Upload") {
		// Fine, the other fellow want's to send us data...make sure we really want that...
		if(aSource->isSet(UserConnection::FLAG_UPLOAD)) {
			// Huh? Strange...disconnect...
			putConnection(aSource);
			return;
		}
	} else {
		if(aSource->isSet(UserConnection::FLAG_DOWNLOAD)) {
			int number = Util::toInt(num);
			// Damn, both want to download...the one with the highest number wins...
			if(aSource->getNumber() < number) {
				// Damn! We lost!
				aSource->unsetFlag(UserConnection::FLAG_DOWNLOAD);
				aSource->setFlag(UserConnection::FLAG_UPLOAD);
			} else if(aSource->getNumber() == number) {
				putConnection(aSource);
				return;
			}
		}
	}

	dcassert(aSource->isSet(UserConnection::FLAG_DOWNLOAD) ^ aSource->isSet(UserConnection::FLAG_UPLOAD));

	aSource->setState(UserConnection::STATE_KEY);
}

void ConnectionManager::addDownloadConnection(UserConnection* uc) {
	dcassert(uc->isSet(UserConnection::FLAG_DOWNLOAD));
	bool addConn = false;
	{
		Lock l(cs);

		ConnectionQueueItem::Iter i = find(downloads.begin(), downloads.end(), uc->getUser());
		if(i != downloads.end()) {
			ConnectionQueueItem* cqi = *i;
			if(cqi->getState() == ConnectionQueueItem::WAITING || cqi->getState() == ConnectionQueueItem::CONNECTING) {
				cqi->setState(ConnectionQueueItem::ACTIVE);
				uc->setFlag(UserConnection::FLAG_ASSOCIATED);

				fire(ConnectionManagerListener::Connected(), cqi);

				dcdebug("ConnectionManager::addDownloadConnection, leaving to downloadmanager\n");
				addConn = true;
			}
		}
	}

	if(addConn) {
		DownloadManager::getInstance()->addConnection(uc);
	} else {
		putConnection(uc);
	}
}

void ConnectionManager::addUploadConnection(UserConnection* uc) {
	dcassert(uc->isSet(UserConnection::FLAG_UPLOAD));

	bool addConn = false;
	{
		Lock l(cs);

		ConnectionQueueItem::Iter i = find(uploads.begin(), uploads.end(), uc->getUser());
		if(i == uploads.end()) {
			ConnectionQueueItem* cqi = getCQI(uc->getUser(), false);

			cqi->setState(ConnectionQueueItem::ACTIVE);
			uc->setFlag(UserConnection::FLAG_ASSOCIATED);

			fire(ConnectionManagerListener::Connected(), cqi);

			dcdebug("ConnectionManager::addUploadConnection, leaving to uploadmanager\n");
			addConn = true;
		}
	}

	if(addConn) {
		UploadManager::getInstance()->addConnection(uc);
	} else {
		putConnection(uc);
	}
}

void ConnectionManager::on(UserConnectionListener::Key, UserConnection* aSource, const string&/* aKey*/) throw() {
	if(aSource->getState() != UserConnection::STATE_KEY) {
		dcdebug("CM::onKey Bad state, ignoring");
		return;
	}

	dcassert(aSource->getUser());

	if(aSource->isSet(UserConnection::FLAG_DOWNLOAD)) {
		addDownloadConnection(aSource);
	} else {
		addUploadConnection(aSource);
	}
}

void ConnectionManager::on(UserConnectionListener::Failed, UserConnection* aSource, const string& aError) throw() {
	Lock l(cs);

	if(aSource->isSet(UserConnection::FLAG_ASSOCIATED)) {
		if(aSource->isSet(UserConnection::FLAG_DOWNLOAD)) {
			ConnectionQueueItem::Iter i = find(downloads.begin(), downloads.end(), aSource->getUser());
			dcassert(i != downloads.end());
			ConnectionQueueItem* cqi = *i;
			cqi->setState(ConnectionQueueItem::WAITING);
			cqi->setLastAttempt(GET_TICK());
			fire(ConnectionManagerListener::Failed(), cqi, aError);
		} else if(aSource->isSet(UserConnection::FLAG_UPLOAD)) {
			ConnectionQueueItem::Iter i = find(uploads.begin(), uploads.end(), aSource->getUser());
			dcassert(i != uploads.end());
		
			//ugly fix, haven't found the actual problem yet
			//seems like two upload connections get added so when the first fail the upload
			//cqi is removed and doesn't exist when the second one fail
			if(i != uploads.end()) {
				ConnectionQueueItem* cqi = *i;
				putCQI(cqi);
			}
		}
	}
	putConnection(aSource);
}

void ConnectionManager::disconnect(const User::Ptr& aUser, int isDownload) {
	Lock l(cs);
	for(UserConnection::Iter i = userConnections.begin(); i != userConnections.end(); ++i) {
		UserConnection* uc = *i;
		if(uc->getUser() == aUser && uc->isSet(isDownload ? UserConnection::FLAG_DOWNLOAD : UserConnection::FLAG_UPLOAD)) {
			uc->disconnect(true);
			break;
		}
	}
}

void ConnectionManager::shutdown() {
	TimerManager::getInstance()->removeListener(this);
	shuttingDown = true;
	disconnect();
	{
		Lock l(cs);
		for(UserConnection::Iter j = userConnections.begin(); j != userConnections.end(); ++j) {
			(*j)->disconnect(true);
		}
	}
	// Wait until all connections have died out...
	while(true) {
		{
			Lock l(cs);
			if(userConnections.empty()) {
				break;
			}
		}
		Thread::sleep(50);
	}
}

// UserConnectionListener
void ConnectionManager::on(UserConnectionListener::Supports, UserConnection* conn, const StringList& feat) throw() {
	for(StringList::const_iterator i = feat.begin(); i != feat.end(); ++i) {
		if(*i == UserConnection::FEATURE_MINISLOTS) {
			conn->setFlag(UserConnection::FLAG_SUPPORTS_MINISLOTS);
		} else if(*i == UserConnection::FEATURE_XML_BZLIST) {
			conn->setFlag(UserConnection::FLAG_SUPPORTS_XML_BZLIST);
		} else if(*i == UserConnection::FEATURE_ADCGET) {
			conn->setFlag(UserConnection::FLAG_SUPPORTS_ADCGET);
		} else if(*i == UserConnection::FEATURE_ZLIB_GET) {
			conn->setFlag(UserConnection::FLAG_SUPPORTS_ZLIB_GET);
		} else if(*i == UserConnection::FEATURE_TTHL) {
			conn->setFlag(UserConnection::FLAG_SUPPORTS_TTHL);
		} else if(*i == UserConnection::FEATURE_TTHF) {
			conn->setFlag(UserConnection::FLAG_SUPPORTS_TTHF);
			if(conn->getUser())
				conn->getUser()->setFlag(User::TTH_GET);
		}
	}
}
