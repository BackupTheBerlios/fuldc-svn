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

#include "NmdcHub.h"

#include "ResourceManager.h"
#include "ClientManager.h"
#include "SearchManager.h"
#include "ShareManager.h"
#include "CryptoManager.h"
#include "ConnectionManager.h"

#include "Socket.h"
#include "UserCommand.h"
#include "StringTokenizer.h"

NmdcHub::NmdcHub(const string& aHubURL) : Client(aHubURL, '|', false), supportFlags(0), state(STATE_CONNECT),
	lastUpdate(0)
{
}

NmdcHub::~NmdcHub() throw() {
	clearUsers();
}

void NmdcHub::connect() {
	supportFlags = 0;
	lastMyInfoA.clear();
 	lastMyInfoB.clear();
	lastUpdate = 0;

	state = STATE_LOCK;

	Client::connect();
}

void NmdcHub::connect(const User::Ptr aUser) {
	checkstate(); 
	dcdebug("NmdcHub::connectToMe %s\n", aUser->getNick().c_str());
	if(ClientManager::getInstance()->isActive()) {
		send("$ConnectToMe " + toNmdc(aUser->getNick()) + " " + getLocalIp() + ":" + Util::toString(ConnectionManager::getInstance()->getPort()) + "|");
	} else {
		send("$RevConnectToMe " + toNmdc(getNick()) + " " + toNmdc(aUser->getNick())  + "|");
	}
}

int64_t NmdcHub::getAvailable() const {
	Lock l(cs);
	int64_t x = 0;
	for(User::NickMap::const_iterator i = users.begin(); i != users.end(); ++i) {
		x+=i->second->getBytesShared();
	}
	return x;
}

void NmdcHub::refreshUserList(bool unknownOnly /* = false */) {
	Lock l(cs);
	if(unknownOnly) {
		for(User::NickIter i = users.begin(); i != users.end(); ++i) {
			if(i->second->getConnection().empty()) {
				getInfo(i->second);
			}
		}
	} else {
		clearUsers();
		getNickList();
	}
}

void NmdcHub::clearUsers() {
	Lock l(cs);
	for(User::NickIter i = users.begin(); i != users.end(); ++i) {
		ClientManager::getInstance()->putUserOffline(i->second);		
	}
	users.clear();
}

