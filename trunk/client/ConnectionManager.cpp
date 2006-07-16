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

ConnectionManager::ConnectionManager() : port(0), securePort(0), floodCounter(0), server(0), secureServer(0), shuttingDown(false) {
	TimerManager::getInstance()->addListener(this);

	adcFeatures.push_back("AD" + UserConnection::FEATURE_ADC_BASE);
}
// @todo clean this up
void ConnectionManager::listen() throw(Exception){
	unsigned short lastPort = (unsigned short)SETTING(TCP_PORT);
	
	if(lastPort == 0)
		lastPort = (unsigned short)Util::rand(1025, 32000);

	unsigned short firstPort = lastPort;

	disconnect();

	while(true) {
		try {
			server = new Server(false, lastPort, SETTING(BIND_ADDRESS));
			port = lastPort;
			break;
		} catch(const Exception&) {
			short newPort = (short)((lastPort == 32000) ? 1025 : lastPort + 1);
			if(!SettingsManager::getInstance()->isDefault(SettingsManager::TCP_PORT) || (firstPort == newPort)) {
				throw Exception("Could not find a suitable free port");
			}
			lastPort = newPort;
		}
	}

	if(!CryptoManager::getInstance()->TLSOk()) {
		return;
	}
	lastPort = (unsigned short)SETTING(TLS_PORT);
	firstPort = lastPort;

	while(true) {
		try {
			secureServer = new Server(true, lastPort, SETTING(BIND_ADDRESS));
			securePort = lastPort;
			break;
		} catch(const Exception&) {
			short newPort = (short)((lastPort == 32000) ? 1025 : lastPort + 1);
			if(!SettingsManager::getInstance()->isDefault(SettingsManager::TCP_PORT) || (firstPort == newPort)) {
				throw Exception("Could not find a suitable free port");
			}
			lastPort = newPort;
		}
	}
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

UserConnection* ConnectionManager::getConnection(bool aNmdc, bool secure) throw() {
	UserConnection* uc = new UserConnection(secure);
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

void ConnectionManager::on(TimerManagerListener::Second, u_int32_t aTick) throw() {
	User::List passiveUsers;
	ConnectionQueueItem::List removed;
	User::List idlers;

	bool tooMany = ((SETTING(DOWNLOAD_SLOTS) != 0) && DownloadManager::getInstance()->getDownloadCount() >= (size_t)SETTING(DOWNLOAD_SLOTS));
	bool tooFast = ((SETTING(MAX_DOWNLOAD_SPEED) != 0 && DownloadManager::getInstance()->getAverageSpeed() >= (SETTING(MAX_DOWNLOAD_SPEED)*1024)));

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

					if(!QueueManager::getInstance()->hasDownload(cqi->getUser())) {
						removed.push_back(cqi);
						continue;
					}

					// Always start high-priority downloads unless we have 3 more than maxdownslots already...
					bool startDown = !tooMany && !tooFast;

					if(!startDown) {
						bool extraFull = (SETTING(DOWNLOAD_SLOTS) != 0) && (DownloadManager::getInstance()->getDownloadCount() >= (size_t)(SETTING(DOWNLOAD_SLOTS)+3));
						startDown = !extraFull && QueueManager::getInstance()->hasDownload(cqi->getUser(), QueueItem::HIGHEST);
					}

					if(cqi->getState() == ConnectionQueueItem::WAITING) {
						if(startDown) {
							cqi->setState(ConnectionQueueItem::CONNECTING);
							ClientManager::getInstance()->connect(cqi->getUser());
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

void ConnectionManager::on(TimerManagerListener::Minute, u_int32_t aTick) throw() {	
	Lock l(cs);

	for(UserConnection::Iter j = userConnections.begin(); j != userConnections.end(); ++j) {
		if(((*j)->getLastActivity() + 180*1000) < aTick) {
			(*j)->disconnect(true);
		}
	}
}

static const u_int32_t FLOOD_TRIGGER = 20000;
static const u_int32_t FLOOD_ADD = 2000;

ConnectionManager::Server::Server(bool secure_, short port, const string& ip /* = "0.0.0.0" */) : secure(secure_), die(false) {
	sock.create();
	sock.bind(port, ip);
	sock.listen();

	start();
}


static const u_int32_t POLL_TIMEOUT = 250;

int ConnectionManager::Server::run() throw() {
	while(!die) {
		if(sock.wait(POLL_TIMEOUT, Socket::WAIT_READ) == Socket::WAIT_READ) {
			ConnectionManager::getInstance()->accept(sock, secure);
		}
	}
	return 0;
}

/**
 * Someone's connecting, accept the connection and wait for identification...
 * It's always the other fellow that starts sending if he made the connection.
 */
void ConnectionManager::accept(const Socket& sock, bool secure) throw() {
	u_int32_t now = GET_TICK();

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
	UserConnection* uc = getConnection(false, secure);
	uc->setFlag(UserConnection::FLAG_INCOMING);
	uc->setState(UserConnection::STATE_SUPNICK);
	uc->setLastActivity(GET_TICK());
	try { 
		uc->accept(sock);
		if(uc->isSecure() && !uc->isTrusted() && !BOOLSETTING(ALLOW_UNTRUSTED_CLIENTS)) {
			putConnection(uc);
			LogManager::getInstance()->message(STRING(CERTIFICATE_NOT_TRUSTED));
		}
	} catch(const Exception&) {
		putConnection(uc);
		delete uc;
	}
}

void ConnectionManager::adcConnect(const OnlineUser& aUser, short aPort, const string& aToken, bool secure) {
	if(shuttingDown)
		return;

	UserConnection* uc = getConnection(false, secure);
	uc->setToken(aToken);
	uc->setState(UserConnection::STATE_CONNECT);
	if(aUser.getIdentity().isOp()) {
		uc->setFlag(UserConnection::FLAG_OP);
	}
	try {
		uc->connect(aUser.getIdentity().getIp(), aPort);
	} catch(const Exception&) {
		putConnection(uc);
		delete uc;
	}
}

void ConnectionManager::on(AdcCommand::SUP, UserConnection* aSource, const AdcCommand& cmd) throw() {
	if(aSource->getState() != UserConnection::STATE_SUPNICK) {
		// Already got this once, ignore...@todo fix support updates
		dcdebug("CM::onMyNick %p sent nick twice\n", (void*)aSource);
		return;
	}

	bool baseOk = false;

	for(StringIterC i = cmd.getParameters().begin(); i != cmd.getParameters().end(); ++i) {
		if(i->compare(0, 2, "AD") == 0) {
			string feat = i->substr(2);
			if(feat == UserConnection::FEATURE_ADC_BASE)
				baseOk = true;
			else if(feat == UserConnection::FEATURE_ZLIB_GET)
				aSource->setFlag(UserConnection::FLAG_SUPPORTS_ZLIB_GET);
		}
	}

	if(!baseOk) {
		aSource->send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_GENERIC, "Invalid SUP"));
		aSource->disconnect();
		return;
	}

	if(aSource->isSet(UserConnection::FLAG_INCOMING)) {
		StringList defFeatures = adcFeatures;
		if(BOOLSETTING(COMPRESS_TRANSFERS)) {
			defFeatures.push_back("AD" + UserConnection::FEATURE_ZLIB_GET);
		}
		aSource->sup(defFeatures);
		aSource->inf(false);
	} else {
		aSource->inf(true);
	}
	aSource->setState(UserConnection::STATE_INF);
}

void ConnectionManager::on(AdcCommand::STA, UserConnection*, const AdcCommand&) throw() {

}

void ConnectionManager::on(UserConnectionListener::Connected, UserConnection* aSource) throw() {
	if(aSource->isSecure() && !aSource->isTrusted() && !BOOLSETTING(ALLOW_UNTRUSTED_CLIENTS)) {
		putConnection(aSource);
		LogManager::getInstance()->message(STRING(CERTIFICATE_NOT_TRUSTED));
		return;
	}

	dcassert(aSource->getState() == UserConnection::STATE_CONNECT);
	StringList defFeatures = adcFeatures;
	if(BOOLSETTING(COMPRESS_TRANSFERS)) {
		defFeatures.push_back("AD" + UserConnection::FEATURE_ZLIB_GET);
	}
	aSource->sup(defFeatures);
	aSource->setState(UserConnection::STATE_SUPNICK);
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

void ConnectionManager::on(AdcCommand::INF, UserConnection* aSource, const AdcCommand& cmd) throw() {
	if(aSource->getState() != UserConnection::STATE_INF) {
		// Already got this once, ignore...
		aSource->send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_PROTOCOL_GENERIC, "Expecting INF"));
		dcdebug("CM::onINF %p sent INF twice\n", (void*)aSource);
		aSource->disconnect();
		return;
	}

	string cid;
	if(!cmd.getParam("ID", 0, cid)) {
		aSource->send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_INF_MISSING, "ID missing").addParam("FL", "ID"));
		dcdebug("CM::onINF missing ID\n");
		aSource->disconnect();
		return;
	}

	aSource->setUser(ClientManager::getInstance()->findUser(CID(cid)));

	if(!aSource->getUser()) {
		dcdebug("CM::onINF: User not found");
		aSource->send(AdcCommand(AdcCommand::SEV_FATAL, AdcCommand::ERROR_GENERIC, "User not found"));
		putConnection(aSource);
		return;
	}

	if(aSource->isSet(UserConnection::FLAG_INCOMING)) {
		aSource->setFlag(UserConnection::FLAG_DOWNLOAD);
		addDownloadConnection(aSource);
	} else {
		aSource->setFlag(UserConnection::FLAG_UPLOAD);
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
			ConnectionQueueItem* cqi = *i;
			putCQI(cqi);
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

