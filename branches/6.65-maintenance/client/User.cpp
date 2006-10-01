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

#include "SettingsManager.h"
#include "ResourceManager.h"
#include "TimerManager.h"

#include "User.h"

#include "Client.h"
#include "FavoriteUser.h"

User::~User() throw() {
	delete favoriteUser;
}

void User::connect() {
	Lock l(cs);
	if(client) {
		client->connect(this);
	}
}

const string& User::getClientNick() const {
	Lock l(cs);
	if(client) {
		return client->getNick();
	} else {
		return SETTING(NICK);
	}
}

void User::updated(User::Ptr& aUser) {
	Lock l(aUser->cs);
	if(aUser->client) {
		aUser->client->updated(aUser);
	}
}

const string& User::getClientName() const {
	Lock l(cs);
	if(client) {
		return client->getName();
	} else if(!getLastHubName().empty()) {
		return getLastHubName();
	} else {
		return STRING(OFFLINE);
	}
}

string User::getClientUrl() const {
	Lock l(cs);
	if(client) {
		return client->getHubUrl();
	} else {
		return Util::emptyString;
	}
}

void User::privateMessage(const string& aMsg) {
	Lock l(cs);
	if(client) {
		client->privateMessage(this, aMsg);
	}
}

bool User::isClientOp() const {
	Lock l(cs);
	if(client) {
		return client->getOp();
	}
	return false;
}

void User::send(const string& aMsg) {
	Lock l(cs);
	if(client) {
		client->send(aMsg);
	}
}

void User::sendUserCmd(const string& aUserCmd) {
	Lock l(cs);
	if(client) {
		client->sendUserCmd(aUserCmd);
	}
}

void User::clientMessage(const string& aMsg) {
	Lock l(cs);
	if(client) {
		client->hubMessage(aMsg);
	}
}

void User::setClient(Client* aClient) { 
	Lock l(cs); 
	client = aClient; 
	if(client == NULL) {
		if (isSet(ONLINE) && isFavoriteUser())
			setFavoriteLastSeen();
		unsetFlag(ONLINE);
	}
	else {
		setLastHubAddress(aClient->getIpPort());
		setLastHubName(aClient->getName());
		setFlag(ONLINE);
		unsetFlag(QUIT_HUB);
	}
}

void User::getParams(StringMap& ucParams) {
	ucParams["nick"] = getNick();
	ucParams["tag"] = getTag();
	ucParams["description"] = getDescription();
	ucParams["email"] = getEmail();
	ucParams["share"] = Util::toString(getBytesShared());
	ucParams["shareshort"] = Util::formatBytes(getBytesShared());
	ucParams["ip"] = getIp();
}

// favorite user stuff
void User::setFavoriteUser(FavoriteUser* aUser) {
	Lock l(cs);
	delete favoriteUser;
	favoriteUser = aUser;
}

bool User::isFavoriteUser() const {
	Lock l(cs);
	return (favoriteUser != NULL);
}

bool User::getFavoriteGrantSlot() const {
	Lock l(cs);
	return (favoriteUser != NULL && favoriteUser->isSet(FavoriteUser::FLAG_GRANTSLOT));
}

void User::setFavoriteGrantSlot(bool grant) {
	Lock l(cs);
	if (favoriteUser == NULL)
		return;

	if (grant)
		favoriteUser->setFlag(FavoriteUser::FLAG_GRANTSLOT);
	else
		favoriteUser->unsetFlag(FavoriteUser::FLAG_GRANTSLOT);
}

void User::setFavoriteLastSeen(time_t anOfflineTime) {
	Lock l(cs);
	if (favoriteUser != NULL) {
		if (anOfflineTime != 0)
			favoriteUser->setLastSeen(anOfflineTime);
		else
			favoriteUser->setLastSeen(GET_TIME());
	}
}

time_t User::getFavoriteLastSeen() const {
	Lock l(cs);
	if (favoriteUser != NULL)
		return favoriteUser->getLastSeen();
	else
		return 0;
}

const string& User::getUserDescription() const {
	Lock l(cs);
	if (favoriteUser != NULL)
		return favoriteUser->getDescription();
	else
		return Util::emptyString;
}

void User::setUserDescription(const string& aDescription) {
	Lock l(cs);
	if (favoriteUser != NULL)
		favoriteUser->setDescription(aDescription);
}

StringMap& User::clientEscapeParams(StringMap& sm) const {
	return client->escapeParams(sm);
}