void NmdcHub::onLine(const string& aLine) throw() {
	updateActivity();

	if(aLine.length() == 0)
		return;
	
	if(aLine[0] != '$') {
		// Check if we're being banned...
		if(state != STATE_CONNECTED) {
			if(Util::findSubString(aLine, "banned") != string::npos) {
				setAutoReconnect(false);
			}
		}
		fire(ClientListener::Message(), this, validateMessage(fromNmdc(aLine), true));
		return;
	}

	string cmd;
	string param;
	string::size_type x;
	
	if( (x = aLine.find(' ')) == string::npos) {
		cmd = aLine;
	} else {
		cmd = aLine.substr(0, x);
		param = aLine.substr(x+1);
	}

	if(cmd == "$Search") {
		if(state != STATE_CONNECTED) {
			return;
		}
		//there's a race condition between searches and when me gets set
		if(!me) {
			return;
		}
		string::size_type i = 0;
		string::size_type j = param.find(' ', i);
		if(j == string::npos || i == j)
			return;
		
		string seeker = fromNmdc(param.substr(i, j-i));

		// Filter own searches
		if(ClientManager::getInstance()->isActive()) {
			if(seeker == (ClientManager::getInstance()->getCachedIp() + ":" + Util::toString(SearchManager::getInstance()->getPort()))) {
				return;
			}
		} else {
			// Hub:seeker
			if(Util::stricmp(seeker.c_str() + 4, getNick().c_str()) == 0) {
				return;
			}
		}

		i = j + 1;
		
		{
			Lock l(cs);
			time_t tick = GET_TICK();

			seekers.push_back(make_pair(seeker, tick));

			// First, check if it's a flooder
			for(FloodIter fi = flooders.begin(); fi != flooders.end(); ++fi) {
				if(fi->first == seeker) {
					return;
				}
			}

			int count = 0;
			for(FloodIter fi = seekers.begin(); fi != seekers.end(); ++fi) {
				if(fi->first == seeker)
					count++;

				if(count > 7) {
					flooders.push_back(make_pair(seeker, tick));
					return;
				}
			}
		}

		int a;
		if(param[i] == 'F') {
			a = SearchManager::SIZE_DONTCARE;
		} else if(param[i+2] == 'F') {
			a = SearchManager::SIZE_ATLEAST;
		} else {
			a = SearchManager::SIZE_ATMOST;
		}
		i += 4;
		j = param.find('?', i);
		if(j == string::npos || i == j)
			return;
		string size = param.substr(i, j-i);
		i = j + 1;
		j = param.find('?', i);
		if(j == string::npos || i == j)
			return;
		int type = Util::toInt(param.substr(i, j-i)) - 1;
		i = j + 1;
		param = param.substr(i);

		if(param.size() > 0) {
			fire(ClientListener::NmdcSearch(), this, seeker, a, Util::toInt64(size), type, fromNmdc(param));
			
			if(seeker.compare(0, 4, "Hub:") == 0) {
				User::Ptr u;
				{
					Lock l(cs);
					User::NickIter ni = users.find(seeker.substr(4));
					if(ni != users.end() && !ni->second->isSet(User::PASSIVE)) {
						u = ni->second;
						u->setFlag(User::PASSIVE);
					}
				}

				if(u) {
					updated(u);
				}
			}
		}
	} else if(cmd == "$MyINFO") {
		string::size_type i, j;
		i = 5;
		j = param.find(' ', i);
		if( (j == string::npos) || (j == i) )
			return;
		string nick = fromNmdc(param.substr(i, j-i));
		i = j + 1;
		User::Ptr u;
		dcassert(nick.size() > 0);

		{
			Lock l(cs);
			User::NickIter ni = users.find(nick);
			if(ni == users.end()) {
				u = users[nick] = ClientManager::getInstance()->getUser(nick, this);
			} else {
				u  = ni->second;
			}
		}
		j = param.find('$', i);
		if(j == string::npos)
			return;
		string tmpDesc = validateMessage(fromNmdc(param.substr(i, j-i)), true);
		// Look for a tag...
		if(tmpDesc.size() > 0 && tmpDesc[tmpDesc.size()-1] == '>') {
			x = tmpDesc.rfind('<');
			if(x != string::npos) {
				// Hm, we have something...
				u->setTag(tmpDesc.substr(x));
				tmpDesc.erase(x);
			} else {
				u->setTag(Util::emptyString);
			}
		} else {
			u->setTag(Util::emptyString);
		}
		u->setDescription(tmpDesc);
		i = j + 3;
		j = param.find('$', i);
		if(j == string::npos)
			return;
		u->setConnection(fromNmdc(param.substr(i, j-i-1)));
		i = j + 1;
		j = param.find('$', i);
		if(j == string::npos)
			return;
		u->setEmail(validateMessage(fromNmdc(param.substr(i, j-i)), true));
		i = j + 1;
		j = param.find('$', i);
		if(j == string::npos)
			return;
		u->setBytesShared(param.substr(i, j-i));

		fire(ClientListener::UserUpdated(), this, u);
	} else if(cmd == "$Quit") {
		if(!param.empty()) {
			User::Ptr u;
			{
				Lock l(cs);
				User::NickIter i = users.find(fromNmdc(param));
				if(i == users.end()) {
					dcdebug("C::onLine Quitting user %s not found\n", param.c_str());
					return;
				}
				
				u = i->second;
				users.erase(i);
			}
			
			fire(ClientListener::UserRemoved(), this, u);
			ClientManager::getInstance()->putUserOffline(u, true);
		}
	} else if(cmd == "$ConnectToMe") {
		if(state != STATE_CONNECTED) {
			return;
		}
		string::size_type i = param.find(' ');
		string::size_type j;
		if( (i == string::npos) || ((i + 1) >= param.size()) ) {
			return;
		}
		i++;
		j = param.find(':', i);
		if(j == string::npos) {
			return;
		}
		string server = fromNmdc(param.substr(i, j-i));
		if(j+1 >= param.size()) {
			return;
		}
		string port = param.substr(j+1);
		ConnectionManager::getInstance()->nmdcConnect(server, (short)Util::toInt(port), getNick(), getHubUrl()); 
	} else if(cmd == "$RevConnectToMe") {
		if(state != STATE_CONNECTED) {
			return;
		}
		User::Ptr u;
		bool up = false;
		{
			Lock l(cs);
			string::size_type j = param.find(' ');
			if(j == string::npos) {
				return;
			}

			User::NickIter i = users.find(fromNmdc(param.substr(0, j)));
			if(i == users.end()) {
				return;
			}

			u = i->second;
			if(!u->isSet(User::PASSIVE)) {
				u->setFlag(User::PASSIVE);
				up = true;
			}
		}

		if(u) {
			if(ClientManager::getInstance()->isActive()) {
				connect(u);
			} else {
				// Notify the user that we're passive too...
				if(up)
					connect(u);
			}

			if(up)
				updated(u);
		}
	} else if(cmd == "$SR") {
		SearchManager::getInstance()->onSearchResult(aLine);
	} else if(cmd == "$HubName") {
		param = fromNmdc(param);
		name = Util::replace(param, "\r\n", " ");
	
		string::size_type i;
		if( (i = name.rfind(" - ")) != string::npos)
			shortName = name.substr(0, i);
		else
			shortName = name;
		fire(ClientListener::HubUpdated(), this);
	} else if(cmd == "$Supports") {
		StringTokenizer<string> st(param, ' ');
		StringList& sl = st.getTokens();
		for(StringIter i = sl.begin(); i != sl.end(); ++i) {
			if(*i == "UserCommand") {
				supportFlags |= SUPPORTS_USERCOMMAND;
			} else if(*i == "NoGetINFO") {
				supportFlags |= SUPPORTS_NOGETINFO;
			} else if(*i == "UserIP2") {
				supportFlags |= SUPPORTS_USERIP2;
			}
		}
	} else if(cmd == "$UserCommand") {
		string::size_type i = 0;
		string::size_type j = param.find(' ');
		if(j == string::npos)
			return;

		int type = Util::toInt(param.substr(0, j));
		i = j+1;
 		if(type == UserCommand::TYPE_SEPARATOR || type == UserCommand::TYPE_CLEAR) {
			int ctx = Util::toInt(param.substr(i));
			fire(ClientListener::UserCommand(), this, type, ctx, Util::emptyString, Util::emptyString);
		} else if(type == UserCommand::TYPE_RAW || type == UserCommand::TYPE_RAW_ONCE) {
			j = param.find(' ', i);
			if(j == string::npos)
				return;
			int ctx = Util::toInt(param.substr(i));
			i = j+1;
			j = param.find('$');
			if(j == string::npos)
				return;
			string name = fromNmdc(param.substr(i, j-i));
			i = j+1;
			string command = fromNmdc(param.substr(i, param.length() - i));
			fire(ClientListener::UserCommand(), this, type, ctx, validateMessage(name, true, false), validateMessage(command, true, false));
		}
	} else if(cmd == "$Lock") {
		if(state != STATE_LOCK) {
			return;
		}
		state = STATE_HELLO;

		if(!param.empty()) {
			string::size_type j = param.find(" Pk=");
			string lock, pk;
			if( j != string::npos ) {
				lock = param.substr(0, j);
				pk = param.substr(j + 4);
			} else {
				// Workaround for faulty linux hubs...
				j = param.find(" ");
				if(j != string::npos)
					lock = param.substr(0, j);
				else
					lock = param;
			}

			if(CryptoManager::getInstance()->isExtended(lock)) {
				StringList feat;
				feat.push_back("UserCommand");
				feat.push_back("NoGetINFO");
				feat.push_back("NoHello");
				feat.push_back("UserIP2");
				feat.push_back("TTHSearch");
				feat.push_back("ZPipe0");

				if(BOOLSETTING(COMPRESS_TRANSFERS))
					feat.push_back("GetZBlock");
				supports(feat);
			}

			key(CryptoManager::getInstance()->makeKey(lock));
			validateNick(getNick());
		}
	} else if(cmd == "$Hello") {
		if(!param.empty()) {
			string nick = fromNmdc(param);
			User::Ptr u = ClientManager::getInstance()->getUser(nick, this);
			{
				Lock l(cs);
				users[nick] = u;
			}

			//broken
			//if(getNick() == nick) {

			//do the compare in ascii since the nick might not survive a utf8->ascii->utf8 roundtrip
			//might cause problems in other places?
			if(toNmdc(getNick()) == param) {
				setMe(u);

				u->setFlag(User::DCPLUSPLUS);
				if(!ClientManager::getInstance()->isActive())
					u->setFlag(User::PASSIVE);
				else
					u->unsetFlag(User::PASSIVE);
			}

			if(state == STATE_HELLO) {
				state = STATE_CONNECTED;
				updateCounts(false);

				version();
				getNickList();
				myInfo(true);
			}

			fire(ClientListener::UserUpdated(), this, u);
		}
	} else if(cmd == "$ForceMove") {
		socket->disconnect(false);
		fire(ClientListener::Redirect(), this, param);
	} else if(cmd == "$HubIsFull") {
		fire(ClientListener::HubFull(), this);
	} else if(cmd == "$ValidateDenide") {		// Mind the spelling...
		socket->disconnect(false);
		fire(ClientListener::NickTaken(), this);
	} else if(cmd == "$UserIP") {
		if(!param.empty()) {
			User::List v;
			StringTokenizer<string> t(fromNmdc(param), "$$");
			StringList& l = t.getTokens();
			for(StringIter it = l.begin(); it != l.end(); ++it) {
				string::size_type j = 0;
				if((j = it->find(' ')) == string::npos)
					continue;
				if((j+1) == it->length())
					continue;
				v.push_back(ClientManager::getInstance()->getUser(it->substr(0, j), this));
				v.back()->setIp(it->substr(j+1));
				//if it's an internal ip it'll probably cause a mismatch in connection attempts.
				if(!Util::isPrivateIp(it->substr(j+1)))
					v.back()->setUserIp(true);
			}

			fire(ClientListener::UserIp(), this, v);
		}
	} else if(cmd == "$NickList") {
		if(!param.empty()) {
			User::List v;
			StringTokenizer<string> t(fromNmdc(param), "$$");
			StringList& sl = t.getTokens();

			for(StringIter it = sl.begin(); it != sl.end(); ++it) {
				if(it->empty())
					continue;
				v.push_back(ClientManager::getInstance()->getUser(*it, this));
			}

			{
				Lock l(cs);
				for(User::Iter it2 = v.begin(); it2 != v.end(); ++it2) {
					users[(*it2)->getNick()] = *it2;
				}
			}
			
			if(!(getSupportFlags() & SUPPORTS_NOGETINFO)) {
				string tmp;
				// Let's assume 10 characters per nick...
				tmp.reserve(v.size() * (11 + 10 + getNick().length())); 
				string n = ' ' +  toNmdc(getNick()) + '|';
				for(User::List::const_iterator i = v.begin(); i != v.end(); ++i) {
					tmp += "$GetINFO ";
					tmp += toNmdc((*i)->getNick());
					tmp += n;
				}
				if(!tmp.empty()) {
					send(tmp);
				}
			} 

			fire(ClientListener::UsersUpdated(), this, v);
		}
	} else if(cmd == "$OpList") {
		if(!param.empty()) {
			User::List v;
			StringTokenizer<string> t(fromNmdc(param), "$$");
			StringList& sl = t.getTokens();
			for(StringIter it = sl.begin(); it != sl.end(); ++it) {
				if(it->empty())
					continue;
				v.push_back(ClientManager::getInstance()->getUser(*it, this));
				v.back()->setFlag(User::OP);
			}

			{
				Lock l(cs);
				for(User::Iter it2 = v.begin(); it2 != v.end(); ++it2) {
					users[(*it2)->getNick()] = *it2;
				}
			}
			fire(ClientListener::UsersUpdated(), this, v);
			updateCounts(false);
			// Special...to avoid op's complaining that their count is not correctly
			// updated when they log in (they'll be counted as registered first...)
			myInfo(false);
		}
	} else if(cmd == "$To:") {
		string::size_type i = param.find("From:");
		if(i != string::npos) {
			i+=6;
			string::size_type j = param.find("$");
			if(j != string::npos) {
				string from = fromNmdc(param.substr(i, j - 1 - i));
				if(from.size() > 0 && param.size() > (j + 1)) {
					fire(ClientListener::PrivateMessage(), this, ClientManager::getInstance()->getUser(from, this, false), validateMessage(fromNmdc(param.substr(j + 1)), true));
				}
			}
		}
	} else if(cmd == "$GetPass") {
		setRegistered(true);
		fire(ClientListener::GetPassword(), this);
	} else if(cmd == "$BadPass") {
		fire(ClientListener::BadPassword(), this);
	} else if(cmd == "$ZOn") {
		socket->setMode (BufferedSocket::MODE_ZPIPE);
	} else {
		dcassert(cmd[0] == '$');
		dcdebug("NmdcHub::onLine Unknown command %s\n", aLine.c_str());
	} 
}

