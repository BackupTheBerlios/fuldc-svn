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

#include "FavoriteManager.h"

#include "ClientManager.h"
#include "ResourceManager.h"
#include "CryptoManager.h"

#include "HttpConnection.h"
#include "StringTokenizer.h"
#include "SimpleXML.h"
#include "UserCommand.h"

UserCommand FavoriteManager::addUserCommand(int type, int ctx, int flags, const string& name, const string& command, const string& hub) {
	// No dupes, add it...
	Lock l(cs);
	userCommands.push_back(UserCommand(lastId++, type, ctx, flags, name, command, hub));
	UserCommand& uc = userCommands.back();
	if(!uc.isSet(UserCommand::FLAG_NOSAVE))
		save();
	return userCommands.back();
}

bool FavoriteManager::getUserCommand(int cid, UserCommand& uc) {
	Lock l(cs);
	for(UserCommandListIter i = userCommands.begin(); i != userCommands.end(); ++i) {
		if(i->getId() == cid) {
			uc = *i;
			return true;
		}
	}
	return false;
}

bool FavoriteManager::moveUserCommand(int cid, int pos) {
	dcassert(pos == -1 || pos == 1);
	Lock l(cs);
	for(UserCommandListIter i = userCommands.begin(); i != userCommands.end(); ++i) {
		if(i->getId() == cid) {
			swap(*i, *(i + pos));
			return true;
		}
	}
	return false;
}

void FavoriteManager::updateUserCommand(const UserCommand& uc) {
	bool nosave = true;
	Lock l(cs);
	for(UserCommandListIter i = userCommands.begin(); i != userCommands.end(); ++i) {
		if(i->getId() == uc.getId()) {
			*i = uc;
			nosave = uc.isSet(UserCommand::FLAG_NOSAVE);
			break;
		}
	}
	if(!nosave)
		save();
}

int FavoriteManager::findUserCommand(const string& aName) {
	Lock l(cs);
	for(UserCommandListIter i = userCommands.begin(); i != userCommands.end(); ++i) {
		if(i->getName() == aName) {
			return i->getId();
		}
	}
	return -1;
}

void FavoriteManager::removeUserCommand(int cid) {
	bool nosave = true;
	Lock l(cs);
	for(UserCommandListIter i = userCommands.begin(); i != userCommands.end(); ++i) {
		if(i->getId() == cid) {
			nosave = i->isSet(UserCommand::FLAG_NOSAVE);
			userCommands.erase(i);
			break;
		}
	}
	if(!nosave)
		save();
}
void FavoriteManager::removeUserCommand(const string& srv) {
	Lock l(cs);
	for(UserCommandListIter i = userCommands.begin(); i != userCommands.end(); ) {
		if((i->getHub() == srv) && i->isSet(UserCommand::FLAG_NOSAVE)) {
			i = userCommands.erase(i);
		} else {
			++i;
		}
	}
}

void FavoriteManager::removeHubUserCommands(int ctx, const string& hub) {
	Lock l(cs);
	for(UserCommandListIter i = userCommands.begin(); i != userCommands.end(); ) {
		if(i->getHub() == hub && i->isSet(UserCommand::FLAG_NOSAVE) && i->getCtx() & ctx) {
			i = userCommands.erase(i);
		} else {
			++i;
		}
	}
}

void FavoriteManager::addFavoriteUser(User::Ptr& aUser) {
	Lock l(cs);
	if(users.find(aUser->getCID()) == users.end()) {
		StringList urls = ClientManager::getInstance()->getHubs(aUser->getCID());
		StringList nicks = ClientManager::getInstance()->getNicks(aUser->getCID());

		/// @todo make this an error probably...
		if(urls.empty())
			urls.push_back(Util::emptyString);
		if(nicks.empty())
			nicks.push_back(Util::emptyString);

		FavoriteMap::iterator i = users.insert(make_pair(aUser->getCID(), FavoriteUser(aUser, nicks[0], urls[0]))).first;
		fire(FavoriteManagerListener::UserAdded(), i->second);
		save();
	}
}

void FavoriteManager::removeFavoriteUser(User::Ptr& aUser) {
	Lock l(cs);
	FavoriteMap::iterator i = users.find(aUser->getCID());
	if(i != users.end()) {
		fire(FavoriteManagerListener::UserRemoved(), i->second);
		users.erase(i);
		save();
	}
}

