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

#if !defined(AFX_USER_H__26AA222C_500B_4AD2_A5AA_A594E1A6D639__INCLUDED_)
#define AFX_USER_H__26AA222C_500B_4AD2_A5AA_A594E1A6D639__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Util.h"
#include "Pointer.h"
#include "CriticalSection.h"

class Client;
class FavoriteUser;

/** A user connected to one or more hubs. */
class User : public PointerBase, public Flags
{
public:
	enum {
		OP_BIT,
		ONLINE_BIT,
		DCPLUSPLUS_BIT,
		PASSIVE_BIT,
		QUIT_HUB_BIT,
		HIDDEN_BIT,
		HUB_BIT,
		BOT_BIT,
		TTH_GET_BIT
	};

	enum {
		OP = 1<<OP_BIT,
		ONLINE = 1<<ONLINE_BIT,
		DCPLUSPLUS = 1<<DCPLUSPLUS_BIT,
		PASSIVE = 1<<PASSIVE_BIT,
		QUIT_HUB = 1<<QUIT_HUB_BIT,
		HIDDEN = 1<<HIDDEN_BIT,
		HUB = 1<<HUB_BIT,
		BOT = 1<<BOT_BIT,
		TTH_GET = 1<<TTH_GET_BIT
	};
	typedef Pointer<User> Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	typedef HASH_MAP_X(string,Ptr,noCaseStringHash,noCaseStringEq,noCaseStringLess) NickMap;
	typedef NickMap::iterator NickIter;

	struct HashFunction {
		static const size_t bucket_size = 4;
		static const size_t min_buckets = 8;
		size_t operator()(const Ptr& x) const { return ((size_t)(&(*x)))/sizeof(User); };
		bool operator()(const Ptr& a, const Ptr& b) const { return (&(*a)) < (&(*b)); };
	};

	User(const string& aNick) throw() : nick(aNick), bytesShared(0), slots(0), udpPort(0), client(NULL), favoriteUser(NULL), userIp(false) { 
		string::size_type pos = aNick.find("[");
		if( pos != string::npos ) {
			string::size_type rpos = aNick.rfind("]");
			if( rpos == aNick.length() -1 && rpos > 0 ) //ugly nick =)
				rpos = aNick.rfind("]", rpos-1);
			if(rpos != string::npos) {
				shortNick = aNick.substr(rpos+1);
				isp = aNick.substr(0, rpos+1);
			} else {
				shortNick = aNick;
				isp = Util::emptyString;
			}
		}else{
			shortNick = aNick;
			isp = Util::emptyString;
		}
	};
	virtual ~User() throw();

	void setClient(Client* aClient);
	void connect();
	const string& getClientNick() const;
	const string& getClientName() const;
	string getClientUrl() const;
	void privateMessage(const string& aMsg);
	void clientMessage(const string& aMsg);
	bool isClientOp() const;
	void send(const string& msg);
	void sendUserCmd(const string& aUserCmd);
	
	string getFullNick() const { 
		string tmp(getNick());
		tmp += " (";
		tmp += getClientName();
		tmp += ")";
		return tmp;
	}
	
	void setBytesShared(const string& aSharing) { setBytesShared(Util::toInt64(aSharing)); };

	bool isOnline() const { return isSet(ONLINE); };
	bool isClient(Client* aClient) const { return client == aClient; };
	
	void getParams(StringMap& ucParams);

	// favorite user stuff
	void setFavoriteUser(FavoriteUser* aUser);
	bool isFavoriteUser() const;
	bool getFavoriteGrantSlot() const;
	void setFavoriteGrantSlot(bool grant);
	void setFavoriteLastSeen(time_t anOfflineTime = 0);
	time_t getFavoriteLastSeen() const;
	const string& getUserDescription() const;
	void setUserDescription(const string& aDescription);

	static void updated(User::Ptr& aUser);

	StringMap& clientEscapeParams(StringMap& sm) const;
	
	GETSET(string, connection, Connection);
	GETSET(string, nick, Nick);
	GETSET(string, shortNick, ShortNick);
	GETSET(string, isp, Isp);
	GETSET(string, email, Email);
	GETSET(string, description, Description);
	GETSET(string, tag, Tag);
	GETSET(string, lastHubAddress, LastHubAddress);
	GETSET(string, lastHubName, LastHubName);
	GETSET(string, ip, Ip);
	GETSET(int64_t, bytesShared, BytesShared);
	GETSET(int, slots, Slots);
	GETSET(short, udpPort, UDPPort);
	//used to determine if the ip is to be trusted for fake nick checking
	//don't trust ip's received in c-c connections.
	GETSET(bool, userIp, UserIp);
private:
	mutable RWLock<> cs;

	User(const User&);
	User& operator=(const User&);

	Client* client;
	FavoriteUser* favoriteUser;
};

#endif // !defined(AFX_USER_H__26AA222C_500B_4AD2_A5AA_A594E1A6D639__INCLUDED_)