string NmdcHub::checkNick(const string& aNick) {
	string tmp = aNick;
	for(size_t i = 0; i < aNick.size(); ++i) {
		if(static_cast<unsigned char>(tmp[i]) <= 32 || tmp[i] == '|' || tmp[i] == '$' || tmp[i] == '<' || tmp[i] == '>') {
			tmp[i] = '_';
		}
	}
	return tmp;
}

void NmdcHub::myInfo(bool alwaysSend) {
	checkstate();
	
	reloadSettings(false);

	dcdebug("MyInfo %s...\n", getNick().c_str());
	lastCounts = counts;
	
	string tmp1 = ";**\x1fU9";
	string tmp2 = "+L9";
	string tmp3 = "+G9";
	string tmp4 = "+R9";
	string tmp5 = "+N9";
	string::size_type i;
	
	for(i = 0; i < 6; i++) {
		tmp1[i]++;
	}
	for(i = 0; i < 3; i++) {
		tmp2[i]++; tmp3[i]++; tmp4[i]++; tmp5[i]++;
	}
	char modeChar = '?';
	if(SETTING(OUTGOING_CONNECTIONS) == SettingsManager::OUTGOING_SOCKS5)
		modeChar = '5';
	else if(ClientManager::getInstance()->isActive())
		modeChar = 'A';
	else 
		modeChar = 'P';
	
	string uMin = (SETTING(MIN_UPLOAD_SPEED) == 0) ? Util::emptyString : tmp5 + Util::toString(SETTING(MIN_UPLOAD_SPEED));
	string myInfoA = 
		"$MyINFO $ALL " + toNmdc(checkNick(getNick())) + " " + toNmdc(validateMessage(getDescription(), false)) + 
		tmp1 + VERSIONSTRING + tmp2 + modeChar + tmp3 + getCounts() + tmp4 + Util::toString(SETTING(SLOTS)) + uMin + 
		">$ $" + toNmdc(validateMessage(SETTING(UPLOAD_SPEED), false)) + "\x01$" + 
		toNmdc(validateMessage(SETTING(EMAIL), false)) + '$';
	string myInfoB = ShareManager::getInstance()->getShareSizeString() + "$|";
 	
 	if(lastMyInfoA != myInfoA || alwaysSend || (lastMyInfoB != myInfoB && lastUpdate + 15*60*1000 < GET_TICK()) ){
 		send(myInfoA + myInfoB);
 		lastMyInfoA = myInfoA;
 		lastMyInfoB = myInfoB;
 		lastUpdate = GET_TICK();
	}
}