void FavoriteManager::addFavorite(const FavoriteHubEntry& aEntry) {
	FavoriteHubEntry* f;

	FavoriteHubEntry::Iter i = getFavoriteHub(aEntry.getServer());
	if(i != favoriteHubs.end()) {
		return;
	}
	f = new FavoriteHubEntry(aEntry);
	favoriteHubs.push_back(f);
	fire(FavoriteManagerListener::FavoriteAdded(), f);
	save();
}

void FavoriteManager::removeFavorite(FavoriteHubEntry* entry) {
	FavoriteHubEntry::Iter i = find(favoriteHubs.begin(), favoriteHubs.end(), entry);
	if(i == favoriteHubs.end()) {
		return;
	}

	fire(FavoriteManagerListener::FavoriteRemoved(), entry);
	favoriteHubs.erase(i);
	delete entry;
	save();
}

bool FavoriteManager::checkFavHubExists(const FavoriteHubEntry& aEntry){
	FavoriteHubEntry::Iter i = getFavoriteHub(aEntry.getServer());
	if(i != favoriteHubs.end()) {
		return true;
	}
	return false;
}

bool FavoriteManager::addFavoriteDir(const string& aDirectory, const string & aName){
	string path = aDirectory;

	if( path[ path.length() -1 ] != PATH_SEPARATOR )
		path += PATH_SEPARATOR;

	for(StringPairIter i = favoriteDirs.begin(); i != favoriteDirs.end(); ++i) {
		if((Util::strnicmp(path, i->first, i->first.length()) == 0) && (Util::strnicmp(path, i->first, path.length()) == 0)) {
			return false;
		}
		if(Util::stricmp(aName, i->second) == 0) {
			return false;
		}
	}
	favoriteDirs.push_back(make_pair(aDirectory, aName));
	sort(favoriteDirs.begin(), favoriteDirs.end(), SortSecond<string, string>());
	save();
	return true;
}

bool FavoriteManager::removeFavoriteDir(const string& aName) {
	string d(aName);

	if(d[d.length() - 1] != PATH_SEPARATOR)
		d += PATH_SEPARATOR;

	for(StringPairIter j = favoriteDirs.begin(); j != favoriteDirs.end(); ++j) {
		if(Util::stricmp(j->first.c_str(), d.c_str()) == 0) {
			favoriteDirs.erase(j);
			save();
			return true;
		}
	}
	return false;
}

bool FavoriteManager::renameFavoriteDir(const string& aName, const string& anotherName) {

	for(StringPairIter j = favoriteDirs.begin(); j != favoriteDirs.end(); ++j) {
		if(Util::stricmp(j->second.c_str(), aName.c_str()) == 0) {
			j->second = anotherName;
			sort(favoriteDirs.begin(), favoriteDirs.end(), SortSecond<string, string>());
			save();
			return true;
		}
	}
	return false;
}

void FavoriteManager::onHttpFinished() throw() {
	string::size_type i, j;
	string* x;
	string bzlist;

	if(listType == TYPE_BZIP2) {
		try {
			CryptoManager::getInstance()->decodeBZ2((u_int8_t*)downloadBuf.data(), downloadBuf.size(), bzlist);
		} catch(const CryptoException&) {
			bzlist.clear();
		}
		x = &bzlist;
	} else {
		x = &downloadBuf;
	}

	{
		Lock l(cs);
		publicListMatrix[publicListServer].clear();

		if(x->compare(0, 5, "<?xml") == 0 || x->compare(0, 8, "\xEF\xBB\xBF<?xml") == 0) {
			loadXmlList(*x);
		} else {
			i = 0;

			string utfText = Text::acpToUtf8(*x);

			while( (i < utfText.size()) && ((j=utfText.find("\r\n", i)) != string::npos)) {
				StringTokenizer<string> tok(utfText.substr(i, j-i), '|');
				i = j + 2;
				if(tok.getTokens().size() < 4)
					continue;

				StringList::const_iterator k = tok.getTokens().begin();
				const string& name = *k++;
				const string& server = *k++;
				const string& desc = *k++;
				const string& usersOnline = *k++;
				publicListMatrix[publicListServer].push_back(HubEntry(name, server, desc, usersOnline));
			}
		}
	}
	downloadBuf = Util::emptyString;
}

