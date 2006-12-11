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

#include "ClientManager.h"

#include "ShareManager.h"
#include "SearchManager.h"
#include "CryptoManager.h"
#include "FavoriteManager.h"
#include "LogManager.h"

//#include "AdcHub.h"
#include "NmdcHub.h"

Client* ClientManager::getClient(const string& aHubURL) {
	Client* c;
	c = new NmdcHub(aHubURL);

	{
		Lock l(cs);
		clients.push_back(c);
	}

	c->addListener(this);

	return c;
}

void ClientManager::putClient(Client* aClient) {
	aClient->shutdown();

	fire(ClientManagerListener::ClientDisconnected(), aClient);
	aClient->removeListeners();

	{
		Lock l(cs);
		clients.remove(aClient);
	}
	delete aClient;
}

void ClientManager::infoUpdated() {
	Lock l(cs);
	for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->isConnected()) {
			(*i)->info(false);
		}
	}
}

void ClientManager::on(NmdcSearch, Client* aClient, const string& aSeeker, int aSearchType, int64_t aSize,
									int aFileType, const string& aString) throw()
{
	Speaker<ClientManagerListener>::fire(ClientManagerListener::IncomingSearch(), aString);

	bool isPassive = (aSeeker.compare(0, 4, "Hub:") == 0);

	// We don't wan't to answer passive searches if we're in passive mode...
	if(isPassive && !ClientManager::getInstance()->isActive()) {
		return;
	}

	SearchResult::List l;
	ShareManager::getInstance()->search(l, aString, aSearchType, aSize, aFileType, aClient, isPassive ? 5 : 10);
//		dcdebug("Found %d items (%s)\n", l.size(), aString.c_str());
	if(l.size() > 0) {
		if(isPassive) {
			string name = aSeeker.substr(4);
			// Good, we have a passive seeker, those are easier...
			string str;
			for(SearchResult::Iter i = l.begin(); i != l.end(); ++i) {
				SearchResult* sr = *i;
				str += sr->toSR(*aClient);
				str[str.length()-1] = 5;
				str += name;
				str += '|';

				sr->decRef();
			}

			if(str.size() > 0)
				aClient->send(str);

		} else {
			try {
				string ip, file;
				uint16_t port = 0;
				Util::decodeUrl(aSeeker, ip, port, file);
				ip = Socket::resolve(ip);

				// Temporary fix to avoid spamming hublist.org and dcpp.net
				if(ip == "70.85.55.252" || ip == "207.44.220.108") {
					LogManager::getInstance()->message("Someone is trying to use your client to spam " + ip + ", please urge hub owner to fix this");
					return;
				}

				if(port == 0)
					port = 412;
				for(SearchResult::Iter i = l.begin(); i != l.end(); ++i) {
					SearchResult* sr = *i;
					udp.writeTo(ip, port, sr->toSR(*aClient));
					sr->decRef();
				}
			} catch(const SocketException& /* e */) {
				udp.disconnect();
				dcdebug("Search caught error\n");
			}
		}
	}
}

User::Ptr ClientManager::getUser(const string& aNick, const string& aHint /* = Util::emptyString */) {
	Lock l(cs);
	dcassert(aNick.size() > 0);
	UserPair p = users.equal_range(aNick);

	if(p.first == p.second) {
		User::Ptr& u = users.insert(make_pair(aNick, new User(aNick)))->second;
		u->setLastHubAddress(aHint);
		return u;
	}

	UserIter i;
	if(aHint.empty()) {
		// No hint, first, try finding an online user...
		for(i = p.first; i != p.second; ++i) {
			if(i->second->isOnline()) {
				return i->second;
			}
		}
		// Blah...return the first one...doesn't matter now...
		return p.first->second;
	}

	// Since we have a hint, make sure we use it...
	for(i = p.first; i != p.second; ++i) {
		if(i->second->getLastHubAddress() == aHint) {
			return i->second;
		}
	}
	// Since old dc++'s didn't return port in $SR's we'll check for port-less hints as well
	string::size_type k = aHint.find(':');
	if(k != string::npos) {
		string hint = aHint.substr(0, k); 
		for(i = p.first; i != p.second; ++i) {
			if(i->second->getLastHubAddress() == hint) {
				return i->second;
			}
		}
	}
	
	// Try to find an online user, higher probablility that it's one of these...
	for(i = p.first; i != p.second; ++i) {
		if(i->second->isOnline()) {
			return i->second;
		}
	}

	return users.insert(make_pair(aNick, new User(aNick)))->second;
}