void NmdcHub::search(int aSizeType, int64_t aSize, int aFileType, const string& aString, const string&) {
	checkstate(); 
	AutoArray<char> buf((char*)NULL);
	char c1 = (aSizeType == SearchManager::SIZE_DONTCARE) ? 'F' : 'T';
	char c2 = (aSizeType == SearchManager::SIZE_ATLEAST) ? 'F' : 'T';
	string tmp = validateMessage(toNmdc((aFileType == SearchManager::TYPE_TTH) ? "TTH:" + aString : aString), false);
	string::size_type i;
	while((i = tmp.find(' ')) != string::npos) {
		tmp[i] = '$';
	}
	int chars = 0;
	if(ClientManager::getInstance()->isActive()) {
		string x = ClientManager::getInstance()->getCachedIp();
		buf = new char[x.length() + aString.length() + 64];
		chars = sprintf(buf, "$Search %s:%d %c?%c?%s?%d?%s|", x.c_str(), SearchManager::getInstance()->getPort(), c1, c2, Util::toString(aSize).c_str(), aFileType+1, tmp.c_str());
	} else {
		buf = new char[getNick().length() + aString.length() + 64];
		chars = sprintf(buf, "$Search Hub:%s %c?%c?%s?%d?%s|", toNmdc(getNick()).c_str(), c1, c2, Util::toString(aSize).c_str(), aFileType+1, tmp.c_str());
	}
	send(buf, chars);
}