class XmlListLoader : public SimpleXMLReader::CallBack {
public:
	XmlListLoader(HubEntryList& lst) : publicHubs(lst) { }
	virtual ~XmlListLoader() { }
	virtual void startTag(const string& name, StringPairList& attribs, bool) {
		if(name == "Hub") {
			const string& name = getAttrib(attribs, "Name", 0);
			const string& server = getAttrib(attribs, "Address", 1);
			const string& description = getAttrib(attribs, "Description", 2);
			const string& users = getAttrib(attribs, "Users", 3);
			const string& country = getAttrib(attribs, "Country", 4);
			const string& shared = getAttrib(attribs, "Shared", 5);
			const string& minShare = getAttrib(attribs, "Minshare", 5);
			const string& minSlots = getAttrib(attribs, "Minslots", 5);
			const string& maxHubs = getAttrib(attribs, "Maxhubs", 5);
			const string& maxUsers = getAttrib(attribs, "Maxusers", 5);
			const string& reliability = getAttrib(attribs, "Reliability", 5);
			const string& rating = getAttrib(attribs, "Rating", 5);
			publicHubs.push_back(HubEntry(name, server, description, users, country, shared, minShare, minSlots, maxHubs, maxUsers, reliability, rating));
		}
	}
	virtual void endTag(const string&, const string&) {

	}
private:
	HubEntryList& publicHubs;
};

void FavoriteManager::loadXmlList(const string& xml) {
	try {
		XmlListLoader loader(publicListMatrix[publicListServer]);
		SimpleXMLReader(&loader).fromXML(xml);
	} catch(const SimpleXMLException&) {

	}
}

void FavoriteManager::save() {
	if(dontSave)
		return;

	Lock l(cs);
	try {
		SimpleXML xml;

		xml.addTag("Favorites");
		xml.stepIn();

		xml.addTag("Hubs");
		xml.stepIn();

		for(FavoriteHubEntry::Iter i = favoriteHubs.begin(); i != favoriteHubs.end(); ++i) {
			xml.addTag("Hub");
			xml.addChildAttrib("Name",				(*i)->getName());
			xml.addChildAttrib("Connect",			(*i)->getConnect());
			xml.addChildAttrib("Description",		(*i)->getDescription());
			xml.addChildAttrib("Nick",				(*i)->getNick(false));
			xml.addChildAttrib("Password",			(*i)->getPassword());
			xml.addChildAttrib("Server",			(*i)->getServer());
			xml.addChildAttrib("UserDescription",	(*i)->getUserDescription());
			xml.addChildAttrib("StripIsp",			(*i)->getStripIsp());
			xml.addChildAttrib("ShowJoins",			(*i)->getShowJoins());
			xml.addChildAttrib("showUserList",		(*i)->getShowUserList());
			xml.addChildAttrib("LogMainchat",		(*i)->getLogMainChat());
			xml.addChildAttrib("Bottom",			Util::toString((*i)->getBottom()));
			xml.addChildAttrib("Top",				Util::toString((*i)->getTop()));
			xml.addChildAttrib("Right",				Util::toString((*i)->getRight()));
			xml.addChildAttrib("Left",				Util::toString((*i)->getLeft()));
			xml.addChildAttrib("HeaderOrder",		(*i)->getHeaderOrder());
			xml.addChildAttrib("HeaderWidths",		(*i)->getHeaderWidths());
			xml.addChildAttrib("HeaderVisible",		(*i)->getHeaderVisible());
		}
		xml.stepOut();
		xml.addTag("Users");
		xml.stepIn();
		for(FavoriteMap::iterator j = users.begin(); j != users.end(); ++j) {
			xml.addTag("User");
			xml.addChildAttrib("LastSeen", j->second.getLastSeen());
			xml.addChildAttrib("GrantSlot", j->second.isSet(FavoriteUser::FLAG_GRANTSLOT));
			xml.addChildAttrib("UserDescription", j->second.getDescription());
			xml.addChildAttrib("Nick", j->second.getNick());
			xml.addChildAttrib("URL", j->second.getUrl());
			xml.addChildAttrib("CID", j->first.toBase32());
		}
		xml.stepOut();
		xml.addTag("UserCommands");
		xml.stepIn();
		for(UserCommandListIter k = userCommands.begin(); k != userCommands.end(); ++k) {
			if(!k->isSet(UserCommand::FLAG_NOSAVE)) {
				xml.addTag("UserCommand");
				xml.addChildAttrib("Type", k->getType());
				xml.addChildAttrib("Context", k->getCtx());
				xml.addChildAttrib("Name", k->getName());
				xml.addChildAttrib("Command", k->getCommand());
				xml.addChildAttrib("Hub", k->getHub());
			}
		}
		xml.stepOut();
		//Favorite download to dirs
		xml.addTag("FavoriteDirs");
		xml.stepIn();
		StringPairList spl = getFavoriteDirs();
		for(StringPairIter i = spl.begin(); i != spl.end(); ++i) {
			xml.addTag("Directory", i->first);
			xml.addChildAttrib("Name", i->second);
		}
		xml.stepOut();

		xml.stepOut();

		string fname = getConfigFile();

		File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write(SimpleXML::utf8Header);
		f.write(xml.toXML());
		f.close();
		File::deleteFile(fname);
		File::renameFile(fname + ".tmp", fname);

	} catch(const Exception& e) {
		dcdebug("FavoriteManager::save: %s\n", e.getError().c_str());
	}
}