User::Ptr ClientManager::getUser(const string& aNick, Client* aClient, bool putOnline /* = true */) {
	Lock l(cs);
	dcassert(aNick.size() > 0);
	dcassert(aClient != NULL);
	dcassert(find(clients.begin(), clients.end(), aClient) != clients.end());

	UserPair p = users.equal_range(aNick);
	UserIter i;

	// Check for a user already online
	for(i = p.first; i != p.second; ++i) {
		if(i->second->isClient(aClient)) {
			return i->second;
		}
	}

	// Check for an offline user that was on that hub that we can put online again
	for(i = p.first; i != p.second; ++i) {
		if( (!i->second->isOnline()) && 
			((i->second->getLastHubAddress() == aClient->getHubUrl()) || (i->second->getLastHubAddress() == aClient->getIpPort())) )
		{
			if(putOnline) {
				i->second->setClient(aClient);
				fire(ClientManagerListener::UserUpdated(), i->second);
			}
			return i->second;
		}
	}

	// Check for any offline user
	for(i = p.first; i != p.second; ++i) {
		if( (!i->second->isOnline()) ) {
			if(putOnline) {
				i->second->setClient(aClient);
				fire(ClientManagerListener::UserUpdated(), i->second);
			}
			return i->second;
		}
	}
	
	// Create a new user
	i = users.insert(make_pair(aNick, new User(aNick)));
	if(putOnline) {
		i->second->setClient(aClient);
		fire(ClientManagerListener::UserUpdated(), i->second);
	}
	return i->second;
}

void ClientManager::search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken) {
	Lock l(cs);

	updateCachedIp(); // no point in doing a resolve for every single hub we're searching on

	for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
		if((*i)->isConnected()) {
			(*i)->search(aSizeMode, aSize, aFileType, aString, aToken);
		}
	}
}

void ClientManager::search(StringList& who, int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken) {
	Lock l(cs);

	updateCachedIp(); // no point in doing a resolve for every single hub we're searching on

	for(StringIter it = who.begin(); it != who.end(); ++it) {
		string& client = *it;
		for(Client::Iter j = clients.begin(); j != clients.end(); ++j) {
			Client* c = *j;
			if(c->isConnected() && c->getHubUrl() == client) {
				c->search(aSizeMode, aSize, aFileType, aString, aToken);
			}
		}
	}
}

void ClientManager::putUserOffline(User::Ptr& aUser, bool quitHub /*= false*/) {
	{
		Lock l(cs);
		aUser->setIp(Util::emptyString);
		aUser->setUserIp(false);
		aUser->unsetFlag(User::PASSIVE);
		aUser->unsetFlag(User::OP);
		aUser->unsetFlag(User::DCPLUSPLUS);
		if(quitHub)
			aUser->setFlag(User::QUIT_HUB);
		aUser->setClient(NULL);
	}
	fire(ClientManagerListener::UserUpdated(), aUser);
}

void ClientManager::on(TimerManagerListener::Minute, time_t /* aTick */) throw() {
	Lock l(cs);

	// Collect some garbage...
	UserIter i = users.begin();
	while(i != users.end()) {
		if(i->second->unique()) {
			users.erase(i++);
		} else {
			++i;
		}
	}

	for(Client::Iter j = clients.begin(); j != clients.end(); ++j) {
		(*j)->info(false);
	}
}

void ClientManager::on(Failed, Client* client, const string&) throw() { 
	FavoriteManager::getInstance()->removeUserCommand(client->getHubUrl());
	fire(ClientManagerListener::ClientDisconnected(), client);
}

void ClientManager::on(UserCommand, Client* client, int aType, int ctx, const string& name, const string& command) throw() { 
	if(BOOLSETTING(HUB_USER_COMMANDS)) {
 		if(aType == ::UserCommand::TYPE_CLEAR) {
 			FavoriteManager::getInstance()->removeHubUserCommands(ctx, client->getHubUrl());
 		} else {
 			FavoriteManager::getInstance()->addUserCommand(aType, ctx, ::UserCommand::FLAG_NOSAVE, name, command, client->getHubUrl());
 		}
	}
}

void ClientManager::updateCachedIp() {
	// Best case - the server detected it
	if((!BOOLSETTING(NO_IP_OVERRIDE) || SETTING(EXTERNAL_IP).empty())) {
		for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
			if((*i)->getMe() && !(*i)->getMe()->getIp().empty()) {
				cachedIp = (*i)->getMe()->getIp();
				return;
			}
		}
	}

	if(!SETTING(EXTERNAL_IP).empty()) {
		cachedIp = Socket::resolve(SETTING(EXTERNAL_IP));
		return;
	}

	//if we've come this far just use the first client to get the ip.
	if(clients.size() > 0)
		cachedIp = (*clients.begin())->getLocalIp();
}