// TimerManagerListener
void NmdcHub::on(Second, time_t aTick) throw() {
	Client::on(Second(), aTick);

	if(state == STATE_CONNECTED && (getLastActivity() + getReconnDelay() * 1000) < aTick) {
		
		send("|", 1);
	}

	{
		Lock l(cs);
		
		while(!seekers.empty() && seekers.front().second + (5 * 1000) < aTick) {
			seekers.pop_front();
		}
		
		while(!flooders.empty() && flooders.front().second + (120 * 1000) < aTick) {
			flooders.pop_front();
		}
	}
}

// BufferedSocketListener
void NmdcHub::on(BufferedSocketListener::Failed, const string& aLine) throw() {
	clearUsers();
	Client::on(Failed(), aLine);
}

string NmdcHub::validateMessage(string tmp, bool reverse, bool checkNewLines) {
	string::size_type i = 0;

	if(reverse) {
		while( (i = tmp.find("&#36;", i)) != string::npos) {
			tmp.replace(i, 5, "$");
			i++;
		}
		i = 0;
		while( (i = tmp.find("&#124;", i)) != string::npos) {
			tmp.replace(i, 6, "|");
			i++;
		}
		i = 0;
		while( (i = tmp.find("&amp;", i)) != string::npos) {
			tmp.replace(i, 5, "&");
			i++;
		}
		if(checkNewLines) {
			// Check all '<' and '[' after newlines...
			i = 0;
			while( (i = tmp.find('\n', i)) != string::npos) {
				if(i + 1 < tmp.length()) {
					if(tmp[i+1] == '[' || tmp[i+1] == '<') {
						tmp.insert(i+1, "- ");
						i += 2;
					}
				}
				i++;
			}
		}
	} else {
		i = 0;
		while( (i = tmp.find("&amp;", i)) != string::npos) {
			tmp.replace(i, 1, "&amp;");
			i += 4;
		}
		i = 0;
		while( (i = tmp.find("&#36;", i)) != string::npos) {
			tmp.replace(i, 1, "&amp;");
			i += 4;
		}
		i = 0;
		while( (i = tmp.find("&#124;", i)) != string::npos) {
			tmp.replace(i, 1, "&amp;");
			i += 4;
		}
		i = 0;
		while( (i = tmp.find('$', i)) != string::npos) {
			tmp.replace(i, 1, "&#36;");
			i += 4;
		}
		i = 0;
		while( (i = tmp.find('|', i)) != string::npos) {
			tmp.replace(i, 1, "&#124;");
			i += 5;
		}
	}
	return tmp;
}