void FavoriteManager::load() {

	// Add NMDC standard op commands
	static const char kickstr[] =
		"$To: %[userNI] From: %[myNI] $<%[myNI]> You are being kicked because: %[line:Reason]|<%[myNI]> %[myNI] is kicking %[userNI] because: %[line:Reason]|$Kick %[userNI]|";
	addUserCommand(UserCommand::TYPE_RAW_ONCE, UserCommand::CONTEXT_CHAT | UserCommand::CONTEXT_SEARCH, UserCommand::FLAG_NOSAVE,
		STRING(KICK_USER), kickstr, "op");
	static const char redirstr[] =
		"$OpForceMove $Who:%[userNI]$Where:%[line:Target Server]$Msg:%[line:Message]|";
	addUserCommand(UserCommand::TYPE_RAW_ONCE, UserCommand::CONTEXT_CHAT | UserCommand::CONTEXT_SEARCH, UserCommand::FLAG_NOSAVE,
		STRING(REDIRECT_USER), redirstr, "op");

	// Add ADC standard op commands
	static const char adc_disconnectstr[] =
		"HDSC %[userSID]\n";
	addUserCommand(UserCommand::TYPE_RAW_ONCE, UserCommand::CONTEXT_CHAT | UserCommand::CONTEXT_SEARCH, UserCommand::FLAG_NOSAVE,
		STRING(DISCONNECT_USER), adc_disconnectstr, "adc://op");

	try {
		SimpleXML xml;
		xml.fromXML(File(getConfigFile(), File::READ, File::OPEN).read());

		if(xml.findChild("Favorites")) {
			xml.stepIn();
			load(xml);
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("FavoriteManager::load: %s\n", e.getError().c_str());
	}
}

void FavoriteManager::load(SimpleXML& aXml) {
	dontSave = true;

	aXml.resetCurrentChild();
	if(aXml.findChild("Hubs")) {
		aXml.stepIn();
		while(aXml.findChild("Hub")) {
			FavoriteHubEntry* e = new FavoriteHubEntry();
			e->setName(				aXml.getChildAttrib("Name"));
			e->setConnect(			aXml.getBoolChildAttrib("Connect"));
			e->setDescription(		aXml.getChildAttrib("Description"));
			e->setNick(				aXml.getChildAttrib("Nick"));
			e->setPassword(			aXml.getChildAttrib("Password"));
			e->setServer(			aXml.getChildAttrib("Server"));
			e->setUserDescription(	aXml.getChildAttrib("UserDescription"));
			e->setStripIsp(			aXml.getBoolChildAttrib("StripIsp"));
			e->setShowJoins(		aXml.getBoolChildAttrib("ShowJoins"));
			e->setShowUserList(		aXml.getBoolChildAttrib("ShowUserList", BOOLSETTING(GET_USER_INFO)));
			e->setLogMainChat(		aXml.getBoolChildAttrib("LogMainchat", BOOLSETTING(LOG_MAIN_CHAT)));
			e->setBottom((u_int16_t)aXml.getIntChildAttrib("Bottom") );
			e->setTop((u_int16_t)	aXml.getIntChildAttrib("Top"));
			e->setRight((u_int16_t)	aXml.getIntChildAttrib("Right"));
			e->setLeft((u_int16_t)	aXml.getIntChildAttrib("Left"));
			e->setHeaderOrder(		aXml.getChildAttrib("HeaderOrder", SETTING(HUBFRAME_ORDER)));
			e->setHeaderWidths(		aXml.getChildAttrib("HeaderWidths", SETTING(HUBFRAME_WIDTHS)));
			e->setHeaderVisible(	aXml.getChildAttrib("HeaderVisible", SETTING(HUBFRAME_VISIBLE)));
			favoriteHubs.push_back(e);
		}
		aXml.stepOut();
	}
	aXml.resetCurrentChild();
	if(aXml.findChild("Users")) {
		aXml.stepIn();
		while(aXml.findChild("User")) {
			User::Ptr u;
			const string& cid = aXml.getChildAttrib("CID");
			const string& nick = aXml.getChildAttrib("Nick");
			const string& hubUrl = aXml.getChildAttrib("URL");

			if(cid.length() != 39) {
				if(nick.empty() || hubUrl.empty())
					continue;
				u = ClientManager::getInstance()->getUser(nick, hubUrl);
			} else {
				u = ClientManager::getInstance()->getUser(CID(cid));
				if(u->getFirstNick().empty())
					u->setFirstNick(nick);
			}
			FavoriteMap::iterator i = users.insert(make_pair(u->getCID(), FavoriteUser(u, nick, hubUrl))).first;

			if(aXml.getBoolChildAttrib("GrantSlot"))
				i->second.setFlag(FavoriteUser::FLAG_GRANTSLOT);

			i->second.setLastSeen((u_int32_t)aXml.getIntChildAttrib("LastSeen"));
			i->second.setDescription(aXml.getChildAttrib("UserDescription"));

		}
		aXml.stepOut();
	}
	aXml.resetCurrentChild();
	if(aXml.findChild("UserCommands")) {
		aXml.stepIn();
		while(aXml.findChild("UserCommand")) {
			addUserCommand(aXml.getIntChildAttrib("Type"), aXml.getIntChildAttrib("Context"),
				0, aXml.getChildAttrib("Name"), aXml.getChildAttrib("Command"), aXml.getChildAttrib("Hub"));
		}
		aXml.stepOut();
	}

	//compatibility with old fuldc favorite dirs, they're stored in dcplusplus.xml
	aXml.resetCurrentChild();
	if(aXml.findChild("DownloadPaths")) {
		aXml.stepIn();
		while(aXml.findChild("DownloadPath")){
			string name = aXml.getChildAttrib("Name");
			string path = aXml.getChildData();
			addFavoriteDir(path, name);
		}
		aXml.stepOut();
	} 

	//Favorite download to dirs
	aXml.resetCurrentChild();
	if(aXml.findChild("FavoriteDirs")) {
		aXml.stepIn();
		while(aXml.findChild("Directory")) {
			string virt = aXml.getChildAttrib("Name");
			string d(aXml.getChildData());
			FavoriteManager::getInstance()->addFavoriteDir(d, virt);
		}
		aXml.stepOut();
	}

	dontSave = false;
}

void FavoriteManager::userUpdated(const OnlineUser& info) {
	Lock l(cs);
	FavoriteMap::iterator i = users.find(info.getUser()->getCID());
	if(i != users.end()) {
		FavoriteUser& fu = i->second;
		fu.update(info);
		save();
	}
}

FavoriteHubEntry* FavoriteManager::getFavoriteHubEntry(const string& aServer) {
	for(FavoriteHubEntry::Iter i = favoriteHubs.begin(); i != favoriteHubs.end(); ++i) {
		FavoriteHubEntry* hub = *i;
		if(Util::stricmp(hub->getServer(), aServer) == 0) {
			return hub;
		}
	}
	return NULL;
}

bool FavoriteManager::hasSlot(const User::Ptr& aUser) const {
	Lock l(cs);
	FavoriteMap::const_iterator i = users.find(aUser->getCID());
	if(i == users.end())
		return false;
	return i->second.isSet(FavoriteUser::FLAG_GRANTSLOT);
}

time_t FavoriteManager::getLastSeen(const User::Ptr& aUser) const {
	Lock l(cs);
	FavoriteMap::const_iterator i = users.find(aUser->getCID());
	if(i == users.end())
		return 0;
	return i->second.getLastSeen();
}

void FavoriteManager::setAutoGrant(const User::Ptr& aUser, bool grant) {
	Lock l(cs);
	FavoriteMap::iterator i = users.find(aUser->getCID());
	if(i == users.end())
		return;
	if(grant)
		i->second.setFlag(FavoriteUser::FLAG_GRANTSLOT);
	else
		i->second.unsetFlag(FavoriteUser::FLAG_GRANTSLOT);
	save();
}
void FavoriteManager::setUserDescription(const User::Ptr& aUser, const string& description) {
	Lock l(cs);
	FavoriteMap::iterator i = users.find(aUser->getCID());
	if(i == users.end())
		return;
	i->second.setDescription(description);
	save();
}

StringList FavoriteManager::getHubLists() {
	StringTokenizer<string> lists(SETTING(HUBLIST_SERVERS), ';');
	return lists.getTokens();
}

void FavoriteManager::setHubList(const string& hubs) {
	SettingsManager::getInstance()->set(SettingsManager::HUBLIST_SERVERS, hubs);
	fire(FavoriteManagerListener::PublicHubsUpdated());
}

bool FavoriteManager::setHubList(int aHubList) {
	if(!running) {
		lastServer = aHubList;
		StringList sl = getHubLists();
		publicListServer = sl[(lastServer) % sl.size()];
		return true;
	}
	return false;
}

void FavoriteManager::refresh() {
	StringList sl = getHubLists();
	if(sl.empty())
		return;
	publicListServer = sl[(lastServer) % sl.size()];
	if(Util::strnicmp(publicListServer.c_str(), "http://", 7) != 0) {
		lastServer++;
		return;
	}

	fire(FavoriteManagerListener::DownloadStarting(), publicListServer);
	if(!running) {
		if(!c)
			c = new HttpConnection();
		{
			Lock l(cs);
			publicListMatrix[publicListServer].clear();
		}
		c->addListener(this);
		c->downloadFile(publicListServer);
		running = true;
	}
}

UserCommandList FavoriteManager::getUserCommands(int ctx, const StringList& hubs) {
	vector<bool> isOp(hubs.size());

	for(size_t i = 0; i < hubs.size(); ++i) {
		if(ClientManager::getInstance()->isOp(ClientManager::getInstance()->getMe(), hubs[i])) {
			isOp[i] = true;
		}
	}

	Lock l(cs);
	UserCommandList lst;
	for(UserCommandListIter i = userCommands.begin(); i != userCommands.end(); ++i) {
		UserCommand& uc = *i;
		if(!(uc.getCtx() & ctx)) {
			continue;
		}

		for(size_t j = 0; j < hubs.size(); ++j) {
			const string& hub = hubs[j];
			bool hubAdc = hub.compare(0, 6, "adc://") == 0;
			bool commandAdc = uc.getHub().compare(0, 6, "adc://") == 0;
			if(hubAdc && commandAdc) {
				if((uc.getHub().length() == 6) ||
					(uc.getHub() == "adc://op" && isOp[j]) ||
					(uc.getHub() == hub) )
				{
					lst.push_back(*i);
					break;
				}
			} else if(!hubAdc && !commandAdc) {
				if((uc.getHub().length() == 0) ||
					(uc.getHub() == "op" && isOp[j]) ||
					(uc.getHub() == hub) )
				{
					lst.push_back(*i);
					break;
				}
			}
		}
	}
	return lst;
}

// HttpConnectionListener
void FavoriteManager::on(Data, HttpConnection*, const u_int8_t* buf, size_t len) throw() {
	downloadBuf.append((const char*)buf, len);
}

void FavoriteManager::on(Failed, HttpConnection*, const string& aLine) throw() {
	c->removeListener(this);
	lastServer++;
	running = false;
	fire(FavoriteManagerListener::DownloadFailed(), aLine);
}
void FavoriteManager::on(Complete, HttpConnection*, const string& aLine) throw() {
	c->removeListener(this);
	onHttpFinished();
	running = false;
	fire(FavoriteManagerListener::DownloadFinished(), aLine);
}
void FavoriteManager::on(Redirected, HttpConnection*, const string& aLine) throw() {
	fire(FavoriteManagerListener::DownloadStarting(), aLine);
}
void FavoriteManager::on(TypeNormal, HttpConnection*) throw() {
	listType = TYPE_NORMAL;
}
void FavoriteManager::on(TypeBZ2, HttpConnection*) throw() {
	listType = TYPE_BZIP2;
}

void FavoriteManager::on(UserUpdated, const OnlineUser& user) throw() {
	userUpdated(user);
}
void FavoriteManager::on(UserDisconnected, const User::Ptr& user) throw() {
	bool isFav = false;
	{
		Lock l(cs);
		FavoriteMap::iterator i = users.find(user->getCID());
		if(i != users.end()) {
			isFav = true;
			i->second.setLastSeen(GET_TIME());
			save();
		}
	}
	if(isFav)
		fire(FavoriteManagerListener::StatusChanged(), user);
}

void FavoriteManager::on(UserConnected, const User::Ptr& user) throw() {
	bool isFav = false;
	{
		Lock l(cs);
		FavoriteMap::iterator i = users.find(user->getCID());
		if(i != users.end()) {
			isFav = true;
		}
	}
	if(isFav)
		fire(FavoriteManagerListener::StatusChanged(), user);
}
